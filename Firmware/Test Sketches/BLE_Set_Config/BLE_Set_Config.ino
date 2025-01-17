/*
  Set a baud rate to 115200bps over BLE

  Adding descriptor
  https://github.com/espressif/arduino-esp32/blob/master/libraries/BLE/src/BLECharacteristic.h
  https://gist.github.com/heiko-r/f284d95141871e12ca0164d9070d61b4
  Roughly working Characteristic descriptor: https://github.com/espressif/arduino-esp32/issues/1038

  Float to string: https://iotbyhvm.ooo/esp32-ble-tutorials/

  Custom UUIDs? https://www.bluetooth.com/specifications/assigned-numbers/

  ESP32 with chrome: https://github.com/kpatel122/ESP32-Web-Bluetooth-Terminal/blob/master/ESP32-BLE/ESP32-BLE.ino
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/


#define BLE_BROADCAST_NAME "OpenLog"

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

#define CHARACTERISTIC_UUID_LED "beb5483e-36e1-4688-b7f5-ea07361b26a9"
BLECharacteristic *ledCharacteristic;
bool ledState = 1;

#define CHARACTERISTIC_UUID_BAUD "beb5483e-36e1-4688-b7f5-ea07361b26a8"
BLECharacteristic *baudCharacteristic;
int baudRate = 9600;

#define CHARACTERISTIC_UUID_QWIIC_PWR_DOWN "beb5483e-36e1-4688-b7f5-ea07361b26ab"
BLECharacteristic *qwiicPowerCharacteristic;
bool qwiicPwrDown = 1;

bool deviceConnected = false;
bool newConfig = true;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      Serial.println("Connect!");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      Serial.println("Disconnect");
      deviceConnected = false;
    }
};

//4 bytes come in but they are little endian. Flip them around.
//Convert a std string to a int
int32_t stringToValue(std::string myString)
{
  int newValue = 0;
  for (int i = myString.length() ; i > 0 ; i--)
  {
    newValue <<= 8;
    newValue |= (myString[i - 1]);
  }

  return (newValue);
}

class cbSetBaud: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *baudCharacteristic)
    {
      baudRate = stringToValue(baudCharacteristic->getValue());
      newConfig = true;
    }
};

class cbSetLedState: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *ledCharacteristic)
    {
      ledState = stringToValue(ledCharacteristic->getValue());
      newConfig = true;
    }
};

class cbSetQwiicPower: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *qwiicPowerCharacteristic)
    {
      qwiicPwrDown = stringToValue(qwiicPowerCharacteristic->getValue());
      newConfig = true;
    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init(BLE_BROADCAST_NAME);

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);

  //Setup characterstics

  baudCharacteristic = pService->createCharacteristic(
                         CHARACTERISTIC_UUID_BAUD,
                         BLECharacteristic::PROPERTY_READ |
                         BLECharacteristic::PROPERTY_WRITE
                       );
  baudCharacteristic->setValue((uint8_t *)&baudRate, 4);
  BLEDescriptor *pDescriptor3 = new BLEDescriptor((uint16_t)0x2901); // Characteristic User Description
  baudCharacteristic->addDescriptor(pDescriptor3);
  pDescriptor3->setValue("USB Interface Baud Rate");
  baudCharacteristic->setCallbacks(new cbSetBaud());

  ledCharacteristic = pService->createCharacteristic(
                        CHARACTERISTIC_UUID_LED,
                        BLECharacteristic::PROPERTY_READ |
                        BLECharacteristic::PROPERTY_WRITE
                      );
  ledCharacteristic->setValue((uint8_t *)&ledState, 1);
  ledCharacteristic->setCallbacks(new cbSetLedState());


  qwiicPowerCharacteristic = pService->createCharacteristic(
                               CHARACTERISTIC_UUID_QWIIC_PWR_DOWN,
                               BLECharacteristic::PROPERTY_READ |
                               BLECharacteristic::PROPERTY_WRITE
                             );
  qwiicPowerCharacteristic->setValue((uint8_t *)&qwiicPwrDown, 1);
  qwiicPowerCharacteristic->setCallbacks(new cbSetQwiicPower());


  //Begin broadcasting
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();

  Serial.println("BLE Started");
}

void loop() {
  delay(200);

  if (newConfig == true)
  {
    newConfig = false;

    Serial.print("Baud rate:");
    Serial.println(baudRate);

    Serial.print("LED State:");
    Serial.println(ledState);

    if (qwiicPwrDown == true)
      Serial.println("Qwiic Power On");
    else
      Serial.println("Qwiic Power Off");

  }

  if (Serial.available())
  {
    byte incoming = Serial.read();

    if (incoming == '1')
    {
      //baudCharacteristic->setValue("Value 1");
      Serial.println("Val set");
    }
    else if (incoming == '2')
    {
      //baudCharacteristic->setValue("Value 2");
      Serial.println("Val set");
    }
    else
    {
      Serial.println("Unknown");
    }

    delay(10);
    while (Serial.available()) Serial.read(); //Clear buffer

  }
}
