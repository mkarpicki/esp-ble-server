/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Based on Neil Kolban docs: https://github.com/nkolban/esp32-snippets/blob/master/Documentation/BLE%20C%2B%2B%20Guide.pdf
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: SERVICE_UUID
   And has a characteristic of: CHARACTERISTIC_UUID

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;

int connectedDevices = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "9d319c9c-3abb-4b58-b99d-23c9b1b69ebc"                    
#define CHARACTERISTIC_UUID "a869a793-4b6e-4334-b1e3-eb0b74526c14" 

void notify (void);



class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      connectedDevices++;
      Serial.println("onConnect...");
      BLEDevice::startAdvertising();
      notify();
    };

    void onDisconnect(BLEServer* pServer) {
      connectedDevices--;
      Serial.println("onDisconnect...");
      notify();
    }
};

void notify (void) {
  pCharacteristic->setValue((int&)connectedDevices);
  pCharacteristic->notify();   
}

void setup() {
  Serial.begin(115200);

  
  String name = "ESP32_2";
  Serial.println(name);
  
  // Create the BLE Device
  BLEDevice::init(name.c_str());

  Serial.println(BLEDevice::getAddress().toString().c_str());
  
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      //BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // https://www.bluetooth.com/specifications/assigned-numbers/gatt-namespace-descriptors/
  // https://www.mathworks.com/help/matlab/ref/matlabshared.blelib.characteristic.html
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

// change value to string and test on floats
// if not as alternative use characteristics for 
// - value
// - name
// - type

void loop() {

    int interval = 1000 * 60;

    if (connectedDevices > 0) {
        Serial.print("Connected devices: "); 
        Serial.println(connectedDevices);

        notify();
    }
    delay(interval);
  
}
