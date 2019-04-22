//
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_HX8357.h>
#include <Adafruit_STMPE610.h>
#include <Adafruit_ImageReader.h> // Image-reading functions
#include <Fonts/FreeMono12pt7b.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Fonts/Chewy_Regular_24.h>

//ESP32 PinOut
#define STMPE_CS 32
#define TFT_CS   15
#define TFT_DC   33
#define SD_CS    14

#define TFT_RST -1

// Use hardware SPI and the above for CS/DC
Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_ImageReader reader;     // Class w/image-reading functions
Adafruit_Image       img;        // An image loaded into RAM
int32_t              width  = 0, // BMP image dimensions
                     height = 0;
Adafruit_STMPE610 ts = Adafruit_STMPE610(STMPE_CS);

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 3800
#define TS_MAXX 100
#define TS_MINY 100
#define TS_MAXY 3750

//Extra Colors
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

int mode = 0;    // Currently-active animation mode, 0-XX
bool lastState = false;

#define BOXSIZE 40
#define PENRADIUS 3

#define MINPRESSURE 10
#define MAXPRESSURE 1000

bool change = 0;
bool touch = 0;


void slideImages() {  
    tft.setRotation(0);  
    reader.drawBMP("/01.bmp", tft, 0, 0);      
    delay(2000);
    tft.setRotation(0);
    reader.drawBMP("/adabot.bmp", tft, 0, 0);    
    delay(1000);    
}

void salir(){
  tft.setRotation(1);
  tft.fillScreen(OLIVE);
  tft.setCursor(0, 0);
  tft.setTextColor(HX8357_WHITE);  tft.setTextSize(5);
  tft.println("SALIR!");      
  delay(2000);
  
}

void setup() {
  // put your setup code here, to run once:
  //drawHomeScreen();  // Draws the Home Screen

  ImageReturnCode stat; // Status from image-reading functions
  
  Serial.begin(115200);
  Serial.println("Inicializando...");
  Serial.flush();
  tft.begin();  
  tft.setRotation(1);
  //Test LCD
  tft.setRotation(0);  
  tft.fillScreen(NAVY); //
  delay(3000);

  // If using I2C you can select the I2C address (there are two options) by calling
  // touch.begin(0x41), the default, or touch.begin(0x44) if A0 is tied to 3.3V
  // If no address is passed, 0x41 is used
  if (! ts.begin()) {
    Serial.println("STMPE not found!");
    while(1);
  }
  Serial.println("Waiting for touch sense");
  Serial.print(ts.bufferSize());
  Serial.print(mode);
  
  //BMP
  Serial.print(F("Initializing SD card..."));
  if(!SD.begin(SD_CS)) {
    Serial.println(F("failed!"));
    for(;;); // Loop here forever
  }
  Serial.println(F("OK!"));  
}

void loop() {
  // put your main code here, to run repeatedly: 
  //if(++mode > XX) mode = 0; // Advance to next mode, wrap around after #8

  if (change == 0 && touch == 0) {
    digitalWrite(A5, LOW);
    tft.setRotation(1);  
    tft.fillCircle(120, 160, 50, HX8357_RED);
    tft.setCursor(95, 150);
    tft.println("RED_LED");
    change = 1;
    delay(200);
  }

  if (change == 0 && touch == 1) {
    tft.setRotation(1);  
    digitalWrite(A5, HIGH);
    tft.fillCircle(120, 160, 50, HX8357_GREEN);
    tft.setCursor(95, 150);
    tft.println("GREEN_LED");
    change = 1;
    delay(200);
  }

  TS_Point p = ts.getPoint();
//  pinMode(XM, OUTPUT);
//  pinMode(YP, OUTPUT);
  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, tft.width());
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, tft.height());
    Serial.print("("); Serial.print(p.x);
    Serial.print(", "); Serial.print(p.y);
    Serial.println(")");
    if (p.x > 94 && p.x < 210 ) {
      if (p.y > 94 && p.y < 210) {
        change = 0;
        touch = !touch;
      }
    }
  }


}

/*  

if (lastState == false){
  slideImages();
  delay(1000);  
}
salir();
if (ts.touched()) {
  lastState=true;
}
if (lastState == true){ 
  if (touch.touched()) {
    if(++mode > 4) mode = 0;
      switch(mode) {           // Start the new animation...
        case 0:
          tft.setRotation(1);
          tft.fillScreen(OLIVE);
          tft.setCursor(0, 0);
          tft.setTextColor(HX8357_WHITE);  tft.setTextSize(5);
          tft.println("OLIVE!");
          tft.println(mode);
          break;
        case 1:
          tft.setRotation(1);
          tft.fillScreen(NAVY); //
          tft.setCursor(0, 0);
          tft.setTextColor(HX8357_WHITE);  tft.setTextSize(5);
          tft.println("NAVY!");
          tft.println(mode);
          break;
        case 2:
          tft.setRotation(1);
          tft.fillScreen(ORANGE); //
          tft.setCursor(0, 0);
          tft.setTextColor(HX8357_WHITE);  tft.setTextSize(5);
          tft.println("ORANGE!");
          tft.println(mode);
          break;
        case 3:
          tft.setRotation(1);
          tft.fillScreen(PURPLE); //
          tft.setCursor(0, 0);
          tft.setTextColor(HX8357_WHITE);  tft.setTextSize(5);
          tft.println("PURPLE!");
          tft.println(mode);
          break;
        case 4:
          lastState = false;
        break;                 
      }
    }
  }
*/
