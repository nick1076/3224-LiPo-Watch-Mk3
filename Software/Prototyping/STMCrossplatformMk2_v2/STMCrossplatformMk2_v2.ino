
#define WAKE_INPUT PB0 //Wake Button Input
#define SET_INPUT PA4 //Set Button Input
#define BLK_OUTPUT PA7 //TFT Backlight Output

bool awake = false; //Tracks if screen is currently awake
bool setMode = false; //Tracks if watch is currently in SET mode (for setting time)
bool initialized = false; //Tracks if screen initialized properly

//Other Watch control variables
byte dispCounter = 0;
int tick = 0;
int parallelTick = 0;
byte setIndex = 0;
byte currentSetHour = 0;
byte currentsetMinutes = 0;

//Time (in increments of 10ms) between each uptick in current number
int setMode_increaseTimeCooldown = 10;

//Time (in increments of 10ms) between each index (hour to minute to exiting set mode)
int setMode_switchSetIndexCooldown = 25;

int cycle = 0;
int curSteps = 0;
int curTemp = 0;
int curHeartRate = 0;

bool initFailed = false;

static const unsigned char PROGMEM image_heart[] = {0x3c,0xf0,0x3c,0xf0,0xc3,0x0c,0xc3,0x0c,0xc0,0x0c,0xc0,0x0c,0xc0,0x0c,0xc0,0x0c,0x30,0x30,0x30,0x30,0x0c,0xc0,0x0c,0xc0,0x03,0x00,0x03,0x00};
static const unsigned char PROGMEM image_degrees[] = {0x30,0x30,0xcc,0xcc,0x30,0x30};
static const unsigned char PROGMEM image_footstep[] = {0x30,0x30,0xcc,0xcc,0xcc,0xcc,0x30,0x30,0xcc,0xcc,0x30,0x30};

static const unsigned char PROGMEM image_sensorIcon_MCP_thermometerLiquid[] = {0x40,0x40,0xe0,0xe0};
static const unsigned char PROGMEM image_sensorIcon_RV_clockHands[] = {0x20,0x40,0x80,0x40};
static const unsigned char PROGMEM image_sensorIcon_MAX_heart[] = {0x63,0x00,0xf7,0x80,0xff,0x80,0xff,0x80,0x7f,0x00,0x3e,0x00,0x1c,0x00,0x08,0x00};
static const unsigned char PROGMEM image_sensorIcon_TFT_dispFrame[] = {0xff,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0xff,0x80};
static const unsigned char PROGMEM image_sensorIcon_TFT_dispRedSquiggle[] = {0x40,0xa0};
static const unsigned char PROGMEM image_sensorIcon_TFT_dispBlueSquiggle[] = {0xd8,0x20};
static const unsigned char PROGMEM image_sensorIcon_LIS_footprint[] = {0x40,0xa0,0xa0,0x40,0xa0,0x40};
static const unsigned char PROGMEM image_sensorIcon_MCP_thermometer[] = {0x20,0x50,0x58,0x50,0x58,0x50,0x88,0x88,0x70};

static const unsigned char PROGMEM image_checkmark[] = {0x10,0xa0,0x40};
static const unsigned char PROGMEM image_checkmark_backing[] = {0x7e,0xff,0xfb,0xd7,0xef,0xff,0x7e};

static const unsigned char PROGMEM image_x[] = {0xa0,0x40,0xa0};
static const unsigned char PROGMEM image_x_backing[] = {0x7c,0xfe,0xd6,0xee,0xd6,0xfe,0x7c};

#include <Wire.h> //Custom i2c pinout library


#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <SPI.h>
SPIClass tftSPI(PA12 /*MOSI = D2*/, PA6 /*MISO = A5, unused*/, PA1 /*SCLK = A1*/);
Adafruit_ST7735 tft = Adafruit_ST7735(&tftSPI, D10, D9, D6); //CS, DC, RST

#include <LIS2DUXS12Sensor.h>
LIS2DUXS12Sensor LIS2DUXS12(&Wire);
uint16_t step_count = 0;
char report[256];

#include <MAX3010x.h>
#include "filters.h"
MAX30101 sensor;
const auto kSamplingRate = sensor.SAMPLING_RATE_400SPS;
const float kSamplingFrequency = 400.0;
const unsigned long kFingerThreshold = 10000;
const unsigned int kFingerCooldownMs = 500;
const float kEdgeThreshold = -2000.0;
const float kLowPassCutoff = 5.0;
const float kHighPassCutoff = 0.5;
const bool kEnableAveraging = true;
const int kAveragingSamples = 50;
const int kSampleThreshold = 5;

#include <RV-3028-C7.h>
RV3028 rtc;

#include "Adafruit_MCP9808.h"
Adafruit_MCP9808 tempsensor = Adafruit_MCP9808();

int lastDrawnData_heartRate = -1;
int lastDrawnData_steps = -1;
int lastDrawnData_temperature = -1;
int lastHour = -1;
int lastMinute = -1;
String lastAMPM = "-1";
String lastDots = "-1";

void drawData_heartRate(){
  bool redraw = false;
  
  if (curHeartRate != lastDrawnData_heartRate){
    redraw = true;
  }

  if (redraw){
    tft.drawBitmap(143, 63, image_heart, 14, 14, 0xF206);
    tft.fillRect(106, 62, 36, 16, 0x0);
    tft.setTextColor(0xF206);
    tft.setTextSize(2);
    
    if (curHeartRate>99){
      tft.setCursor(107, 63);
    }
    else if (curHeartRate < 10){
      tft.setCursor(131, 63);
      tft.print("-");
      lastDrawnData_heartRate = curHeartRate;
      return;
    }
    else{
      tft.setCursor(119, 63);
    }
    
    tft.print(curHeartRate);
    lastDrawnData_heartRate = curHeartRate;
  }
}

void drawData_steps(){
  bool redraw = false;

  if (curSteps != lastDrawnData_steps){
    redraw = true;
  }

  if (redraw){
    tft.drawBitmap(3, 3, image_footstep, 6, 12, 0xFE00);
    tft.drawBitmap(11, 7, image_footstep, 6, 12, 0xFE00);
    tft.fillRect(19, 3, 60, 16, 0x0);
    tft.setTextColor(0xFE00);
    tft.setTextSize(2);
    tft.setCursor(20, 4);
    tft.print(curSteps);
    
    lastDrawnData_steps = curSteps;
  }

}

void drawData_temperature(){
  bool redraw = false;

  if (curTemp != lastDrawnData_temperature){
    redraw = true;
  }

  if (redraw){
    tft.fillRect(2, 62, 44, 16, 0x0);
    if (curTemp < 10){
      tft.drawBitmap(15, 63, image_degrees, 6, 6, 0x24BE);
    }
    else if (curTemp < 100){
      tft.drawBitmap(27, 63, image_degrees, 6, 6, 0x24BE);
    }
    else{
      tft.drawBitmap(39, 63, image_degrees, 6, 6, 0x24BE);
    }
    tft.setTextColor(0x24BE);
    tft.setCursor(3, 63);
    tft.setTextSize(2);
    tft.print(curTemp);
    
    lastDrawnData_temperature = curTemp;
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(WAKE_INPUT, INPUT_PULLUP);
  pinMode(SET_INPUT, INPUT_PULLUP);
  pinMode(BLK_OUTPUT, OUTPUT);
  
  tft.enableDisplay(true);
  digitalWrite(BLK_OUTPUT, HIGH);
  
  tft.initR(INITR_MINI160x80_PLUGIN);
  tft.setRotation(3);
  tft.invertDisplay(true);

  // -- Splash Screen --
  tft.fillScreen(0x0);
  tft.setTextColor(0xFFFF);
  tft.setTextSize(2);
  tft.setTextWrap(false);
  tft.setCursor(15, 25);
  tft.print("Smart Watch");
  tft.setTextColor(0x24BE);
  tft.setTextSize(1);
  tft.setCursor(72, 16);
  tft.print("Mk3");
  tft.setTextColor(0xFC00);
  tft.setCursor(39, 42);
  tft.print("Nicholas Niles");
  tft.setTextColor(0xFF47);
  tft.setCursor(3, 70);
  tft.print("https://nicholasniles.com/");
  delay(3000);

  // -- Init Screen --
  tft.fillScreen(0x0);
  tft.setTextColor(0xFFFF);
  tft.setTextWrap(false);
  tft.setCursor(2, 2);
  tft.print("Initializing Watch...");
  tft.drawLine(67, 10, 2, 10, 0xFFFF);
  tft.drawLine(126, 10, 73, 10, 0xFFFF);
  tft.setTextColor(0x5FA);
  tft.setCursor(16, 15);
  tft.print("ST7735 TFT");
  tft.setCursor(16, 27);
  tft.print("MCP9808");
  tft.setCursor(16, 39);
  tft.print("RV-3028-C7");
  tft.setCursor(16, 51);
  tft.print("MAX30101");
  tft.setCursor(16, 63);
  tft.print("LIS2DUXS12");
  tft.drawBitmap(7, 26, image_sensorIcon_MCP_thermometer, 5, 9, 0xFFFF);
  tft.drawBitmap(8, 30, image_sensorIcon_MCP_thermometerLiquid, 3, 4, 0xF206);
  tft.fillCircle(9, 42, 4, 0xFFFF);
  tft.drawBitmap(9, 40, image_sensorIcon_RV_clockHands, 3, 4, 0x0);
  tft.drawBitmap(6, 62, image_sensorIcon_LIS_footprint, 3, 6, 0xFE00);
  tft.drawBitmap(5, 50, image_sensorIcon_MAX_heart, 9, 8, 0xF206);
  tft.drawBitmap(10, 65, image_sensorIcon_LIS_footprint, 3, 6, 0xFE00);
  tft.fillRect(6, 15, 7, 7, 0x4208);
  tft.drawBitmap(5, 14, image_sensorIcon_TFT_dispFrame, 9, 9, 0xFFFF);
  tft.drawBitmap(7, 19, image_sensorIcon_TFT_dispRedSquiggle, 3, 2, 0xF206);
  tft.drawBitmap(7, 16, image_sensorIcon_TFT_dispBlueSquiggle, 5, 2, 0x55E);
  tft.setTextColor(0xFF47);
  tft.setCursor(75, 15);
  tft.print("...");
  tft.setCursor(57, 27);
  tft.print("...");
  tft.setCursor(75, 39);
  tft.print("...");
  tft.setCursor(62, 51);
  tft.print("...");
  tft.setCursor(75, 63);
  tft.print("...");
  tft.setTextColor(0xBDF7);
  tft.setCursor(150, 27);
  tft.print("-");
  tft.setCursor(150, 39);
  tft.print("-");
  tft.setCursor(150, 51);
  tft.print("-");
  tft.setCursor(150, 63);
  tft.print("-");
  tft.drawBitmap(150, 17, image_checkmark, 4, 3, 0xFFFF);
  tft.drawBitmap(148, 15, image_checkmark_backing, 8, 7, 0x4D6A);
  delay(1250);

  //TEMPERATURE SENSOR
  Serial.println("MCP9808 loading");
  if (!tempsensor.begin(0x1F)) {
    Serial.println("Couldn't find MCP9808!");
    tft.drawBitmap(149, 27, image_x_backing, 7, 7, 0xF206);
    tft.drawBitmap(151, 29, image_x, 3, 3, 0xFFFF);
    initFailed = true;
    return;
  }
  Serial.println("Found MCP9808!");
  tempsensor.setResolution(3);
  tft.drawBitmap(148, 27, image_checkmark_backing, 8, 7, 0x4D6A);
  tft.drawBitmap(150, 29, image_checkmark, 4, 3, 0xFFFF);
  delay(1250);

  //RTC
  if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
    tft.drawBitmap(149, 39, image_x_backing, 7, 7, 0xF206);
    tft.drawBitmap(151, 41, image_x, 3, 3, 0xFFFF);
    initFailed = true;
    return;
  }
  else
    Serial.println("RTC online!");
  tft.drawBitmap(148, 39, image_checkmark_backing, 8, 7, 0x4D6A);
  tft.drawBitmap(150, 41, image_checkmark, 4, 3, 0xFFFF);
  delay(1250);
  rtc.set24Hour();
  rtc.setTime(0, 51, 14, 0, 0, 0, 0);
  SetClock(12, 0);

  //Heartrate
  if(sensor.begin() && sensor.setSamplingRate(kSamplingRate)) { 
    Serial.println("Sensor initialized");
    tft.drawBitmap(148, 51, image_checkmark_backing, 8, 7, 0x4D6A);
    tft.drawBitmap(150, 53, image_checkmark, 4, 3, 0xFFFF);
  }
  else {
    Serial.println("Sensor not found");  
    tft.drawBitmap(149, 51, image_x_backing, 7, 7, 0xF206);
    tft.drawBitmap(151, 53, image_x, 3, 3, 0xFFFF);
    initFailed = true;
    return;
  }
  delay(1250);

  //PEDOMETER
  LIS2DUXS12.begin();
  LIS2DUXS12.Enable_X();
  LIS2DUXS12.Enable_Pedometer(LIS2DUXS12_INT1_PIN);
  Serial.print("LIS2DUXS12 Initialized!");
  tft.drawBitmap(148, 63, image_checkmark_backing, 8, 7, 0x4D6A);
  tft.drawBitmap(150, 65, image_checkmark, 4, 3, 0xFFFF);

  Serial.println("Watch Ready!");
  delay(3000);
  tft.fillScreen(0x0);
}

// Filter Instances
HighPassFilter high_pass_filter(kHighPassCutoff, kSamplingFrequency);
LowPassFilter low_pass_filter(kLowPassCutoff, kSamplingFrequency);
Differentiator differentiator(kSamplingFrequency);
MovingAverageFilter<kAveragingSamples> averager;

// Timestamp of the last heartbeat
long last_heartbeat = 0;

// Timestamp for finger detection
long finger_timestamp = 0;
bool finger_detected = false;

// Last diff to detect zero crossing
float last_diff = NAN;
bool crossed = false;
long crossed_time = 0;







void DisplayTimeAndData(int x, int y, int size, bool showDots, bool updateDisplay = true, bool showData = true){

  tft.setTextColor(0xFFFF);
  tft.setTextSize(4);
  tft.setTextWrap(false);

  int hoursUsed = 0;
  int minutesUsed = 0;

  if (setMode){
    hoursUsed = currentSetHour;
    minutesUsed = currentsetMinutes;
  }
  else{
    hoursUsed = rtc.getHours();
    minutesUsed = rtc.getMinutes();
  }

  bool dualDigitHour = false;

  if (hoursUsed > 12){
    //Check if double digit hour, if so, move cursor X to the left some more
    if (hoursUsed-12>9){
      dualDigitHour = true;
      tft.setCursor(22, 26);
    }
    else{
      tft.setCursor(34, 26);
    }
  }
  else{
    if (hoursUsed > 9){
      dualDigitHour = true;
      tft.setCursor(22, 26);
    }
    else{
      tft.setCursor(34, 26);
    }
  }

  bool am = false;
  if (hoursUsed > 12){
    //PM
  }
  else{
    //AM
    am = true;
  }
    
  
  if (lastMinute == minutesUsed && lastHour == hoursUsed){
    //No need to update
    if (showDots){
      if (lastDots != "dots"){
        if (dualDigitHour){
          tft.print("  :");
        }
        else{
          tft.print(" :");
        }
        lastDots = "dots";
      }
    }
    else{
      if (lastDots != "space"){
          if (dualDigitHour){
            tft.fillRect(77, 33, 6, 14, 0x0);
          }
          else{
            tft.fillRect(65, 33, 6, 14, 0x0);
          }
        lastDots = "space";
      }
    }
  }
  else{
    tft.fillRect(21, 25, 118, 30, 0x0);
    if (hoursUsed > 12){
      //PM
      tft.print(hoursUsed-12, DEC);
    }
    else{
      //AM
      am = true;
      tft.print(hoursUsed, DEC);
    }
    if (showDots){
      tft.print(":");
      lastDots = "dots";
    }
    else{
      if (hoursUsed > 9){
        tft.fillRect(77, 33, 6, 14, 0x0);
      }
      else{
        tft.fillRect(65, 33, 6, 14, 0x0);
      }
      tft.print(" ");
      lastDots = "space";
    }
        
    if (minutesUsed < 10){
      tft.print("0");
      tft.println(minutesUsed, DEC);
    }
    else{
      tft.println(minutesUsed, DEC);
    }
  }
  
  


  
  tft.setTextSize(2);

  if (am){
    if (lastAMPM != "am"){
      tft.fillRect(67, 57, 26, 16, 0x0);
      tft.setCursor(69, 55);
      tft.print("am");
      lastAMPM = "am";
    }
  }
  else{
    if (lastAMPM != "pm"){
      tft.fillRect(67, 57, 26, 16, 0x0);
      tft.setCursor(69, 55);
      tft.print("pm");
      lastAMPM = "pm";
    }
  }

  lastHour = hoursUsed;
  lastMinute = minutesUsed;

  if (!showData){
    if (!updateDisplay) { return; }
    return;
  }

  drawData_heartRate();
  drawData_steps();
  drawData_temperature();

  if (!updateDisplay) { return; }
}

void loop() {
  if (initFailed){
    delay(1000);
    return;
  }
  
  delay(10); //10 ms delay per tick
  tick=tick+1;

  auto sample = sensor.readSample(1000);
  float current_value = sample.red;
  
  // Detect Finger using raw sensor value
  if(sample.red > kFingerThreshold) {
    if(millis() - finger_timestamp > kFingerCooldownMs) {
      finger_detected = true;
    }
  }
  else {
    // Reset values if the finger is removed
    differentiator.reset();
    averager.reset();
    low_pass_filter.reset();
    high_pass_filter.reset();
    
    finger_detected = false;
    finger_timestamp = millis();
  }

  if(finger_detected) {
    current_value = low_pass_filter.process(current_value);
    current_value = high_pass_filter.process(current_value);
    float current_diff = differentiator.process(current_value);

    // Valid values?
    if(!isnan(current_diff) && !isnan(last_diff)) {
      
      // Detect Heartbeat - Zero-Crossing
      if(last_diff > 0 && current_diff < 0) {
        crossed = true;
        crossed_time = millis();
      }
      
      if(current_diff > 0) {
        crossed = false;
      }
  
      // Detect Heartbeat - Falling Edge Threshold
      if(crossed && current_diff < kEdgeThreshold) {
        
        if(last_heartbeat != 0 && crossed_time - last_heartbeat > 300) {
          // Show Results
          int bpm = 60000/(crossed_time - last_heartbeat);
          if(bpm > 50 && bpm < 250) {
              Serial.println("3");  
            // Average?
            if(kEnableAveraging) {
              int average_bpm = averager.process(bpm);
  
              // Show if enough samples have been collected
              if(averager.count() > kSampleThreshold) {
                Serial.print("Heart Rate (avg, bpm): ");
                Serial.println(average_bpm);
                curHeartRate = average_bpm;
              }
            }
            else {
              Serial.print("Heart Rate (current, bpm): ");
              Serial.println(bpm);  
              curHeartRate = bpm;
            }
          }
        }
  
        crossed = false;
        last_heartbeat = crossed_time;
      }
    }

    last_diff = current_diff;
  }

  if (digitalRead(SET_INPUT) == LOW && !setMode){
    setMode = true;
    setIndex = 0;
    parallelTick = 0;
    tick=0;
    currentSetHour = rtc.getHours();
    currentsetMinutes = rtc.getMinutes();        
    lastDrawnData_heartRate = -1;
    lastDrawnData_steps = -1;
    lastDrawnData_temperature = -1;
    lastHour = -1;
    lastMinute = -1;
    lastAMPM = "-1";
    lastDots = "-1";
    tft.fillScreen(0x0);
    tft.setTextColor(0xF206);
    tft.setTextSize(1);
    tft.setCursor(36, 3);
    tft.print("Setting time...");
    tft.setTextColor(0xFE00);
    tft.setCursor(96, 14);
    tft.print("Hour");
    tft.setTextColor(0x55E);
    tft.setCursor(42, 14);
    tft.print("Editing:");
    return;
  }

  if (setMode){
    SetClock(currentSetHour, currentsetMinutes);
    tft.enableDisplay(true);
    digitalWrite(BLK_OUTPUT, HIGH);
    DisplayTimeAndData(30, 20, 3, true, false, false);

    parallelTick=parallelTick+1;
    awake = false;
    if (digitalRead(WAKE_INPUT) == LOW && parallelTick >= setMode_increaseTimeCooldown){
      parallelTick = 0;

      //Hour slot
      if (setIndex == 0){
        currentSetHour++;
        
        if (currentSetHour > 24){
          currentSetHour = 1;
        }
        
        return;
      }

      //Minutes slot
      else if (setIndex == 1){
        currentsetMinutes++;
        
        if (currentsetMinutes > 59){
          currentsetMinutes = 0;
        }
        
        return;
      }
    }

    //If user hits set button again after cooldown
    if (digitalRead(SET_INPUT) == LOW && tick >= setMode_switchSetIndexCooldown){
      tick=0;
      setIndex++;

      if (setIndex == 1){
        //Now in minutes mode
        tft.fillRect(40, 12, 81, 11, 0x0);
        tft.setTextSize(1);
        tft.setTextColor(0xFE00);
        tft.setCursor(90, 14);
        tft.print("Minute");
        tft.setTextColor(0x55E);
        tft.setCursor(36, 14);
        tft.print("Editing:");
      }

      //Exit set mode
      if (setIndex >= 2){
        tick = 0;
        parallelTick;
        Clear();
        setMode = false;
        lastDrawnData_heartRate = -1;
        lastDrawnData_steps = -1;
        lastDrawnData_temperature = -1;
        lastHour = -1;
        lastMinute = -1;
        lastAMPM = "-1";
        lastDots = "-1";
        delay(250);
        return;
      }
      return;
    }
    
    return;
  }
  
  if (!awake && digitalRead(WAKE_INPUT) == LOW){
    dispCounter = 0;
    awake = true;

  }
  else if (!awake){
    tft.enableDisplay(false);
    digitalWrite(BLK_OUTPUT, LOW);
    lastDrawnData_heartRate = -1;
    lastDrawnData_steps = -1;
    lastDrawnData_temperature = -1;
    lastHour = -1;
    lastMinute = -1;
    lastAMPM = "-1";
    lastDots = "-1";
    return;
  }
  else if (awake){
    tft.enableDisplay(true);
    digitalWrite(BLK_OUTPUT, HIGH);
    if (dispCounter >= 10){
      awake = false;
      dispCounter = 0;
      Clear();
      return;
    }
  }
  
  if (parallelTick < 50){
    parallelTick++;
    return;
  }
  parallelTick=0;

  
  Serial.println("");
  float f = tempsensor.readTempF();
  Serial.print("Temp: "); 
  Serial.print(f, 4); Serial.println("*F.");
  curTemp = int(f);
  
  if (rtc.updateTime() == false) //Updates the time variables from RTC
  {
    Serial.print("RTC failed to update");
  } else {
    String currentTime = rtc.stringTimeStamp();
    Serial.println(currentTime);
  }

  LIS2DUXS12.Get_Step_Count(&step_count);
  snprintf(report, sizeof(report), "Step counter: %d", step_count);
  Serial.println(report);
  curSteps = step_count;
  
  if (dispCounter % 2 == 0){
    DisplayTimeAndData(30, 20, 3, true, true, true);
  }
  else{
    DisplayTimeAndData(30, 20, 3, false, true, true);
  }
  dispCounter++;
  if (dispCounter >= 200){
    dispCounter=0;
  }

  if (tick>10000){
    tick=0;
  }

}

void Clear(){
  tft.fillScreen(0x0);
}

//Set current clock hour and minute
void SetClock(byte hour, byte minute){
  rtc.setTime(0, minute, hour, 0, 0, 0, 0);
}
