
#define WAKE_INPUT PA_5 //Wake Button Input
#define SET_INPUT PA_4 //Set Button Input

bool awake = false; //Tracks if screen is currently awake
bool inSetMode = false; //Tracks if watch is currently in SET mode (for setting time)
bool initialized = false; //Tracks if screen initialized properly

//Other Watch control variables
byte dispCounter = 0;
int tick = 0;
int parallelTick = 0;
byte setIndex = 0;
byte currentSetHour = 0;
byte currentsetMinutes = 0;

//Time (in increments of 10ms) between each uptick in current number
int setMode_increaseTimeCooldown = 20; //5

//Time (in increments of 10ms) between each index (hour to minute to exiting set mode)
int setMode_switchSetIndexCooldown = 20; //8

int cycle = 0;
int curSteps;
float curTemp;
int curHeartRate = 0;

const unsigned char image_paint_5_copy_1_bits[] = {0x6c,0x92,0x82,0x82,0x44,0x28,0x10};
const unsigned char image_paint_6_copy_1_bits[] = {0x40,0xa0,0x40};
const unsigned char image_paint_8_copy_1_bits[] = {0x60,0x90,0x90,0x90,0x90,0x60,0x90,0x90,0x60};

#include <Wire.h> //Custom i2c pinout library
#include <Adafruit_GFX.h> //Graphics library for display
#include <Adafruit_SSD1306.h> //Display library

#define SCREEN_WIDTH 128 //Width of display used
#define SCREEN_HEIGHT 64 //Height of display used
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); //Screen

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

void setup() {
  Serial.begin(115200);
  while (!Serial); 
  Serial.println("Booting Smart Watch...");

  pinMode(WAKE_INPUT, INPUT_PULLUP);
  pinMode(SET_INPUT, INPUT_PULLUP);
  

  //DISPLAY
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    return;
  }
  else{
    Serial.println("Initialized.");
    initialized = true;
    
    display.clearDisplay();
    display.setTextColor(1);
    display.setTextWrap(false);
    display.setCursor(44, 10);
    display.print("Made By");
    display.setTextSize(2);
    display.setCursor(17, 21);
    display.print("Nicholas");
    display.setCursor(35, 38);
    display.print("Niles");
    display.display();
    delay(5000);
  }



  

  //TEMPERATURE SENSOR
  Serial.println("MCP9808 loading");
  if (!tempsensor.begin(0x1F)) {
    Serial.println("Couldn't find MCP9808!");
    return;
  }
  Serial.println("Found MCP9808!");
  tempsensor.setResolution(3);

  //RTC
  if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
    return;
  }
  else
    Serial.println("RTC online!");
  delay(1000);
  rtc.set24Hour();
  rtc.setTime(0, 51, 14, 0, 0, 0, 0);
  SetClock(12, 0);




  //Heartrate
  if(sensor.begin() && sensor.setSamplingRate(kSamplingRate)) { 
    Serial.println("Sensor initialized");
  }
  else {
    Serial.println("Sensor not found");  
    return;
  }




  //PEDOMETER
  LIS2DUXS12.begin();
  LIS2DUXS12.Enable_X();
  LIS2DUXS12.Enable_Pedometer(LIS2DUXS12_INT1_PIN);
  Serial.print("LIS2DUXS12 Initialized!");







  Serial.println("Watch Ready!");
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
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

  bool dualDigitHour = false;

  if (rtc.getHours() > 12){
    //Check if double digit hour, if so, move cursor X to the left some more
    if (rtc.getHours()-12>9){
      dualDigitHour = true;
      display.setCursor(35, 25);
    }
    else{
      display.setCursor(41, 25);
    }
  }
  else{
    if (rtc.getHours() > 9){
      dualDigitHour = true;
      display.setCursor(35, 25);
    }
    else{
      display.setCursor(41, 25);
    }
  }

  bool am = false;

  if (rtc.getHours() > 12){
    //PM
    display.print(rtc.getHours()-12, DEC);
  }
  else{
    //AM
    am = true;
    display.print(rtc.getHours(), DEC);
  }
  if (showDots){
    display.print(":");
  }
  else{
    display.print(" ");
  }
  if (rtc.getMinutes() < 10){
    display.print("0");
    display.println(rtc.getMinutes(), DEC);
  }
  else{
    display.println(rtc.getMinutes(), DEC);
  }
  
  display.setTextSize(1);

  if (dualDigitHour){
    if (am){
      display.setCursor(95, 32);
      display.print("am");
    }
    else{
      display.setCursor(95, 32);
      display.print("pm");
    }
  }
  else{
    if (am){
      display.setCursor(89, 32);
      display.print("am");
    }
    else{
      display.setCursor(89, 32);
      display.print("pm");
    }
  }

  if (!showData){
    if (!updateDisplay) { return; }
    display.display(); 
    return;
  }
  
  display.setTextSize(1);
  
  display.setCursor(94, 55);
  display.println(String(curTemp));
  display.drawBitmap(123, 52, image_paint_6_copy_1_bits, 3, 3, 1);
  
  display.setCursor(13, 53);
  display.println(String(curSteps) + " steps");
  display.drawBitmap(2, 50, image_paint_8_copy_1_bits, 4, 9, 1);
  display.drawBitmap(7, 53, image_paint_8_copy_1_bits, 4, 9, 1);
  
  display.setCursor(12, 3);
  display.println(String(curHeartRate) + " bpm");
  display.drawBitmap(3, 3, image_paint_5_copy_1_bits, 7, 7, 1);

  if (!updateDisplay) { return; }
  display.display(); 
}














void loop() {
  
  delay(10); //10 ms delay per tick
  tick++;

  /*
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
  */

  if (digitalRead(SET_INPUT) == LOW && !inSetMode){
    inSetMode = true;
    setIndex = 0;
    parallelTick = 0;
    tick=0;
    currentSetHour = rtc.getHours();
    currentsetMinutes = rtc.getMinutes();
    return;
  }

  if (inSetMode){
    DisplayTimeAndData(30, 20, 3, true, false, false);
    
    display.setTextSize(1);
    display.setCursor(38, 10);
    display.print("Set time:");
    
    if (setIndex == 0){
      display.setCursor(44, 48);
      display.print("Hour...");
    }   
    else{
      display.setCursor(38, 48);
      display.print("Minute...");
    }

    display.display();
    
    parallelTick++;
    awake = false;
    if (digitalRead(WAKE_INPUT) == LOW && parallelTick >= setMode_increaseTimeCooldown){
      parallelTick = 0;

      //Hour slot
      if (setIndex == 0){
        currentSetHour++;
        
        if (currentSetHour > 24){
          currentSetHour = 1;
        }
        
        rtc.setHours(currentSetHour);
      }

      //Minutes slot
      else if (setIndex == 1){
        currentsetMinutes++;
        
        if (currentsetMinutes > 59){
          currentsetMinutes = 0;
        }
        
        rtc.setMinutes(currentsetMinutes);
      }
    }

    //If user hits set button again after cooldown
    if (digitalRead(SET_INPUT) == LOW && tick >= setMode_switchSetIndexCooldown){
      tick=0;
      setIndex++;

      //Exit set mode
      if (setIndex >= 2){
        tick = 0;
        Clear();
        inSetMode = false;
        delay(500);
        return;
      }
    }
  }

  if (inSetMode){
    display.ssd1306_command(SSD1306_DISPLAYON);
    return;
  }
  
  if (!awake && digitalRead(WAKE_INPUT) == LOW){
    dispCounter = 0;
    awake = true;

  }
  else if (!awake){
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    return;
  }
  else if (awake){
    display.ssd1306_command(SSD1306_DISPLAYON);
    if (dispCounter >= 10){
      awake = false;
      dispCounter = 0;
      Clear();
      return;
    }
  }
  
  if (tick >= 50){
    tick=0;
  }

  
  Serial.println("");
  tempsensor.wake();
  float f = tempsensor.readTempF();
  tempsensor.shutdown_wake(1);
  Serial.print("Temp: "); 
  Serial.print(f, 4); Serial.println("*F.");
  curTemp = f;
  
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
    DisplayTimeAndData(30, 20, 3, true);
  }
  else{
    DisplayTimeAndData(30, 20, 3, false);
  }
  dispCounter++;
  if (dispCounter >= 200){
    dispCounter=0;
  }

}

void Clear(){
  display.clearDisplay();
  display.display(); 
}


//Set current clock hour and minute
void SetClock(byte hour, byte minute){
  rtc.setTime(0, minute, hour, 0, 0, 0, 0);
}
