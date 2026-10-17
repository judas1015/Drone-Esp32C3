#pragma once

#include <Arduino.h>
#include "config.h"
#include "imu.h"

struct PidGains {
    float kpAngle;
    float kpRate;
    float kiRate;
    float kdRate;
};

class PIDController {
public:
    PIDController();
    void reset();

    // Cập nhật hệ số PID từ giao diện Web
    void setRollGains(float kpAngle, float kpRate, float kiRate, float kdRate);
    void setPitchGains(float kpAngle, float kpRate, float kiRate, float kdRate);
    void setYawGains(float kpRate, float kiRate, float kdRate);

    // Tính toán motor outputs từ lệnh điều khiển và trạng thái IMU
    void compute(float desiredRollAngle, float desiredPitchAngle, float desiredYawRate,
                 float targetThrottle, const ImuData& imuData, float dt,
                 int16_t &m1, int16_t &m2, int16_t &m3, int16_t &m4);

    PidGains getRollGains() const { return rollGains; }
    PidGains getPitchGains() const { return pitchGains; }
    PidGains getYawGains() const { return yawGains; }

private:
    PidGains rollGains;
    PidGains pitchGains;
    PidGains yawGains;

    float iTermRoll;
    float iTermPitch;
    float iTermYaw;

    float prevRateErrorRoll;
    float prevRateErrorPitch;
    float prevRateErrorYaw;

    float clamp(float value, float minVal, float maxVal);
};

extern PIDController pid;
