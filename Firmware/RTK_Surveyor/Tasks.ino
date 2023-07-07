//High frequency tasks made by createTask()
//And any low frequency tasks that are called by Ticker

char latitude_str[12];
char longitude_str[12];
char elevation_str[12];
char speed_str[12];
char heading_str[12];
long alt;
char json_psm[512] = "['v2x_PSM', {'id': 1, 'payload': {'longitude': -83.698641, 'latitude': 42.299598}}]";
bool v2xMessageAvailable;

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
      else if (nmea.process(c) && nmea.isValid()) {
        dtostrf(nmea.getLatitude() / 1000000.0, 0, 7, latitude_str);
        dtostrf(nmea.getLongitude() / 1000000.0, 0, 7, longitude_str);
        if (nmea.getAltitude(alt))
          dtostrf(alt / 1000., 0, 3, elevation_str);
        else
          // JSON null, so the string
          strcpy(elevation_str, "null"); 

        dtostrf(nmea.getSpeed() / 1000., 0, 3, speed_str);
        dtostrf(nmea.getCourse() / 1000., 0, 3, heading_str);

        // sprintf_P(json_psm, PSTR("[\"v2x_PSM\", {\"id\": 1, \"payload\": {"
        //                     "\"messageSet\": \"J2735_201603\"," 
        //                     "\"id\": \"0010BEEF\","
        //                     "\"type\": \"pedestrian\","
        //                     "\"size\": \"small\","
        //                     "\"latitude\": %s,"
        //                     "\"longitude\": %s,"
        //                     "\"elevation\": %s,"
        //                     "\"speed\": %s,"
        //                     "\"heading\": %s}}]"), latitude_str, longitude_str, elevation_str, speed_str, heading_str);

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
    if (v2xMessageAvailable)
    {
      v2xMessageAvailable = false;

      // Send event
//      if (wsclient.available())
      if (wsclient.isConnected()) {
        Serial.println("Sending test");
        wsclient.sendTXT("test");
      }
        // socketIO.sendEVENT("test");
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

//If the phone has any new data (NTRIP RTCM, etc), read it in over Bluetooth and pass along to ZED
//Task for writing to the GNSS receiver
void F9PSerialWriteTask(void *e)
{
  while (true)
  {
    //Receive RTCM corrections or UBX config messages over bluetooth and pass along to ZED
    if (SerialBT.available())
    {
      while (SerialBT.available())
      {
        if (inTestMode == false)
        {
          //Pass bytes to GNSS receiver
          auto s = SerialBT.readBytes(wBuffer, SERIAL_SIZE_RX);
          serialGNSS.write(wBuffer, s);
        }
        else
        {
          char incoming = SerialBT.read();
          Serial.printf("I heard: %c\n", incoming);
          incomingBTTest = incoming; //Displayed during system test
        }
      }
    }

    taskYIELD();
  }
}

//If the ZED has any new NMEA data, pass it out over Bluetooth
//Task for reading data from the GNSS receiver.
void F9PSerialReadTask(void *e)
{
  while (true)
  {
    if (serialGNSS.available())
    {
      auto s = serialGNSS.readBytes(rBuffer, SERIAL_SIZE_RX);

      //If we are actively survey-in then do not pass NMEA data from ZED to phone
      if (systemState == STATE_BASE_TEMP_SETTLE || systemState == STATE_BASE_TEMP_SURVEY_STARTED)
      {
        //Do nothing
      }
      else if (SerialBT.connected())
      {
        SerialBT.write(rBuffer, s);
      }

      if (settings.enableHeapReport == true)
      {
        if (millis() - lastTaskHeapReport > 1000)
        {
          lastTaskHeapReport = millis();
          Serial.printf("Task freeHeap: %d\n\r", ESP.getFreeHeap());
        }
      }

      //If user wants to log, record to SD
      if (online.logging == true)
      {
        //Check if we are inside the max time window for logging
        if ((systemTime_minutes - startLogTime_minutes) < settings.maxLogTime_minutes)
        {
          //Attempt to write to file system. This avoids collisions with file writing from other functions like recordSystemSettingsToFile()
          if (xSemaphoreTake(xFATSemaphore, fatSemaphore_shortWait_ms) == pdPASS)
          {
            ubxFile.write(rBuffer, s);

            //Force file sync every 5000ms
            if (millis() - lastUBXLogSyncTime > 5000)
            {
              if (productVariant == RTK_SURVEYOR)
                digitalWrite(pin_baseStatusLED, !digitalRead(pin_baseStatusLED)); //Blink LED to indicate logging activity

              long startWriteTime = micros();
              ubxFile.sync();
              long stopWriteTime = micros();
              totalWriteTime += stopWriteTime - startWriteTime; //Used to calculate overall write speed

              if (productVariant == RTK_SURVEYOR)
                digitalWrite(pin_baseStatusLED, !digitalRead(pin_baseStatusLED)); //Return LED to previous state

              updateDataFileAccess(&ubxFile); // Update the file access time & date

              lastUBXLogSyncTime = millis();
            }

            xSemaphoreGive(xFATSemaphore);
          } //End xFATSemaphore
          else
          {
            log_d("F9SerialRead: Semaphore failed to yield");
          }
        } //End maxLogTime
      } //End logging
    } //End serial available from GNSS

    taskYIELD();
  }
}

//Assign UART2 interrupts to the current core. See: https://github.com/espressif/arduino-esp32/issues/3386
void startUART2Task( void *pvParameters )
{
  serialGNSS.begin(settings.dataPortBaud); //UART2 on pins 16/17 for SPP. The ZED-F9P will be configured to output NMEA over its UART1 at the same rate.
  serialGNSS.setRxBufferSize(SERIAL_SIZE_RX);
  serialGNSS.setTimeout(50);

  uart2Started = true;

  vTaskDelete( NULL ); //Delete task once it has run once
}

//Control BT status LED according to bluetoothState
void updateBTled()
{
  if (productVariant == RTK_SURVEYOR)
  {
    if (radioState == BT_ON_NOCONNECTION)
      digitalWrite(pin_bluetoothStatusLED, !digitalRead(pin_bluetoothStatusLED));
    else if (radioState == BT_CONNECTED)
      digitalWrite(pin_bluetoothStatusLED, HIGH);
    else
      digitalWrite(pin_bluetoothStatusLED, LOW);
  }
}
