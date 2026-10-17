#include "motors.h"
#include <esp_arduino_version.h>

MotorManager motors;

MotorManager::MotorManager() : armed(false) {}

void MotorManager::begin() {
    // Luôn đảm bảo kéo chân xuống LOW trước khi khởi tạo PWM
    pinMode(PIN_MOTOR_1, OUTPUT);
    digitalWrite(PIN_MOTOR_1, LOW);
    pinMode(PIN_MOTOR_2, OUTPUT);
    digitalWrite(PIN_MOTOR_2, LOW);
    pinMode(PIN_MOTOR_3, OUTPUT);
    digitalWrite(PIN_MOTOR_3, LOW);
    pinMode(PIN_MOTOR_4, OUTPUT);
    digitalWrite(PIN_MOTOR_4, LOW);

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    // Arduino-ESP32 Core v3.x API
    ledcAttachChannel(PIN_MOTOR_1, PWM_FREQUENCY, PWM_RESOLUTION, PWM_CHANNEL_M1);
    ledcAttachChannel(PIN_MOTOR_2, PWM_FREQUENCY, PWM_RESOLUTION, PWM_CHANNEL_M2);
    ledcAttachChannel(PIN_MOTOR_3, PWM_FREQUENCY, PWM_RESOLUTION, PWM_CHANNEL_M3);
    ledcAttachChannel(PIN_MOTOR_4, PWM_FREQUENCY, PWM_RESOLUTION, PWM_CHANNEL_M4);
#else
    // Arduino-ESP32 Core v2.x API
    ledcSetup(PWM_CHANNEL_M1, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PIN_MOTOR_1, PWM_CHANNEL_M1);

    ledcSetup(PWM_CHANNEL_M2, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PIN_MOTOR_2, PWM_CHANNEL_M2);

    ledcSetup(PWM_CHANNEL_M3, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PIN_MOTOR_3, PWM_CHANNEL_M3);

    ledcSetup(PWM_CHANNEL_M4, PWM_FREQUENCY, PWM_RESOLUTION);
    ledcAttachPin(PIN_MOTOR_4, PWM_CHANNEL_M4);
#endif

    stopAll();
    Serial.println("[MOTORS] LEDC PWM khoi tao thanh cong o 20kHz, 10-bit.");
}

void MotorManager::writeChannel(uint8_t channel, uint8_t pin, uint16_t value) {
    if (value > PWM_MAX_VALUE) value = PWM_MAX_VALUE;

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
    ledcWrite(pin, value);
#else
    ledcWrite(channel, value);
#endif
}

void MotorManager::setArmed(bool armState) {
    armed = armState;
    if (!armed) {
        stopAll();
    }
}

void MotorManager::stopAll() {
    writeChannel(PWM_CHANNEL_M1, PIN_MOTOR_1, 0);
    writeChannel(PWM_CHANNEL_M2, PIN_MOTOR_2, 0);
    writeChannel(PWM_CHANNEL_M3, PIN_MOTOR_3, 0);
    writeChannel(PWM_CHANNEL_M4, PIN_MOTOR_4, 0);
}

void MotorManager::writeOutputs(int16_t m1, int16_t m2, int16_t m3, int16_t m4) {
    if (!armed) {
        stopAll();
        return;
    }

    // Giới hạn an toàn trong dải [0, PWM_MAX_VALUE]
    uint16_t pwmM1 = (m1 < 0) ? 0 : ((m1 > PWM_MAX_VALUE) ? PWM_MAX_VALUE : (uint16_t)m1);
    uint16_t pwmM2 = (m2 < 0) ? 0 : ((m2 > PWM_MAX_VALUE) ? PWM_MAX_VALUE : (uint16_t)m2);
    uint16_t pwmM3 = (m3 < 0) ? 0 : ((m3 > PWM_MAX_VALUE) ? PWM_MAX_VALUE : (uint16_t)m3);
    uint16_t pwmM4 = (m4 < 0) ? 0 : ((m4 > PWM_MAX_VALUE) ? PWM_MAX_VALUE : (uint16_t)m4);

    writeChannel(PWM_CHANNEL_M1, PIN_MOTOR_1, pwmM1);
    writeChannel(PWM_CHANNEL_M2, PIN_MOTOR_2, pwmM2);
    writeChannel(PWM_CHANNEL_M3, PIN_MOTOR_3, pwmM3);
    writeChannel(PWM_CHANNEL_M4, PIN_MOTOR_4, pwmM4);
}

void MotorManager::testSingleMotor(uint8_t motorIndex, uint16_t pwmValue) {
    // Chỉ cho phép chạy test ở mức PWM an toàn (< 250 tương đương ~24% công suất)
    if (pwmValue > 250) pwmValue = 250;
    stopAll();

    switch (motorIndex) {
        case 1: writeChannel(PWM_CHANNEL_M1, PIN_MOTOR_1, pwmValue); break;
        case 2: writeChannel(PWM_CHANNEL_M2, PIN_MOTOR_2, pwmValue); break;
        case 3: writeChannel(PWM_CHANNEL_M3, PIN_MOTOR_3, pwmValue); break;
        case 4: writeChannel(PWM_CHANNEL_M4, PIN_MOTOR_4, pwmValue); break;
        default: break;
    }
}
