    /////////////////////////////////////////////////////////////////
   //          ESP32 & Xiaomi Bluetooth  sensor      v1.00        //
  //       Get the latest version of the code here:              //
 //           http://educ8s.tv/esp32-xiaomi-hack                //
/////////////////////////////////////////////////////////////////


#include "SPI.h"
#include "Adafruit_GFX.h"      //https://github.com/adafruit/Adafruit-GFX-Library
#include "Adafruit_ILI9341.h"  //https://github.com/adafruit/Adafruit_ILI9341

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "esp_system.h"
#include <sstream>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "esp_system.h"

#define SCAN_TIME  10 // seconds

boolean METRIC = true; //Set true for metric system; false for imperial

BLEScan *pBLEScan;

void IRAM_ATTR resetModule(){
    ets_printf("reboot\n");
    //esp_restart_noos();
    ESP.restart();
}

#define TFT_DC 4
#define TFT_CS 15
#define TFT_RST 2
#define TFT_MISO 19         
#define TFT_MOSI 23           
#define TFT_CLK 18 

float  current_humidity = -100;
float  previous_humidity = -100;
float current_temperature = -100;
float previous_temperature = -100;

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        if (advertisedDevice.haveName() && advertisedDevice.haveServiceData() && !advertisedDevice.getName().compare("CHUNCHE1")) {
            std::string strServiceData = advertisedDevice.getServiceData();
            uint8_t cServiceData[100];
            char charServiceData[100];

            strServiceData.copy((char *)cServiceData, strServiceData.length(), 0);

            Serial.printf("\n\nAdvertised Device: %s\n", advertisedDevice.toString().c_str());

            for (int i=0;i<strServiceData.length();i++) {
                sprintf(&charServiceData[i*2], "%02x", cServiceData[i]);
            }

            std::stringstream ss;
            //ss << "fe95" << charServiceData; 
            ss << "ef680200-9b35-4933-9b10-52ffa9740042" << charServiceData; //0x181A
            
            Serial.print("Payload:");
            Serial.println(ss.str().c_str());

            char eventLog[256];
            unsigned long value, value2;
            char charValue[5] = {0,};
            switch (cServiceData[11]) {
                case 0x2A1F://0x04:
                    sprintf(charValue, "%02X%02X", cServiceData[15], cServiceData[14]);
                    value = strtol(charValue, 0, 16);
                    if(METRIC)
                    {
                      current_temperature = (float)value/10;
                    }else
                    {
                      current_temperature = CelciusToFahrenheit((float)value/10);
                    }
                    displayTemperature();  
                    break;
                case 0x06:
                    sprintf(charValue, "%02X%02X", cServiceData[15], cServiceData[14]);
                    value = strtol(charValue, 0, 16);  
                    current_humidity = (float)value/10;
                    displayHumidity();                      
                    Serial.printf("HUMIDITY_EVENT: %s, %d\n", charValue, value);
                    break;
                case 0x0A:
                    sprintf(charValue, "%02X", cServiceData[14]);
                    value = strtol(charValue, 0, 16);                    
                    Serial.printf("BATTERY_EVENT: %s, %d\n", charValue, value);
                    break;
                case 0x0D:
                    sprintf(charValue, "%02X%02X", cServiceData[15], cServiceData[14]);
                    value = strtol(charValue, 0, 16);      
                    if(METRIC)
                    {
                      current_temperature = (float)value/10;
                    }else
                    {
                      current_temperature = CelciusToFahrenheit((float)value/10);
                    }
                    displayTemperature();               
                    Serial.printf("TEMPERATURE_EVENT: %s, %d\n", charValue, value);                    
                    sprintf(charValue, "%02X%02X", cServiceData[17], cServiceData[16]);
                    value2 = strtol(charValue, 0, 16);
                    current_humidity = (float)value2/10;
                    displayHumidity();                                        
                    Serial.printf("HUMIDITY_EVENT: %s, %d\n", charValue, value2);
                    break;
            }
        }
    }
};

void setup() {

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  
  tft.begin();

  Serial.begin(115200);
  Serial.println("ESP32 THINGY DISPLAY");
 
  initBluetooth();

  drawUI();
}

void loop() {
  
    char printLog[256];
    Serial.printf("Start BLE scan for %d seconds...\n", SCAN_TIME);
    BLEScanResults foundDevices = pBLEScan->start(SCAN_TIME);
    int count = foundDevices.getCount();
    printf("Found device count : %d\n", count);
    displayTemperature();

    delay(100);
}

void drawUI()
{
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextSize(2);

  tft.drawRoundRect(0, 0, 239 , 158, 4, ILI9341_WHITE);
  tft.fillRoundRect(20, 1, 200, 40, 4, ILI9341_GREEN);

  tft.drawRoundRect(0, 160, 239 , 158, 4, ILI9341_WHITE);
  tft.fillRoundRect(20, 161, 200, 40, 4, ILI9341_CYAN);

   tft.setCursor(55, 15);
   tft.setTextColor(ILI9341_BLACK);
   tft.setTextSize(2);

   tft.print("TEMPERATURE");

   tft.setCursor(70, 173);
   tft.setTextColor(ILI9341_BLACK);
   tft.setTextSize(2);
   tft.print("HUMIDITY");
}

void initBluetooth()
{
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
    pBLEScan->setInterval(0x50);
    pBLEScan->setWindow(0x30);
}

void displayTemperature()
{
   if(current_temperature != previous_temperature)
   {
      tft.setCursor(65, 80);
      tft.setTextColor(ILI9341_BLACK);
      tft.setTextSize(4);
      tft.print(convertFloatToString(previous_temperature));
      Serial.println(convertFloatToString(previous_temperature));
      
      tft.setCursor(65, 80);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(4);
      tft.print(convertFloatToString(current_temperature));
      Serial.println(convertFloatToString(current_temperature));
      
      tft.setTextSize(2);
      tft.setCursor(65, 80);
      tft.setCursor(170, 75);
      tft.print("o");
      tft.setCursor(185, 80);
      tft.setTextSize(4);
      if(METRIC)
      {
        tft.print("C");
      }else
      {
        tft.print("F");
      }
      
      previous_temperature = current_temperature;
   }
}

void displayHumidity()
{
   if(current_humidity != previous_humidity)
   {
      tft.setCursor(65, 240);
      tft.setTextColor(ILI9341_BLACK);
      tft.setTextSize(4);
      tft.print(convertFloatToString(previous_humidity));
      
      tft.setCursor(65, 240);
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(4);
      tft.print(convertFloatToString(current_humidity));
      tft.print(" %");
      
      previous_humidity = current_humidity;
   }
}

String convertFloatToString(float f)
{
  String s = String(f,1);
  return s;
}

float CelciusToFahrenheit(float Celsius)
{
 float Fahrenheit=0;
 Fahrenheit = Celsius * 9/5 + 32;
 return Fahrenheit;
}
