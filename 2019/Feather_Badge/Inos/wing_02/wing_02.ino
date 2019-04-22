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

#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_HX8357.h>
#include <Adafruit_ImageReader.h> // Image-reading functions
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/Chewy_Regular_24.h>


#include "WiFi.h"
const char* WIFI_SSID     = "anacleta3456";
const char* WIFI_PASSWORD = "Cachito3017";

// initiate the WifiClient
WiFiClient wifiClient;


#include <JsonListener.h>
#include <time.h>
#include "OpenWeatherMapForecast.h"
String OPEN_WEATHER_MAP_LANGUAGE = "sp";
boolean IS_METRIC = true;
uint8_t MAX_FORECASTS = 1;

// initiate the client
OpenWeatherMapForecast client;
String OPEN_WEATHER_MAP_APP_ID = "023f1b942f8d97beaeee66e048793ed4"; //023f1b942f8d97beaeee66e048793ed4 //546c12da98fe6dda8fd7ff8b88fed87f
String OPEN_WEATHER_MAP_LOCATION_ID = "3996322";

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

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

#ifdef ESP32
   #define STMPE_CS 32
   #define TFT_CS   15
   #define TFT_DC   33
   #define SD_CS    14
#endif

int scanTime = 5; //In seconds
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
    }
};

// Anything else!


#define TFT_RST -1

// Use hardware SPI and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_ImageReader reader;     // Class w/image-reading functions
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;
void setup() {

  ImageReturnCode stat; // Status from image-reading functions
  
  Serial.begin(115200);
  delay(500);
  connectWifi();
  Serial.println("Connect...");  
  
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
  tft.setRotation(0);  
  reader.drawBMP("/01.bmp", tft, 0, 0);
  // (Absolute path isn't necessary on most devices, but something
  // with the ESP32 SD library seems to require it.)
  //reader.printStatus(stat);   // How'd we do?
  tft.setRotation(1);  
  delay(5000);  
  testLines1(HX8357_CYAN);
  delay(500);
  //showForecast();
  delay(5000);
  testLines2(HX8357_CYAN);
  delay(500);
  
}

void connectWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.println(WiFi.localIP());
  Serial.println();
}

unsigned long showForecast() {
  OpenWeatherMapForecastData data[MAX_FORECASTS];
  client.setMetric(IS_METRIC);
  client.setLanguage(OPEN_WEATHER_MAP_LANGUAGE);
  uint8_t allowedHours[] = {0,12};
  client.setAllowedHours(allowedHours, 2);
  uint8_t foundForecasts = client.updateForecastsById(data, OPEN_WEATHER_MAP_APP_ID, OPEN_WEATHER_MAP_LOCATION_ID, MAX_FORECASTS);
  time_t time;
  for (uint8_t i = 0; i < foundForecasts; i++) {
    tft.fillScreen(HX8357_BLUE);    
    tft.setFont(&Chewy_Regular_24);
    tft.setTextColor(HX8357_YELLOW);
    tft.setCursor(0, 22);
    tft.println("Clima: ");      
    tft.setFont(&FreeMono12pt7b);
    tft.setTextColor(HX8357_WHITE);    
    tft.printf("Temperatura: %f\n", data[i].temp);
    //   "temp_min":16.89, float tempMin;
    tft.printf("TempMin: %f\n", data[i].tempMin);
    //   "temp_max":17.35, float tempMax;
    tft.printf("TempMax: %f\n", data[i].tempMax);
    //   "pressure":970.8, float pressure;
    tft.printf("Presion: %f\n", data[i].pressure);
    //   "sea_level":1030.62, float pressureSeaLevel;
    tft.printf("Humedad: %d\n", data[i].humidity);
    //   "temp_kf":0.46
    // },"weather":[{
    //   "id":802, uint16_t weatherId;    
    tft.printf("Nubes: %d\n", data[i].clouds);
    // "wind":{
    //   "speed":1.77, float windSpeed;
    tft.printf("Viento: %f\n", data[i].windSpeed);
    //   "deg":207.501 float windDeg;
    tft.printf("Lluvia: %f\n", data[i].rain);    
  }
  
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
