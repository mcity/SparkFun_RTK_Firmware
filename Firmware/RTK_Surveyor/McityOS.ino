/*
 * WebSocketClientSocketIOack.ino
 *
 *  Created on: 20.07.2019
 *
 */
 
// Websocket connection to Mcity OS
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
#include <ArduinoJson.h>
#include <SocketIOclient.h>
#include <MicroNMEA.h>

#include "NTRIPClient.h"

// SocketIO helper functions
void socketIOSendWithNamespace(socketIOmessageType_t type, String payload);
void socketIOSendEventWithNamespace(String payload);
void socketIOEventHandler(socketIOmessageType_t type, uint8_t * payload, size_t length);

// Tasks to handle read/write from GPS to McityOS
TaskHandle_t McityOSF9PSerialReadTaskHandle = NULL; //Mcity OS PSM/BSM construction task handle
TaskHandle_t McityOSSendV2XTaskHandle = NULL; //Mcity OS task transmission handle
TaskHandle_t F9PSerialWriteTaskWiFiHandle = NULL; //Store handle to NTRIP client task in case we need to halt

const int mcityReadTaskStackSize = 2500;
const int mcityV2XTaskStackSize = 6000;

// NTRIP client
NTRIPClient ntrip_c;
char nmeaBuffer[85];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

// SocketIO connection to McityOS
SocketIOclient socketIO;

// How many milliseconds between sending PSMs to Mcity OS? 0 sends as fast as we can
#define MCITY_MS_BETWEEN_PSMS 200

char latitude_str[12];
char longitude_str[12];
char elevation_str[12];
char speed_str[12];
char heading_str[12];
char date_str[26];
long alt;
char json_psm[375] = "['v2x_PSM', {'id': 1, 'payload': {'longitude': -83.698641, 'latitude': 42.299598}}]";
bool v2xMessageAvailable;
uint32_t lastMcityOSSend = 0;

const char* mcitySocketIONamespacePrefix = "/octane,";

bool mcityOSConnected = false;
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

void socketIOSendWithNamespace(socketIOmessageType_t type, String payload) {
    socketIO.send(type, mcitySocketIONamespacePrefix + payload);
}

void socketIOSendEventWithNamespace(String payload) {
    socketIOSendWithNamespace(sIOtype_EVENT, payload);
}

void socketIOEventHandler(socketIOmessageType_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case sIOtype_DISCONNECT:
            printDebug("SocketIOClient: server disconnected!\n");

            mcityOSConnected = false;
            break;
        case sIOtype_CONNECT:
        {
            printDebug("SocketIOClient: connected to url: ");
            printDebug((char*)payload);
            printDebug("\n");

            // join default namespace (no auto join in Socket.IO V3)
            socketIOSendWithNamespace(sIOtype_CONNECT, "{}");

            // Create and send auth event
            DynamicJsonDocument doc(128);
            JsonArray array = doc.to<JsonArray>();
    
            array.add("auth");
    
            // add payload (parameters) for the event
            JsonObject param1 = array.createNestedObject();
            param1["x-api-key"] = settings.mcityOSAPIKey;
    
            // JSON to String (serializion)
            String output;
            serializeJson(doc, output);
    
            // Send event
            socketIOSendEventWithNamespace(output);

            mcityOSConnected = true;
        }
            break;
        case sIOtype_EVENT:
        {
            char * sptr = NULL;
            int id = strtol((char *)payload, &sptr, 10);

            printDebug("SocketIOClient: got event ");
            printDebug((char*)payload);
            printDebug("\n");
        }
            break;
        case sIOtype_ACK:
            printDebug("SocketIOClient: got ack ");
            printDebug((char*)payload);
            printDebug("\n");
            break;
        case sIOtype_ERROR:
            printDebug("SocketIOClient: got error ");
            printDebug((char*)payload);
            printDebug("\n");

            mcityOSConnected = false;

            break;
        case sIOtype_BINARY_EVENT:
            printDebug("SocketIOClient: got binary data\n");
            break;
        case sIOtype_BINARY_ACK:
            printDebug("SocketIOClient: got binary ack\n");
            break;
    }
}

//If the ZED has any new NMEA data, pass it over to Mcity OS
//Task for reading data from the GNSS receiver.
void McityOSF9PSerialReadTask(void *e)
{
  while (true)
  {
    if (serialGNSS.available())
    {
      char c = serialGNSS.read();
      //Serial.print(c);

      //If we are actively survey-in then do not pass NMEA data from ZED to phone
      if (systemState == STATE_BASE_TEMP_SETTLE || systemState == STATE_BASE_TEMP_SURVEY_STARTED)
      {
        //Do nothing
      }
      else if (nmea.process(c) && nmea.isValid() && !v2xMessageAvailable) {
        dtostrf(nmea.getLatitude() / 1000000.0, 0, 7, latitude_str);
        dtostrf(nmea.getLongitude() / 1000000.0, 0, 7, longitude_str);
        if (nmea.getAltitude(alt))
          dtostrf(alt / 1000., 0, 3, elevation_str);
        else
          // JSON null, so the string
          strcpy(elevation_str, "0");  // Bug in McityOS doesn't allow null for this field. Send 0 for now. 

        dtostrf(nmea.getSpeed() / 1000., 0, 3, speed_str);
        dtostrf(nmea.getCourse() / 1000., 0, 3, heading_str);

        // Expected format is 2023-07-08T16:42:16.236Z
        snprintf(date_str, sizeof(date_str), "%d-%02d-%02dT%02d:%02d:%02d.%03dZ", 
          nmea.getYear(), nmea.getMonth(), nmea.getDay(), nmea.getHour(),
          nmea.getMinute(), nmea.getSecond(), nmea.getHundredths());

        nmea.clear();

        sprintf_P(json_psm, PSTR("[\"v2x_PSM\", {\"id\": 1, \"payload\": {"
                            "\"messageSet\": \"J2735_201603\","
                            // updated wasn't working in Mcity OS. Re-add once fixed to ensure we're using the GPS time
                            //"\"updated\": \"%s\","
                            "\"id\": \"0010BEEF\","
                            "\"type\": \"pedestrian\","
                            "\"size\": \"small\","
                            "\"latitude\": %s,"
                            "\"longitude\": %s,"
                            "\"elevation\": %s,"
                            "\"speed\": %s,"
                            //"\"heading\": %s}}]"), date_str, latitude_str, longitude_str, elevation_str, 
                            "\"heading\": %s}}]"), latitude_str, longitude_str, elevation_str, 
                                                   speed_str, heading_str);

        v2xMessageAvailable = true;
      }
    }

    taskYIELD();
  }
}

//If the we have a new PSM/BSM to send to Mcity OS, do so
void McityOSSendV2XTask(void *e)
{
  while (true)
  {
    if (v2xMessageAvailable && (millis() - lastMcityOSSend > MCITY_MS_BETWEEN_PSMS) 
      && (systemState == STATE_ROVER_RTK_FLOAT || systemState == STATE_ROVER_RTK_FIX))
    {
      lastMcityOSSend = millis();

      // Send event
      if (socketIO.isConnected() && mcityOSConnected) {
        printDebug("Sending position: ");
        printDebug(json_psm);
        printDebug("\n");
        
        socketIOSendEventWithNamespace(json_psm);
      }

      v2xMessageAvailable = false;
    }

    taskYIELD();
  }
}

//If the ntrip caster has any new data (NTRIP RTCM, etc), read it in over WiFi and pass along to ZED
//Task for writing to the GNSS receiver
void F9PSerialWriteTaskWiFi(void *e)
{
  while (true)
  {
    //Receive RTCM corrections WiFi and pass along to ZED
    if (ntrip_c.available())
    {
      while (ntrip_c.available())
      {
        if (inTestMode == false)
        {
          //Pass bytes to GNSS receiver
          char ch = ntrip_c.read();
          serialGNSS.write(ch);
        }
        else
        {
          char ch = ntrip_c.read();
          Serial.printf("I heard: %c\n", ch);
          incomingBTTest = ch; //Displayed during system test
        }
      }
    }

    taskYIELD();
  }
}

// Start the tasks for handling outgoing bytes from ZED-F9P to Mcity OS
void startMcityOSTasks() {
  if (McityOSF9PSerialReadTaskHandle == NULL)
    xTaskCreate(
      McityOSF9PSerialReadTask,
      "McityOS F9Read", //Just for humans
      mcityReadTaskStackSize, //Stack Size
      NULL, //Task input parameter
      0, //Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      &McityOSF9PSerialReadTaskHandle); //Task handle

  if (McityOSSendV2XTaskHandle == NULL)
    xTaskCreate(
      McityOSSendV2XTask,
      "McityOS V2XSend", //Just for humans
      mcityV2XTaskStackSize, //Stack Size
      NULL, //Task input parameter
      0, //Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      &McityOSSendV2XTaskHandle); //Task handle
}

void stopMcityOSTasks() {
  //Delete tasks if running
  if (McityOSF9PSerialReadTaskHandle != NULL)
  {
    vTaskDelete(McityOSF9PSerialReadTaskHandle);
    McityOSF9PSerialReadTaskHandle = NULL;
  }

  if (McityOSSendV2XTaskHandle != NULL)
  {
    vTaskDelete(McityOSSendV2XTaskHandle);
    McityOSSendV2XTaskHandle = NULL;
  }
}

void setupMcityOS() {

    if (settings.enableMcityOS != true) {
      Serial.println("McityOS not enabled");
      return;
    }

    Serial.println("McityOS enabled, establishing websocket connection");

    // server address and port (uses EIO v3 by default, as there were Mcity OS server issues with v4)
    if (settings.mcityOSServerUseSSL) 
      socketIO.beginSSL(settings.mcityOSServer, settings.mcityOSServerPort);//, "/socket.io/?EIO=4");
    else
      socketIO.begin(settings.mcityOSServer, settings.mcityOSServerPort);//, "/socket.io/?EIO=4");

    // event handler
    socketIO.onEvent(socketIOEventHandler);
}

void updateMcityOS() {

  socketIO.loop();
}
