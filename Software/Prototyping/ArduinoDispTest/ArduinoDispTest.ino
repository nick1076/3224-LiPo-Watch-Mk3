#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

#define TFT_CS  10
#define TFT_DC   9
#define TFT_RST  8

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
// If colors are glitchy through resistor dividers, use software SPI instead:
// Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, 11 /*MOSI*/, 13 /*SCLK*/, TFT_RST);

static const unsigned char PROGMEM image_paint_3_bits[] = {0x3c,0xf0,0x3c,0xf0,0xc3,0x0c,0xc3,0x0c,0xc0,0x0c,0xc0,0x0c,0xc0,0x0c,0xc0,0x0c,0x30,0x30,0x30,0x30,0x0c,0xc0,0x0c,0xc0,0x03,0x00,0x03,0x00};

static const unsigned char PROGMEM image_paint_5_bits[] = {0x30,0x30,0xcc,0xcc,0x30,0x30};

static const unsigned char PROGMEM image_paint_6_bits[] = {0x30,0x30,0xcc,0xcc,0xcc,0xcc,0x30,0x30,0xcc,0xcc,0x30,0x30};

void setup() {
  tft.initR(INITR_MINI160x80_PLUGIN);
  tft.setRotation(3);
  tft.invertDisplay(true);
  tft.fillScreen(0x0);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(4);
  tft.setTextWrap(false);
  tft.setCursor(22, 26);
  tft.print("12:00");
  tft.setTextColor(0xF206);
  tft.setTextSize(2);
  tft.setCursor(119, 63);
  tft.print("80");
  tft.drawBitmap(143, 63, image_paint_3_bits, 14, 14, 0xF206);
  tft.setTextColor(0x24BE);
  tft.setCursor(3, 63);
  tft.print("65");
  tft.drawBitmap(27, 63, image_paint_5_bits, 6, 6, 0x24BE);
  tft.drawBitmap(110, 3, image_paint_6_bits, 6, 12, 0xFE00);
  tft.drawBitmap(118, 7, image_paint_6_bits, 6, 12, 0xFE00);
  tft.setTextColor(0xFE00);
  tft.setCursor(37, 4);
  tft.print("11,342");
}

void loop() {}
