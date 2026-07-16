


#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
#define TFT_CS   PA4
#define TFT_DC    PB4
#define TFT_RST   PB3
#define TFT_BLK   PA6
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);   // hardware SPI for faster refresh speeds

//SCL -> 4
//SDA -> 6

//NOTE! Change _colstart & _rowstart under Adafruit_ST7735.cpp below "options == INITR_MINI160x80_PLUGIN" to:
//This fixes the garbage rainbow pixels

//_colstart = 25;   --> Affects the horizontal lines
//_rowstart = 3;    --> Affects the vertical lines


void setup() {
  Serial.begin(115200);
  tft.initR(INITR_MINI160x80_PLUGIN);
  tft.setRotation(3);
  pinMode(TFT_BLK, OUTPUT); 
  digitalWrite(TFT_BLK, HIGH);

  delay(100);

  tft.fillScreen(0x0);
  tft.setTextColor(0xFFFF);
  tft.setTextWrap(false);
  tft.setCursor(30, 24);
  tft.print("0.96\" TFT Display");
  tft.setTextColor(0xF206);
  tft.setCursor(54, 34);
  tft.print("Dev Board");
  tft.setTextColor(0x3A96);
  tft.setCursor(39, 44);
  tft.print("Nicholas Niles");
  tft.setTextColor(0xE8EC);
  tft.setTextSize(2);
  tft.setCursor(45, 60);
  tft.print("160x80");
  tft.fillTriangle(56, 8, 60, 16, 52, 16, 0x61D6);
  tft.fillRect(64, 8, 9, 9, 0x7AA9);
  tft.fillCircle(80, 12, 4, 0x4D6A);
  tft.fillTriangle(92, 8, 96, 16, 88, 16, 0xFC00);
  tft.fillRect(100, 8, 9, 9, 0xFF47);
}



void loop(void) {

}
