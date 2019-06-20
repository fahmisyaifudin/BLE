/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define CLIENT_ID "ESPfahmi"

const char* ssid       = "Andromax-M2Y-DFFC";
const char* password   = "katingcaper";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

int scanTime = 5; //In seconds
BLEScan* pBLEScan;
int i = 0;
char text[10][200];
int rssi[10];
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      rssi[i] = advertisedDevice.getRSSI();  
      sprintf(text[i], "*%s$beacons#", BLEUtils::buildHexData(nullptr, (uint8_t*)advertisedDevice.getManufacturerData().data(),advertisedDevice.getManufacturerData().length()));
      Serial.printf("\nID = %s ; RSSI = %d", text[i], rssi[i]);
      i++;
    }
};

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("Scanning...");
  Serial.println(" CONNECTED");
  client.setServer(mqtt_server, 1883);
  client.connect(CLIENT_ID, "", "");
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.printf("Scanning \n");
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  uint8_t n = foundDevices.getCount();
  int max_rssi = rssi[0];
  int i_max_rssi = 0; 
  for (i = 1; i < n; i++){ 
      if (rssi[i] > max_rssi){ 
          max_rssi = rssi[i];
          i_max_rssi = i;
      }
  } 
  Serial.println("\n Scan done!");
  Serial.printf("Max RSSI : %d ; %s \n", max_rssi, text[i_max_rssi]);
  client.publish("/beaconsmubin", text[i_max_rssi]);
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(2000);
  i = 0;
}
