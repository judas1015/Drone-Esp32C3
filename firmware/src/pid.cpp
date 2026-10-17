#include "pid.h"

PIDController pid;

PIDController::PIDController() {
    rollGains.kpAngle = DEFAULT_KP_ANGLE_ROLL;
    rollGains.kpRate  = DEFAULT_KP_RATE_ROLL;
    rollGains.kiRate  = DEFAULT_KI_RATE_ROLL;
    rollGains.kdRate  = DEFAULT_KD_RATE_ROLL;

    pitchGains.kpAngle = DEFAULT_KP_ANGLE_PITCH;
    pitchGains.kpRate  = DEFAULT_KP_RATE_PITCH;
    pitchGains.kiRate  = DEFAULT_KI_RATE_PITCH;
    pitchGains.kdRate  = DEFAULT_KD_RATE_PITCH;

    yawGains.kpAngle = 0.0f;
    yawGains.kpRate  = DEFAULT_KP_RATE_YAW;
    yawGains.kiRate  = DEFAULT_KI_RATE_YAW;
    yawGains.kdRate  = DEFAULT_KD_RATE_YAW;

    reset();
}

float PIDController::clamp(float value, float minVal, float maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

void PIDController::reset() {
    iTermRoll = 0.0f;
    iTermPitch = 0.0f;
    iTermYaw = 0.0f;
    prevRateErrorRoll = 0.0f;
    prevRateErrorPitch = 0.0f;
    prevRateErrorYaw = 0.0f;
}

void PIDController::setRollGains(float kpAngle, float kpRate, float kiRate, float kdRate) {
    rollGains.kpAngle = kpAngle;
    rollGains.kpRate  = kpRate;
    rollGains.kiRate  = kiRate;
    rollGains.kdRate  = kdRate;
}

void PIDController::setPitchGains(float kpAngle, float kpRate, float kiRate, float kdRate) {
    pitchGains.kpAngle = kpAngle;
    pitchGains.kpRate  = kpRate;
    pitchGains.kiRate  = kiRate;
    pitchGains.kdRate  = kdRate;
}

void PIDController::setYawGains(float kpRate, float kiRate, float kdRate) {
    yawGains.kpRate = kpRate;
    yawGains.kiRate = kiRate;
    yawGains.kdRate = kdRate;
}

void PIDController::compute(float desiredRollAngle, float desiredPitchAngle, float desiredYawRate,
                             float targetThrottle, const ImuData& imuData, float dt,
                             int16_t &m1, int16_t &m2, int16_t &m3, int16_t &m4) {
    // 1. Khi hạ ga sát đất (< MIN_IDLE_THROTTLE), reset tích phân để tránh quay tích lũy trên mặt đất
    if (targetThrottle < MIN_IDLE_THROTTLE) {
        reset();
        m1 = 0; m2 = 0; m3 = 0; m4 = 0;
        return;
    }

    // =========================================================================
    // 2. VÒNG ĐIỀU KHIỂN TRỤC ROLL (Góc ngoài -> Tốc độ góc trong)
    // =========================================================================
    float errorAngleRoll = desiredRollAngle - imuData.rollAngle;
    float targetRateRoll = rollGains.kpAngle * errorAngleRoll;

    float errorRateRoll = targetRateRoll - imuData.rollRate;
    float pTermRoll = rollGains.kpRate * errorRateRoll;
    
    iTermRoll += rollGains.kiRate * errorRateRoll * dt;
    iTermRoll = clamp(iTermRoll, -MAX_I_TERM, MAX_I_TERM);
    
    float dTermRoll = (dt > 0.0001f) ? (rollGains.kdRate * (errorRateRoll - prevRateErrorRoll) / dt) : 0.0f;
    prevRateErrorRoll = errorRateRoll;

    float outputRoll = pTermRoll + iTermRoll + dTermRoll;

    // =========================================================================
    // 3. VÒNG ĐIỀU KHIỂN TRỤC PITCH (Góc ngoài -> Tốc độ góc trong)
    // =========================================================================
    float errorAnglePitch = desiredPitchAngle - imuData.pitchAngle;
    float targetRatePitch = pitchGains.kpAngle * errorAnglePitch;

    float errorRatePitch = targetRatePitch - imuData.pitchRate;
    float pTermPitch = pitchGains.kpRate * errorRatePitch;
    
    iTermPitch += pitchGains.kiRate * errorRatePitch * dt;
    iTermPitch = clamp(iTermPitch, -MAX_I_TERM, MAX_I_TERM);
    
    float dTermPitch = (dt > 0.0001f) ? (pitchGains.kdRate * (errorRatePitch - prevRateErrorPitch) / dt) : 0.0f;
    prevRateErrorPitch = errorRatePitch;

    float outputPitch = pTermPitch + iTermPitch + dTermPitch;

    // =========================================================================
    // 4. VÒNG ĐIỀU KHIỂN TRỤC YAW (Tốc độ góc trực tiếp)
    // =========================================================================
    float errorRateYaw = desiredYawRate - imuData.yawRate;
    float pTermYaw = yawGains.kpRate * errorRateYaw;

    iTermYaw += yawGains.kiRate * errorRateYaw * dt;
    iTermYaw = clamp(iTermYaw, -MAX_I_TERM, MAX_I_TERM);

    float dTermYaw = (dt > 0.0001f) ? (yawGains.kdRate * (errorRateYaw - prevRateErrorYaw) / dt) : 0.0f;
    prevRateErrorYaw = errorRateYaw;

    float outputYaw = pTermYaw + iTermYaw + dTermYaw;

    // =========================================================================
    // 5. MOTOR MIXING CHO CẤU HÌNH QUAD-X
    //    M1: Front-Left (CW)   M2: Front-Right (CCW)
    //    M3: Rear-Left (CCW)    M4: Rear-Right (CW)
    // =========================================================================
    // - Khi Pitch dương (chúi mũi xuống): Cần tăng lực M1, M2 và giảm M3, M4 để ngửa lên
    // - Khi Roll dương (nghiêng phải): Cần tăng lực M2, M4 và giảm M1, M3 để đẩy sang trái
    // - Khi Yaw dương (xoay phải): Cần tăng lực động cơ quay ngược CCW (M2, M3) và giảm CW (M1, M4)
    m1 = (int16_t)(targetThrottle + outputPitch - outputRoll - outputYaw);
    m2 = (int16_t)(targetThrottle + outputPitch + outputRoll + outputYaw);
    m3 = (int16_t)(targetThrottle - outputPitch - outputRoll + outputYaw);
    m4 = (int16_t)(targetThrottle - outputPitch + outputRoll - outputYaw);

    // Đảm bảo không động cơ nào bị giảm xuống dưới 0 hoặc vượt quá ngưỡng ga an toàn
    m1 = (int16_t)clamp(m1, 0, MAX_SAFE_THROTTLE);
    m2 = (int16_t)clamp(m2, 0, MAX_SAFE_THROTTLE);
    m3 = (int16_t)clamp(m3, 0, MAX_SAFE_THROTTLE);
    m4 = (int16_t)clamp(m4, 0, MAX_SAFE_THROTTLE);
}
