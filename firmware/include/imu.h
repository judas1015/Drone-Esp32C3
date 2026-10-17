#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "config.h"

struct ImuData {
    float rollAngle;    // Góc Roll ước lượng (độ)
    float pitchAngle;   // Góc Pitch ước lượng (độ)
    float rollRate;     // Tốc độ góc trục Roll (độ/giây)
    float pitchRate;    // Tốc độ góc trục Pitch (độ/giây)
    float yawRate;      // Tốc độ góc trục Yaw (độ/giây)
    float accZ;         // Gia tốc trục Z (G)
    bool isCalibrated;
};

class IMUManager {
public:
    IMUManager();
    bool begin();
    void calibrate(uint16_t sampleCount = 600);
    void update(float dt);
    const ImuData& getData() const { return data; }
    void resetOrientation();

private:
    ImuData data;
    float gyroBiasX;
    float gyroBiasY;
    float gyroBiasZ;
    float accBiasZ;

    uint8_t mpuAddress;

    void writeRegister(uint8_t reg, uint8_t value);
    bool readRawData(int16_t &ax, int16_t &ay, int16_t &az,
                     int16_t &gx, int16_t &gy, int16_t &gz);
};

extern IMUManager imu;
