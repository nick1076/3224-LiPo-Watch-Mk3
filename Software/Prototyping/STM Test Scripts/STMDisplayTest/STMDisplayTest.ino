#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>

// Software (bit-bang) SPI — drives these exact pins directly, no SPI peripheral
#define TFT_CS    PA3    // A2
#define TFT_DC    PB5    // D11
#define TFT_RST   PA0    // A0
#define TFT_MOSI  PA12   // D2  -> SDA
#define TFT_SCLK  PA1    // A1  -> SCL

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

void setup() {
  tft.initR(INITR_MINI160x80);
  tft.invertDisplay(true);

  tft.fillScreen(ST77XX_RED);   delay(800);
  tft.fillScreen(ST77XX_GREEN); delay(800);
  tft.fillScreen(ST77XX_BLUE);  delay(800);
}

void loop() {}
