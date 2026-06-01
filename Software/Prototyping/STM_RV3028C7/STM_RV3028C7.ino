#include <RV-3028-C7.h>
RV3028 rtc;
bool initFailed = false;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("=== BOOT ===");   // prints every time you press reset

  Wire.begin();                      // default I2C1: SDA=PB7, SCL=PB6

  if (!rtc.begin()) {
    Serial.println("Something went wrong, check wiring");
    initFailed = true;
  } else {
    Serial.println("RTC online!");
    rtc.set24Hour();
    rtc.setTime(0, 51, 14, 0, 0, 0, 0);
  }
}

void loop() {
  if (initFailed) {
    Serial.println("alive, waiting on RTC...");   // heartbeat
    delay(1000);
    return;
  }
  if (rtc.updateTime()) {
    Serial.println(rtc.stringTimeStamp());
  } else {
    Serial.println("RTC failed to update");
  }
  delay(1000);
}
