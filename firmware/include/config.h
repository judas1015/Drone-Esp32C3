#pragma once

#include <Arduino.h>

// ==============================================================================
// 1. ÁNH XẠ CHÂN PHẦN CỨNG (GPIO PINOUT - KHỚP SƠ ĐỒ drone.pdf)
// ==============================================================================
// 4 kênh PWM điều khiển Gate của MOSFET IRLR7843 (động cơ chổi than DC)
#define PIN_MOTOR_1         5   // T5 (Front-Left)  - Net GIO5 trong schematic
#define PIN_MOTOR_2         4   // T6 (Front-Right) - Net GPIO4 trong schematic
#define PIN_MOTOR_3         3   // T7 (Rear-Left)   - Net GPIO3 trong schematic
#define PIN_MOTOR_4        10   // T8 (Rear-Right)  - Net GPIO10 trong schematic

// Giao tiếp I2C cảm biến IMU (MPU-6050 nối vào header J3)
#define PIN_I2C_SDA         6   // J3 Pin 4 - SDA
#define PIN_I2C_SCL         7   // J3 Pin 3 - SCL
#define I2C_FREQUENCY  400000   // 400 kHz Fast-Mode I2C

// ==============================================================================
// 2. CẤU HÌNH XUNG PWM ĐỘNG CƠ (LEDC PERIPHERAL TRÊN ESP32-C3)
// ==============================================================================
#define PWM_FREQUENCY   20000   // 20 kHz (siêu âm, không gây rít cuộn dây động cơ)
#define PWM_RESOLUTION     10   // 10-bit: Giá trị PWM từ 0 đến 1023
#define PWM_MAX_VALUE    1023

#define PWM_CHANNEL_M1      0
#define PWM_CHANNEL_M2      1
#define PWM_CHANNEL_M3      2
#define PWM_CHANNEL_M4      3

// ==============================================================================
// 3. THÔNG SỐ VÒNG LẶP ĐIỀU KHIỂN & AN TOÀN (SAFETY & CONTROL LOOP)
// ==============================================================================
#define LOOP_RATE_HZ      250   // 250 Hz (chu kỳ 4000 us - lý tưởng cho ESP32-C3 soft-float)
#define LOOP_TIME_US   (1000000 / LOOP_RATE_HZ)

// Giới hạn ga an toàn
#define MIN_IDLE_THROTTLE  80   // Mức ga quay nhẹ cầm chừng khi đã ARM (~8%)
#define MAX_SAFE_THROTTLE 900   // Giới hạn ga tối đa (~88%) để chừa headroom cho PID thăng bằng

// Giới hạn góc nghiêng điều khiển (Angle Mode)
#define MAX_ROLL_ANGLE     30.0f  // Góc nghiêng tối đa Roll (độ)
#define MAX_PITCH_ANGLE    30.0f  // Góc nghiêng tối đa Pitch (độ)
#define MAX_YAW_RATE      180.0f  // Tốc độ xoay tối đa Yaw (độ/giây)

// Tự ngắt khẩn cấp khi gặp sự cố
#define CRASH_TILT_LIMIT   60.0f  // Tự động Disarm nếu drone bị nghiêng quá 60 độ (lật ngã)
#define FAILSAFE_TIMEOUT_MS 600   // Tự ngắt động cơ nếu mất kết nối Wi-Fi > 600 ms

// ==============================================================================
// 4. CẤU HÌNH MẠNG WI-FI ACCESS POINT (ĐIỀU KHIỂN BẰNG ĐIỆN THOẠI)
// ==============================================================================
#define WIFI_AP_SSID    "ESP32-C3-DRONE"
#define WIFI_AP_PASS    "12345678"     // Mật khẩu Wi-Fi (ít nhất 8 ký tự)
#define WIFI_AP_CHANNEL 1
#define HTTP_SERVER_PORT 80
#define WS_SERVER_PORT   81

// ==============================================================================
// 5. CẤU HÌNH HỆ SỐ PID MẶC ĐỊNH (TUNABLE QUA GIAO DIỆN WEB)
// ==============================================================================
// Bộ điều khiển góc ngoài (Outer Angle Loop)
#define DEFAULT_KP_ANGLE_ROLL   3.2f
#define DEFAULT_KP_ANGLE_PITCH  3.2f

// Bộ điều khiển tốc độ góc trong (Inner Rate Loop)
#define DEFAULT_KP_RATE_ROLL    0.75f
#define DEFAULT_KI_RATE_ROLL    0.40f
#define DEFAULT_KD_RATE_ROLL    0.015f

#define DEFAULT_KP_RATE_PITCH   0.75f
#define DEFAULT_KI_RATE_PITCH   0.40f
#define DEFAULT_KD_RATE_PITCH   0.015f

#define DEFAULT_KP_RATE_YAW     2.20f
#define DEFAULT_KI_RATE_YAW     0.25f
#define DEFAULT_KD_RATE_YAW     0.000f

#define MAX_I_TERM              120.0f  // Giới hạn chống bão hòa tích phân (Anti-windup)
