/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by pcbreflux
*/

 
/*
   Create a BLE server that will send periodic iBeacon frames.
   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create advertising data
   3. Start advertising.
   4. wait
   5. Stop advertising.
   6. deep sleep
   
*/
#include "sys/time.h"

#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEBeacon.h"
#include "esp_sleep.h"
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define GPIO_DEEP_SLEEP_DURATION     2  // sleep x seconds and then wake up
RTC_DATA_ATTR static time_t last;        // remember last boot in RTC Memory
RTC_DATA_ATTR static uint32_t bootcount; // remember number of boots in RTC Memory

#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();
//uint8_t g_phyFuns;

#ifdef __cplusplus
}
#endif

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
BLEAdvertising *pAdvertising;
struct timeval now;

#define BEACON_UUID           "8ec76ea3-6668-48da-9866-75bF8be86f4d" // UUID 1 128-Bit (may use linux tool uuidgen or random numbers via https://www.uuidgenerator.net/)

void setBeacon() {

  BLEBeacon oBeacon = BLEBeacon();
  oBeacon.setManufacturerId(0x4C00); // fake Apple 0x004C LSB (ENDIAN_CHANGE_U16!)
  oBeacon.setProximityUUID(BLEUUID(BEACON_UUID));
  oBeacon.setMajor((bootcount & 0xFFFF0000) >> 16);
  oBeacon.setMinor(bootcount&0xFFFF);
  BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
  BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
  
  oAdvertisementData.setFlags(0x04); // BR_EDR_NOT_SUPPORTED 0x04
  
  std::string strServiceData = "";
  
  strServiceData += (char)26;     // Len
  strServiceData += (char)0xFF;   // Type
  strServiceData += oBeacon.getData(); 
  oAdvertisementData.addData(strServiceData);
  
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);

}

int scanTime = 2; //In seconds
BLEScan* pBLEScan;
class devices{
  public:
  String deviceName;
  devices(){
    deviceName = 'a';
  }
}obj[5];
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
     Serial.println("looping");
     String advertise = advertisedDevice.toString().c_str();
     String rssi = String(advertisedDevice.getRSSI());
     int rs = advertisedDevice.getRSSI();
     int commaIndex = advertise.indexOf(",");
     //Serial.println(commaIndex);
     String namev = advertise.substring(0,commaIndex)  ;
     int secondIndex = namev.indexOf(":");
     int length= namev.length();
     String namevalue = namev.substring(secondIndex+1 , length);
     Serial.print("advertise: ");
     Serial.println(advertise);     
     Serial.print("namevalue: ");
     Serial.println(namevalue);      
      if(namevalue.length() > 1)
      {
        Serial.println("looping if");
        bool flag = false;
        int index = 0;
        int indexNull = 0;
        String defaultName = "a";
        for(int i=0;i<=4;i++)
        {
          Serial.println("class devices: ");
          Serial.print(obj[i].deviceName);
          Serial.print(" pos: ");
          Serial.println(i);
          Serial.print("scanning device: ");
          Serial.print(namevalue);
          if(obj[i].deviceName == namevalue){
            flag = true;
            index = i;
            break;
          }
          else
           flag = false;

          if(obj[i].deviceName.equals(defaultName)) 
            indexNull = i;
        }

        if(!flag){
          if(rs >= -60){
            obj[indexNull].deviceName = namevalue;
            String payload ="{";
            payload +="\"SENDER ID \":"; payload +="\"NTCPW001\""; payload +=",";
            payload +="\"DEVICE ENTERED\":"; payload += namevalue;
            payload +="}";
            Serial.println(payload);
            Serial.print(namevalue);
            Serial.println("entered the radius");            
          }
        }
        else if(flag){
          if(rs <= -60){
            String payload ="{";
            payload +="\"SENDER ID \":"; payload +="\"NTCPW001\""; payload +=",";
            payload +="\"DEVICE EXITED\":"; payload += namevalue;
            payload +="}";
            Serial.println(payload);
            Serial.print(namevalue);
            Serial.println("exited the radius"); 
            obj[index].deviceName = defaultName;           
          }
        }
      }
    }
};

void setup() {
  Serial.begin(115200);
  gettimeofday(&now, NULL);
 Serial.printf("start ESP32 %d\n",bootcount++);
 Serial.printf("deep sleep (%lds since last reset, %lds since last boot)\n",now.tv_sec,now.tv_sec-last);
  last = now.tv_sec;
  // Create the BLE Device
   BLEDevice::init("NTCPW001");
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer(); // <-- no longer required to instantiate BLEServer, less flash and ram usage
  pAdvertising = BLEDevice::getAdvertising();
  BLEDevice::startAdvertising();
  setBeacon();   
   // Start advertising
  pAdvertising->start();
//  Serial.println("Advertizing started...");
  //delay(100);
 // pAdvertising->stop();

//Serial.println("Scanning...");

//  BLEDevice::init("");                              
//  pBLEScan = BLEDevice::getScan(); //create new scan
//  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
//  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
//  pBLEScan->setInterval(100);
//  pBLEScan->setWindow(99);  // less or equal setInterval value
//  put your main code here, to run repeatedly:
//  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
//  Serial.print("Devices found: ");
//  Serial.println(foundDevices.getCount());
//  Serial.println("Scan done!");
 //   pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
 

  
//  Serial.printf("enter deep sleep\n");
//  esp_deep_sleep(1000000LL * GPIO_DEEP_SLEEP_DURATION);
//  Serial.printf("in deep sleep\n");
}

void loop() {
                               
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
   
}
