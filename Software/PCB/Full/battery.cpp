
#include "battery.h"

#include <Arduino.h>
#include <SparkFunBQ27441.h>



uint32_t SR_DATA      = PB5;   // Data in
uint32_t SR_CLK       = PB6;   // Main register clock
uint32_t SR_LATCHCLK  = PB1;   // Storage clock
uint32_t SR_LEDENA    = PB7;   // Output enable



//Actual LiPo Logic
void batteryBegin(int capacity){
    if (!lipo.begin())
    {
        Serial.println("Error: Unable to communicate with BQ27441.");
        Serial.println("  Check wiring and try again.");
        Serial.println("  (Battery must be plugged into Battery Babysitter!)");
        while (1) ;
    }
    Serial.println("Connected to BQ27441!");
    lipo.setCapacity(capacity);
}

int batteryGetCharge(){
    return int(lipo.soc());
}



//Shift Register Logic, for LED Charge % Matrix
static void sendByte(uint8_t value) {
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(SR_DATA, (value & 0x80) ? HIGH : LOW);  //Check MSB (if HIGH or LOW, then pick that for the pin state)
    digitalWrite(SR_CLK, HIGH);
    digitalWrite(SR_CLK, LOW);
    value <<= 1; //Shift value left
  }
  digitalWrite(SR_LATCHCLK, HIGH);    //Pushes data into output register
  digitalWrite(SR_LATCHCLK, LOW);
}

void batteryIndicatorMatrixBegin(uint32_t DataIn, uint32_t Clock, uint32_t LatchClock, uint32_t OutputEnable){
    SR_DATA = DataIn;
    SR_CLK = Clock;
    SR_LATCHCLK = LatchClock;
    SR_LEDENA = OutputEnable;
    
    pinMode(SR_DATA,   OUTPUT);
    pinMode(SR_CLK,  OUTPUT);
    pinMode(SR_LATCHCLK,  OUTPUT);
    pinMode(SR_LEDENA, OUTPUT);

    digitalWrite(SR_CLK, LOW); //Pull clock low
    digitalWrite(SR_LATCHCLK, LOW); //Pull latch clock low
    digitalWrite(SR_LEDENA, LOW);   //Disable output LEDs (Via GPIO, still forced on when charging)

    sendByte(0x00); //Send all 0s
}

void batteryIndicatorMatrixShowByte(uint8_t value){
    sendByte(value);
}

void batteryIndicatorMatrixShowCharge(){
  int chrgPercentage = batteryGetCharge();

  if (chrgPercentage > 87){
    sendByte(0b11111111);
  }
  else if (chrgPercentage > 75){
    sendByte(0b11111110);
  }
  else if (chrgPercentage > 62){
    sendByte(0b11111100);
  }
  else if (chrgPercentage > 50){
    sendByte(0b11111000);
  }
  else if (chrgPercentage > 37){
    sendByte(0b11110000);
  }
  else if (chrgPercentage > 25){
    sendByte(0b11100000);
  }
  else if (chrgPercentage > 12){
    sendByte(0b11000000);
  }
  else if (chrgPercentage > 5){
    sendByte(0b10000000);
  }
  else{
    //Basically dead
    sendByte(0b00000000);
  }
}
