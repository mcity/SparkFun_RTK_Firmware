/*
 * WebSocketClientSocketIOack.ino
 *
 *  Created on: 20.07.2019
 *
 */
bool mcityOSConnected = false;
String mcitySocketIONamespace = "/octane";

void socketIOSendWithNamespace(socketIOmessageType_t type, String payload) {
    socketIO.send(type, mcitySocketIONamespace + "," + payload);
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

            // if(id) {
            //     payload = (uint8_t *)sptr;
            // }
            // DynamicJsonDocument doc(1024);
            // DeserializationError error = deserializeJson(doc, payload, length);
            // if(error) {
            //     printDebug("SocketIOClient: deserializeJson() failed with ");
            //     printDebug(error.c_str());
            //     printDebug("\n");
            //     return;
            // }

            // String eventName = doc[0];
            // printDebug("SocketIOClient: event name ");
            // printDebug(eventName.c_str());
            // printDebug("\n");

            // // Message Includes a ID for a ACK (callback)
            // if(id) {
            //     // creat JSON message for Socket.IO (ack)
            //     DynamicJsonDocument docOut(1024);
            //     JsonArray array = docOut.to<JsonArray>();

            //     // add payload (parameters) for the ack (callback function)
            //     JsonObject param1 = array.createNestedObject();
            //     param1["now"] = millis();

            //     // JSON to String (serializion)
            //     String output;
            //     output += id;
            //     serializeJson(docOut, output);

            //     // Send event
            //     socketIO.send(sIOtype_ACK, output);
            // }
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
            break;
        case sIOtype_BINARY_EVENT:
            printDebug("SocketIOClient: got binary data\n");
            break;
        case sIOtype_BINARY_ACK:
            printDebug("SocketIOClient: got binary ack\n");
            break;
    }
}

void setupMcityOS() {

    if (settings.enableMcityOS != true) {
      Serial.println("McityOS not enabled");
      return;
    }

    Serial.println("McityOS enabled, establishing websocket connection");

    // server address, port and URL
    if (settings.mcityOSServerUseSSL) 
      socketIO.beginSSL(settings.mcityOSServer, settings.mcityOSServerPort, "/socket.io/?EIO=4");
    else
      socketIO.begin(settings.mcityOSServer, settings.mcityOSServerPort, "/socket.io/?EIO=4");

    // event handler
    socketIO.onEvent(socketIOEventHandler);
}


