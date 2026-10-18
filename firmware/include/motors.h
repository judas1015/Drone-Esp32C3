#pragma once

#include <Arduino.h>
#include "config.h"

class MotorManager {
public:
    MotorManager();
    void begin();
    void setArmed(bool armed);
    bool isArmed() const { return armed; }
    
    // Ghi công suất PWM (0 đến PWM_MAX_VALUE) ra 4 động cơ
    void writeOutputs(int16_t m1, int16_t m2, int16_t m3, int16_t m4);
    
    // Dừng khẩn cấp toàn bộ động cơ về 0
    void stopAll();
    
    // Hàm test từng động cơ độc lập (phục vụ Stage 3)
    void testSingleMotor(uint8_t motorIndex, uint16_t pwmValue, uint32_t durationMs = 3000);
    bool isTesting() const { return testing; }
    void updateTest();

private:
    bool armed;
    bool testing;
    uint8_t testIndex;
    uint16_t testPwm;
    uint32_t testEndTime;

    void writeChannel(uint8_t channel, uint8_t pin, uint16_t value);
};

extern MotorManager motors;
