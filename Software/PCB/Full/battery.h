
#pragma once
#include <Arduino.h>
#include <SparkFunBQ27441.h>



class indicatorMatrix {
    public:
        void begin(uint32_t DataIn, uint32_t Clock, uint32_t LatchClock, uint32_t OutputEnable);
        void showCharge(int charge);
        void showByte(uint8_t value);
        void ena(bool onOrOff);
        bool getEna();

    private:
        uint32_t SR_DATA;   // Data in
        uint32_t SR_CLK;   // Main register clock
        uint32_t SR_LATCHCLK;   // Storage clock
        uint32_t SR_LEDENA;   // Output enable
        bool isEna;
};



class battery {
    public:
        void begin(int capacity);
        void initIndicator(uint32_t DataIn, uint32_t Clock, uint32_t LatchClock, uint32_t OutputEnable);
        int getCharge();
        indicatorMatrix indicator;
    private:

};
