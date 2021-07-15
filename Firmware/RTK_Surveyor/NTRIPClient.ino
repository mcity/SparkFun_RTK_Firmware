/*
 *  NTRIP client for Arduino Ver. 1.0.0 
 *  NTRIPClient Sample
 *  Request Source Table (Source Table is basestation list in NTRIP Caster)
 *  Request Reference Data 
 * 
 * 
 */
#include "NTRIPClient.h"


void startNtripClientStream() {

  //Start the tasks for handling incoming bytes to ZED-F9P
  if (F9PSerialWriteTaskWiFiHandle == NULL)
    xTaskCreate(
      F9PSerialWriteTaskWiFi,
      "F9WriteWiFi", //Just for humans
      writeTaskStackSize, //Stack Size
      NULL, //Task input parameter
      0, //Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
      &F9PSerialWriteTaskWiFiHandle); //Task handle
}

void stopNtripClientStream() {

  if (F9PSerialWriteTaskWiFiHandle != NULL)
  {
    vTaskDelete(F9PSerialWriteTaskWiFiHandle);
    F9PSerialWriteTaskWiFiHandle = NULL;
  }
}


bool NTRIPClient::reqSrcTbl(char* host,uint16_t &port)
{
  if (!connect(host, port))
  {
      Serial.print("Cannot connect to ");
      Serial.println(host);
      return false;
  }
  const int SERVER_BUFFER_SIZE  = 512;
  char serverBuffer[SERVER_BUFFER_SIZE];

  snprintf(serverBuffer, SERVER_BUFFER_SIZE, "GET / HTTP/1.0\r\nUser-Agent: NTRIPClient for Arduino v1.0\r\n");

  //Serial.printf("Sending credentials:\n%s\n\r", serverBuffer);
  write(serverBuffer, strlen(serverBuffer));
                            
  unsigned long timeout = millis();
  while (available() == 0) {
     if (millis() - timeout > 5000) {
        Serial.println(F("NTRIP Client timeout connecting to caster"));
        stop();
        return false;
     }
     delay(10);
  }

  char buffer[SERVER_BUFFER_SIZE];
  readLine(buffer, sizeof(buffer));
  
  if(strncmp((char*)buffer,"SOURCETABLE 200 OK",17) == 0) {
     return true;
  }
    
  Serial.print(F("Received unknown response from NTRIP server: "));
  Serial.println((char*)buffer);
  
  return false;   
}

bool NTRIPClient::reqRaw(char* host,uint16_t &port,char* mntpnt,char* user,char* psw)
{
    if(!connect(host,port))return false;
    String p="GET /";
    String auth="";
    Serial.println("Request NTRIP");
    
    p = p + mntpnt + String(" HTTP/1.0\r\n"
        "User-Agent: NTRIPClient for Arduino v1.0\r\n"
    );
    
    if (strlen(user)==0) {
        p = p + String(
            "Accept: */*\r\n"
            "Connection: close\r\n"
            );
    }
    else {
        auth = base64::encode(String(user) + String(":") + psw);
        #ifdef Debug
        Serial.println(String(user) + String(":") + psw);
        #endif

        p = p + String("Authorization: Basic ");
        p = p + auth;
        p = p + String("\r\n");
    }
    p = p + String("\r\n");
    print(p);
    #ifdef Debug
    Serial.println(p);
    #endif
    unsigned long timeout = millis();
    while (available() == 0) {
        if (millis() - timeout > 20000) {
            Serial.println(F("Client Timeout !"));
            return false;
        }
        delay(10);
    }
    char buffer[50];
    readLine(buffer,sizeof(buffer));
    if(strncmp((char*)buffer,"ICY 200 OK",10))
    {
      Serial.print((char*)buffer);
      return false;
    }
    return true;
}

bool NTRIPClient::reqRaw(char* host,uint16_t &port,char* mntpnt)
{
    return reqRaw(host,port,mntpnt,"","");
}

int NTRIPClient::readLine(char* _buffer,int size)
{
  int len = 0;
  while(available()) {
    _buffer[len] = read();
    len++;
    if(_buffer[len-1] == '\n' || len >= size - 1) break;
  }
  _buffer[len]='\0';

  return len;
}
