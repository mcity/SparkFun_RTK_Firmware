/*
 * WebSocketClientSocketIOack.ino
 *
 *  Created on: 20.07.2019
 *
 */


// void socketIOEvent(socketIOmessageType_t type, uint8_t * payload, size_t length) {
//     switch(type) {
//         case sIOtype_DISCONNECT:
//             USE_SERIAL.printf(F"[IOc] Disconnected!\n");
//             break;
//         case sIOtype_CONNECT:
//         {
//             USE_SERIAL.printf(F"[IOc] Connected to url: %s\n", payload);

//             // join default namespace (no auto join in Socket.IO V3)
//             socketIO.send(sIOtype_CONNECT, "/octane");

//             // Create and send auth event
//             DynamicJsonDocument doc(1024);
//             JsonArray array = doc.to<JsonArray>();
    
//             array.add("auth");
    
//             // add payload (parameters) for the event
//             JsonObject param1 = array.createNestedObject();
//             param1["x-api-key"] = settings.mcityOSAPIKey;
    
//             // JSON to String (serializion)
//             String output;
//             serializeJson(doc, output);
    
//             // Send event
//             socketIO.sendEVENT(output);
//         }
//             break;
//         case sIOtype_EVENT:
//         {
//             char * sptr = NULL;
//             int id = strtol((char *)payload, &sptr, 10);
//             USE_SERIAL.printf(F"[IOc] get event: %s id: %d\n", payload, id);
//             if(id) {
//                 payload = (uint8_t *)sptr;
//             }
//             DynamicJsonDocument doc(1024);
//             DeserializationError error = deserializeJson(doc, payload, length);
//             if(error) {
//                 USE_SERIAL.print(F("deserializeJson() failed: "));
//                 USE_SERIAL.println(error.c_str());
//                 return;
//             }

//             String eventName = doc[0];
//             USE_SERIAL.printf(F"[IOc] event name: %s\n", eventName.c_str());

//             // Message Includes a ID for a ACK (callback)
//             if(id) {
//                 // creat JSON message for Socket.IO (ack)
//                 DynamicJsonDocument docOut(1024);
//                 JsonArray array = docOut.to<JsonArray>();

//                 // add payload (parameters) for the ack (callback function)
//                 JsonObject param1 = array.createNestedObject();
//                 param1["now"] = millis();

//                 // JSON to String (serializion)
//                 String output;
//                 output += id;
//                 serializeJson(docOut, output);

//                 // Send event
//                 socketIO.send(sIOtype_ACK, output);
//             }
//         }
//             break;
//         case sIOtype_ACK:
//             USE_SERIAL.printf(F"[IOc] get ack: %u\n", length);
//             break;
//         case sIOtype_ERROR:
//             USE_SERIAL.printf(F"[IOc] get error: %u\n", length);
//             break;
//         case sIOtype_BINARY_EVENT:
//             USE_SERIAL.printf(F"[IOc] get binary: %u\n", length);
//             break;
//         case sIOtype_BINARY_ACK:
//             USE_SERIAL.printf(F"[IOc] get binary ack: %u\n", length);
//             break;
//     }
// }

// void socketIOEvent(WebsocketsEvent event, String data) {
//       if(event == WebsocketsEvent::ConnectionOpened) {
//         Serial.println("Connnection Opened");
//     } else if(event == WebsocketsEvent::ConnectionClosed) {
//         Serial.println("Connnection Closed");
//     } else if(event == WebsocketsEvent::GotPing) {
//         Serial.println("Got a Ping!");
//     } else if(event == WebsocketsEvent::GotPong) {
//         Serial.println("Got a Pong!");
//     }


//     switch(event) {
//         case WebsocketsEvent::ConnectionClosed:
//             Serial.printf(F"[IOc] Disconnected!\n");
//             break;
//         case WebsocketsEvent::ConnectionOpened:
//         {
//             Serial.printf(F"[IOc] Connected to url: %s\n", payload);

//             // join default namespace (no auto join in Socket.IO V3)
//             socketIO.send(sIOtype_CONNECT, "/octane");

//             // Create and send auth event
//             DynamicJsonDocument doc(1024);
//             JsonArray array = doc.to<JsonArray>();
    
//             array.add("auth");
    
//             // add payload (parameters) for the event
//             JsonObject param1 = array.createNestedObject();
//             param1["x-api-key"] = settings.mcityOSAPIKey;
    
//             // JSON to String (serializion)
//             String output;
//             serializeJson(doc, output);
    
//             // Send event
//             socketIO.sendEVENT(output);
//         }
//             break;
//         case sIOtype_EVENT:
//         {
//             char * sptr = NULL;
//             int id = strtol((char *)payload, &sptr, 10);
//             USE_SERIAL.printf(F"[IOc] get event: %s id: %d\n", payload, id);
//             if(id) {
//                 payload = (uint8_t *)sptr;
//             }
//             DynamicJsonDocument doc(1024);
//             DeserializationError error = deserializeJson(doc, payload, length);
//             if(error) {
//                 USE_SERIAL.print(F("deserializeJson() failed: "));
//                 USE_SERIAL.println(error.c_str());
//                 return;
//             }

//             String eventName = doc[0];
//             USE_SERIAL.printf(F"[IOc] event name: %s\n", eventName.c_str());

//             // Message Includes a ID for a ACK (callback)
//             if(id) {
//                 // creat JSON message for Socket.IO (ack)
//                 DynamicJsonDocument docOut(1024);
//                 JsonArray array = docOut.to<JsonArray>();

//                 // add payload (parameters) for the ack (callback function)
//                 JsonObject param1 = array.createNestedObject();
//                 param1["now"] = millis();

//                 // JSON to String (serializion)
//                 String output;
//                 output += id;
//                 serializeJson(docOut, output);

//                 // Send event
//                 socketIO.send(sIOtype_ACK, output);
//             }
//         }
//             break;
//         case sIOtype_ACK:
//             USE_SERIAL.printf(F"[IOc] get ack: %u\n", length);
//             break;
//         case sIOtype_ERROR:
//             USE_SERIAL.printf(F"[IOc] get error: %u\n", length);
//             break;
//         case sIOtype_BINARY_EVENT:
//             USE_SERIAL.printf(F"[IOc] get binary: %u\n", length);
//             break;
//         case sIOtype_BINARY_ACK:
//             USE_SERIAL.printf(F"[IOc] get binary ack: %u\n", length);
//             break;
//     }
// }

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {


    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            break;
        case WStype_CONNECTED:
            {
                USE_SERIAL.printf("[WSc] Connected to url: %s\n",  payload);

			          // send message to server when Connected
				        //webSocket.sendTXT("Connected");
            }
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[WSc] get text: %s\n", payload);

            // send message to server
            // webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
            //hexdump(payload, length);

            // send data to server
            // webSocket.sendBIN(payload, length);
            break;
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
    }
}

void setupMcityOS() {

    if (settings.enableMcityOS != true) 
      return;

    // server address, port and URL
    //socketIO.beginSSL(settings.mcityOSServer, 443, "/socket.io/?EIO=4");
    //wsclient.connect(settings.mcityOSServer, 443, "/");
    wsclient.beginSSL(settings.mcityOSServer, 443, "/");

    // event handler
    //socketIO.onEvent(socketIOEvent);
    wsclient.onEvent(webSocketEvent);
}

//unsigned long messageTimestamp = 0;
//void loop() {
//    socketIO.loop();
//
////    uint64_t now = millis();
////
////    if(now - messageTimestamp > 2000) {
////        messageTimestamp = now;
////
////        // create JSON message for Socket.IO (event)
////        DynamicJsonDocument doc(1024);
////        JsonArray array = doc.to<JsonArray>();
////
////        // add event name
////        // Hint: socket.on('event_name', ....
////        array.add("event_name");
////
////        // add payload (parameters) for the event
////        JsonObject param1 = array.createNestedObject();
////        param1["now"] = (uint32_t) now;
////
////        // JSON to String (serializion)
////        String output;
////        serializeJson(doc, output);
////
////        // Send event
////        socketIO.sendEVENT(output);
////
////        // Print JSON for debugging
////        USE_SERIAL.println(output);
////    }
//}
