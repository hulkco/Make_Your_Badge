/***************************************************
  This is our library for the Adafruit 3.5" TFT (HX8357) FeatherWing
  ----> http://www.adafruit.com/products/3651

  Check out the links above for our tutorials and wiring diagrams
  These displays use SPI to communicate, 4 or 5 pins are required to
  interface (RST is optional)
  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/

//Main Librarys
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include <Adafruit_ImageReader.h> // Image-reading functions
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/Chewy_Regular_24.h>
#include "ListLib.h"

//WiFi Librarys
#include "WiFi.h"
//BLE Librarys
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
//Custom Extra Colors
#define LIME        0x67F3  ///<   0,   0, 123
#define NAVY        0x000F  ///<   0,   0, 123
#define DARKGREEN   0x03E0  ///<   0, 125,   0
#define DARKCYAN    0x03EF  ///<   0, 125, 123
#define MAROON      0x7800  ///< 123,   0,   0
#define PURPLE      0x780F  ///< 123,   0, 123
#define OLIVE       0x7BE0  ///< 123, 125,   0
#define LIGHTGREY   0xC618  ///< 198, 195, 198
#define DARKGREY    0x7BEF  ///< 123, 125, 123
#define ORANGE      0xFD20  ///< 255, 165,   0
#define GREENYELLOW 0xAFE5  ///< 173, 255,  41
#define PINK        0xFC18  ///< 255, 130, 198
//Select ESP32 or ESP8266 Feather
#ifdef ESP32
   #define STMPE_CS 32
   #define TFT_CS   15
   #define TFT_DC   33
   #define SD_CS    14
#endif
#ifdef ESP8266
   #define STMPE_CS 16
   #define TFT_CS   0
   #define TFT_DC   15
   #define SD_CS    2
#endif



// Anything else!


#define TFT_RST -1

// Use hardware SPI and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_ImageReader reader;     // Class w/image-reading functions
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;
int scanTime = 5; //In seconds
String valid = "";
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      /*
      tft.fillScreen(HX8357_BLUE);
      tft.setFont(&FreeMonoBold9pt7b);
      tft.setTextColor(HX8357_YELLOW); 
      tft.setCursor(0, 22);    
      tft.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      */
      valid = ("Advertised Device: %s \n",advertisedDevice.toString().c_str());      
    }
};


unsigned long bleScanPrint() {
  unsigned long start = micros();
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  tft.fillScreen(HX8357_BLUE);    
  tft.setFont(&Chewy_Regular_24);
  tft.setTextColor(HX8357_YELLOW);
  tft.setCursor(0, 22);
  tft.print("BLE Devices found: ");  
  tft.setTextColor(HX8357_RED);  
  tft.println(foundDevices.getCount());
  tft.setFont(&FreeMonoBold9pt7b);
  tft.setTextColor(HX8357_WHITE);
  tft.println(valid);
  return micros() - start;
}

unsigned long wifiScanPrint(){
   int n = WiFi.scanNetworks();    
    if (n == 0) {
        //Serial.println("no networks found");
    } else {        
        tft.fillScreen(HX8357_BLUE);    
        tft.setFont(&Chewy_Regular_24);
        tft.setTextColor(HX8357_YELLOW);
        tft.setCursor(0, 22);  
        tft.print("WiFI networks found ");
        tft.setTextColor(HX8357_RED);  
        tft.println(n);
        //tft.println(" networks found");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            tft.setFont(&FreeMonoBold9pt7b);
            tft.setTextColor(HX8357_WHITE); //tft.setTextSize(2);
            tft.print(i + 1);
            tft.print(": ");
            tft.print(WiFi.SSID(i));
            tft.print(" (");
            tft.print(WiFi.RSSI(i));
            tft.print(")");
            tft.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*");
            delay(10);
        }
    }
    tft.println("");
}

unsigned long testLines1(uint16_t color) {
  unsigned long start, t;
  int           x1, y1, x2, y2,
                w = tft.width(),
                h = tft.height();

  tft.fillScreen(HX8357_BLACK);

  x1 = y1 = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, ORANGE);
  x2    = w - 1;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, PURPLE);
  t     = micros() - start; // fillScreen doesn't count against timing
  return micros() - start;
}

unsigned long testLines2(uint16_t color) {
  unsigned long start, t;
  int           x1, y1, x2, y2,
                w = tft.width(),
                h = tft.height();

tft.fillScreen(HX8357_BLACK);

  x1    = w - 1;
  y1    = 0;
  y2    = h - 1;
  start = micros();
  for(x2=0; x2<w; x2+=6) tft.drawLine(x1, y1, x2, y2, PURPLE);
  x2    = 0;
  for(y2=0; y2<h; y2+=6) tft.drawLine(x1, y1, x2, y2, ORANGE);
  t    += micros() - start;
  return micros() - start;
}

void setup() {

  ImageReturnCode stat; // Status from image-reading functions
  
  Serial.begin(115200);
  Serial.println("Scanning..."); 
  
  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  tft.begin();  
  tft.setRotation(1);  

  //BMP
  Serial.print(F("Initializing SD card..."));
  if(!SD.begin(SD_CS)) {
    Serial.println(F("failed!"));
    for(;;); // Loop here forever
  }
  Serial.println(F("OK!"));  
}


void loop(void) {  
  //bmpReadDraw();
  tft.setRotation(1);  
  reader.drawBMP("/01.bmp", tft, 0, 0);
  delay(1000);      
  testLines1(HX8357_CYAN);
  delay(1000);  
  reader.drawBMP("/02.bmp", tft, 0, 0);
  delay(1000);
  testLines1(HX8357_CYAN);
  delay(1000);
  wifiScanPrint();    
  delay(5000);
  bleScanPrint();  
  delay(5000);  
  testLines2(HX8357_CYAN);
  delay(1000);   
  reader.drawBMP("/03.bmp", tft, 0, 0);
  delay(1000);     
  reader.drawBMP("/04.bmp", tft, 0, 0);
  delay(1000);   
  testLines1(HX8357_CYAN);
  delay(1000);  
}
