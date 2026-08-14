
#include "battery.h"

#include <Arduino.h>
#include <SparkFunBQ27441.h>






void battery::begin(int capacity){
    if (!lipo.begin())
    {
        Serial.println("Error: Unable to communicate with BQ27441.");
        return;
    }

    Serial.println("Connected to BQ27441!");
    lipo.setCapacity(capacity);
    
}

void battery::initIndicator(uint32_t DataIn, uint32_t Clock, uint32_t LatchClock, uint32_t OutputEnable){
  indicator.begin(DataIn, Clock, LatchClock, OutputEnable);
}

int battery::getCharge(){
    Serial.print("Charge Status: ");
    Serial.println(lipo.soc());
    Serial.print("Charge Status, int: ");
    Serial.println(int(lipo.soc()));
    return int(lipo.soc());
}






void indicatorMatrix::begin(uint32_t DataIn, uint32_t Clock, uint32_t LatchClock, uint32_t OutputEnable){
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

    showByte(0x00); //Send all 0s
}

void indicatorMatrix::showByte(uint8_t value){
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(SR_DATA, (value & 0x80) ? HIGH : LOW);  //Check MSB (if HIGH or LOW, then pick that for the pin state)
    digitalWrite(SR_CLK, HIGH);
    digitalWrite(SR_CLK, LOW);
    value <<= 1; //Shift value left
  }
  digitalWrite(SR_LATCHCLK, HIGH);    //Pushes data into output register
  digitalWrite(SR_LATCHCLK, LOW);
}

void indicatorMatrix::showCharge(int charge){
  int chrgPercentage = charge;

  if (chrgPercentage > 95){
    showByte(0b00001111);
  }
  else if (chrgPercentage > 75){
    showByte(0b00000111);
  }
  else if (chrgPercentage > 50){
    showByte(0b00000011);
  }
  else if (chrgPercentage > 25){
    showByte(0b00000001);
  }
  else{
    //Basically dead
    showByte(0b00000000);
  }
}

void indicatorMatrix::ena(bool onOrOff){
  isEna = onOrOff;
  
  if (onOrOff){
    digitalWrite(SR_LEDENA, HIGH);
  }
  else{
    showByte(0b00000000);
    digitalWrite(SR_LEDENA, LOW);
  }
}

bool indicatorMatrix::getEna(){
  return isEna;
}
