/*
  Setting and reading time from RV-3028-C7 Real Time Clock
  By: Constantin Koch
  Date: 7/31/2019
  License: This code is public domain but you buy me a beer if you use this and we meet someday (Beerware license).

  Feel like supporting my work? Give me a star!
  https://github.com/constiko/RV-3028_C7-Arduino_Library

  This example shows how to set the time on the RTC to the compiler time or a custom time, and how to read the time.
  Open the serial monitor at 115200 baud.
*/

#include <RV-3028-C7.h>

RV3028 rtc;

//The below variables control what the date will be set to
int sec = 0;
int minute = 0;
int hour = 16;
int day = 4;
int date = 1;
int month = 8;
int year = 2019;

void setup() {

  Serial.begin(115200);
  while (!Serial);
  Serial.println("Read/Write Time - RTC Example");

  Wire.begin();
  if (rtc.begin() == false) {
    Serial.println("Something went wrong, check wiring");
    while (1);
  }
  else
    Serial.println("RTC online!");
  delay(1000);

  rtc.set24Hour();
  rtc.setTime(0, 51, 14, 0, 0, 0, 0);
  Serial.println("");
  // rtc.setTime(sec, minute, hour, day, date, month, year);
}

void loop() {

  //PRINT TIME
  if (rtc.updateTime() == false) //Updates the time variables from RTC
  {
    Serial.print("RTC failed to update");
  } else {
    delay(1000);
    Serial.print(rtc.getHours());
    Serial.print(":");
    Serial.print(rtc.getMinutes());
    Serial.print(":");
    Serial.print(rtc.getSeconds());
    Serial.println("");
  }
}
