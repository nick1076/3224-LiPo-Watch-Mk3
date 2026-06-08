/*
   @file    LIS2DUXS12_Pedometer.ino
   @author  STMicroelectronics  
   @brief   Example to use the LIS2DUXS12 Pedometer
 *******************************************************************************
   Copyright (c) 2022, STMicroelectronics
   All rights reserved.
   This software component is licensed by ST under BSD 3-Clause license,
   the "License"; You may not use this file except in compliance with the
   License. You may obtain a copy of the License at:
                          opensource.org/licenses/BSD-3-Clause
 *******************************************************************************
*/
#include <LIS2DUXS12Sensor.h>


LIS2DUXS12Sensor LIS2DUXS12(&Wire);

uint16_t step_count = 0;
char report[256];



void setup() {

  // Initlialize serial.
  Serial.begin(115200);
  delay(1000);

  // Initlialize Led.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.print("1");

  // Initlialize i2c.
  Wire.begin();
  Serial.print("2");
    
  // Initlialize components.
  LIS2DUXS12.begin();
  Serial.print("3");
  LIS2DUXS12.Enable_X();
  Serial.print("4");

  // Enable Pedometer.
  LIS2DUXS12.Enable_Pedometer(LIS2DUXS12_INT1_PIN);
  Serial.println("5");
}

void loop() {
  delay(1000);
  LIS2DUXS12.Get_Step_Count(&step_count);
  snprintf(report, sizeof(report), "Step counter: %d", step_count);
  Serial.println(report);

}
