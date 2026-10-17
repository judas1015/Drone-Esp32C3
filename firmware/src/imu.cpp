#include "imu.h"
#include <math.h>

#define MPU6050_ADDR         0x68
#define REG_SMPLRT_DIV       0x19
#define REG_CONFIG           0x1A
#define REG_GYRO_CONFIG      0x1B
#define REG_ACCEL_CONFIG     0x1C
#define REG_ACCEL_XOUT_H     0x3B
#define REG_PWR_MGMT_1       0x6B
#define REG_WHO_AM_I         0x75

// Độ nhạy:
// Gyro +/- 2000 deg/s -> 16.4 LSB / (deg/s)
#define GYRO_SCALE           (1.0f / 16.4f)
// Accel +/- 8g -> 4096 LSB / g
#define ACCEL_SCALE          (1.0f / 4096.0f)
#define RAD_TO_DEG_F         57.295779513f

IMUManager imu;

IMUManager::IMUManager()
    : gyroBiasX(0.0f), gyroBiasY(0.0f), gyroBiasZ(0.0f), accBiasZ(1.0f), mpuAddress(MPU6050_ADDR) {
    data.rollAngle = 0.0f;
    data.pitchAngle = 0.0f;
    data.rollRate = 0.0f;
    data.pitchRate = 0.0f;
    data.yawRate = 0.0f;
    data.accZ = 1.0f;
    data.isCalibrated = false;
}

void IMUManager::writeRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(mpuAddress);
    Wire.write(reg);
    Wire.write(value);
    Wire.endTransmission();
}

bool IMUManager::begin() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL, I2C_FREQUENCY);
    delay(50);

    // Kiểm tra WHO_AM_I
    Wire.beginTransmission(mpuAddress);
    Wire.write(REG_WHO_AM_I);
    if (Wire.endTransmission() != 0) {
        // Thử địa chỉ 0x69 nếu AD0 được nối lên 3.3V
        mpuAddress = 0x69;
        Wire.beginTransmission(mpuAddress);
        Wire.write(REG_WHO_AM_I);
        if (Wire.endTransmission() != 0) {
            return false;
        }
    }

    Wire.requestFrom(mpuAddress, (uint8_t)1);
    if (!Wire.available()) {
        return false;
    }
    uint8_t whoami = Wire.read();
    if (whoami != 0x68 && whoami != 0x70 && whoami != 0x72) {
        // Một số clone MPU6050/6500 trả về 0x70 hoặc 0x72
        Serial.printf("[IMU] Canh bao: WHO_AM_I = 0x%02X\n", whoami);
    }

    // Đánh thức MPU6050 (thoát Sleep mode, chọn Clock từ Gyro X)
    writeRegister(REG_PWR_MGMT_1, 0x01);
    delay(20);

    // Cấu hình Sample Rate Divider = 0 (1kHz)
    writeRegister(REG_SMPLRT_DIV, 0x00);

    // Cấu hình DLPF (Digital Low Pass Filter) = 3 (~42 Hz cutoff)
    // Giúp triệt tiêu xung nhiễu rung cơ học từ 4 động cơ chổi than
    writeRegister(REG_CONFIG, 0x03);

    // Cấu hình dải đo Gyro: +/- 2000 deg/s (FS_SEL = 3)
    writeRegister(REG_GYRO_CONFIG, 0x18);

    // Cấu hình dải đo Accel: +/- 8g (AFS_SEL = 2)
    writeRegister(REG_ACCEL_CONFIG, 0x10);

    delay(30);
    return true;
}

bool IMUManager::readRawData(int16_t &ax, int16_t &ay, int16_t &az,
                             int16_t &gx, int16_t &gy, int16_t &gz) {
    Wire.beginTransmission(mpuAddress);
    Wire.write(REG_ACCEL_XOUT_H);
    if (Wire.endTransmission(false) != 0) {
        return false;
    }

    // Đọc 14 bytes liên tục trong 1 phiên truyền I2C duy nhất
    uint8_t bytesReceived = Wire.requestFrom(mpuAddress, (uint8_t)14);
    if (bytesReceived != 14) {
        return false;
    }

    ax = (int16_t)(Wire.read() << 8 | Wire.read());
    ay = (int16_t)(Wire.read() << 8 | Wire.read());
    az = (int16_t)(Wire.read() << 8 | Wire.read());
    Wire.read(); Wire.read(); // Bỏ qua 2 bytes nhiệt độ (TEMP_OUT_H/L)
    gx = (int16_t)(Wire.read() << 8 | Wire.read());
    gy = (int16_t)(Wire.read() << 8 | Wire.read());
    gz = (int16_t)(Wire.read() << 8 | Wire.read());

    return true;
}

void IMUManager::calibrate(uint16_t sampleCount) {
    int32_t sumGx = 0, sumGy = 0, sumGz = 0;
    int32_t sumAz = 0;
    uint16_t validSamples = 0;

    Serial.println("[IMU] Dang hieu chuan Gyro bias... Giu drone nam tinh!");

    for (uint16_t i = 0; i < sampleCount; ++i) {
        int16_t ax, ay, az, gx, gy, gz;
        if (readRawData(ax, ay, az, gx, gy, gz)) {
            sumGx += gx;
            sumGy += gy;
            sumGz += gz;
            sumAz += az;
            validSamples++;
        }
        delay(3);
    }

    if (validSamples > 0) {
        gyroBiasX = ((float)sumGx / validSamples) * GYRO_SCALE;
        gyroBiasY = ((float)sumGy / validSamples) * GYRO_SCALE;
        gyroBiasZ = ((float)sumGz / validSamples) * GYRO_SCALE;
        accBiasZ  = ((float)sumAz / validSamples) * ACCEL_SCALE;
        data.isCalibrated = true;

        Serial.printf("[IMU] Calib thanh cong! Bias: X=%.2f, Y=%.2f, Z=%.2f (deg/s)\n",
                      gyroBiasX, gyroBiasY, gyroBiasZ);
    }
}

void IMUManager::resetOrientation() {
    data.rollAngle = 0.0f;
    data.pitchAngle = 0.0f;
}

void IMUManager::update(float dt) {
    int16_t rawAx, rawAy, rawAz;
    int16_t rawGx, rawGy, rawGz;

    if (!readRawData(rawAx, rawAy, rawAz, rawGx, rawGy, rawGz)) {
        return; // Bỏ qua nếu I2C bận hoặc timeout
    }

    // Đổi ra đơn vị vật lý (deg/s và G) và trừ bias
    float gx = (float)rawGx * GYRO_SCALE - gyroBiasX;
    float gy = (float)rawGy * GYRO_SCALE - gyroBiasY;
    float gz = (float)rawGz * GYRO_SCALE - gyroBiasZ;

    float ax = (float)rawAx * ACCEL_SCALE;
    float ay = (float)rawAy * ACCEL_SCALE;
    float az = (float)rawAz * ACCEL_SCALE;

    data.rollRate = gx;
    data.pitchRate = gy;
    data.yawRate = gz;
    data.accZ = az;

    // Tính góc Roll & Pitch từ gia tốc kế
    float accelRoll = atan2f(ay, az) * RAD_TO_DEG_F;
    float accelPitch = atan2f(-ax, sqrtf(ay * ay + az * az)) * RAD_TO_DEG_F;

    // Tích phân Gyro
    float gyroRoll = data.rollAngle + gx * dt;
    float gyroPitch = data.pitchAngle + gy * dt;

    // Bộ lọc bù (Complementary Filter):
    // 98% tin cậy tích phân Gyro trong ngắn hạn, 2% bám theo Accel để triệt tiêu trôi góc (drift)
    const float alpha = 0.985f;
    data.rollAngle = alpha * gyroRoll + (1.0f - alpha) * accelRoll;
    data.pitchAngle = alpha * gyroPitch + (1.0f - alpha) * accelPitch;
}
