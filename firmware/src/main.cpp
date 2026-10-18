#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "config.h"
#include "imu.h"
#include "motors.h"
#include "pid.h"
#include "web_page.h"

// Web & WebSocket Server
WebServer server(HTTP_SERVER_PORT);
WebSocketsServer webSocket(WS_SERVER_PORT);

// Trạng thái điều khiển nhận từ Web
volatile float cmdThrottle = 0.0f;
volatile float cmdRoll = 0.0f;
volatile float cmdPitch = 0.0f;
volatile float cmdYaw = 0.0f;

volatile uint32_t lastHeartbeatTime = 0;
bool clientConnected = false;

// Biến đo đạc chu kỳ vòng lặp
uint32_t lastLoopTime = 0;
uint32_t lastTelemBroadcast = 0;
uint16_t loopCounter = 0;
uint16_t measuredLoopRate = 0;
uint32_t lastRateCheckTime = 0;

int16_t currentM1 = 0, currentM2 = 0, currentM3 = 0, currentM4 = 0;

bool imuConnected = false;

void handleRoot() {
    server.send_P(200, "text/html", INDEX_HTML);
}

void parseJsonCommand(uint8_t num, uint8_t *payload, size_t length) {
    char str[length + 1];
    memcpy(str, payload, length);
    str[length] = '\0';

    // Heartbeat update
    lastHeartbeatTime = millis();

    // 1. Lệnh điều khiển Joystick: {"cmd":"ctrl","t":450,"y":0.0,"p":0.0,"r":0.0}
    if (strstr(str, "\"cmd\":\"ctrl\"")) {
        char *tPtr = strstr(str, "\"t\":");
        char *yPtr = strstr(str, "\"y\":");
        char *pPtr = strstr(str, "\"p\":");
        char *rPtr = strstr(str, "\"r\":");
        if (tPtr) cmdThrottle = atof(tPtr + 4);
        if (yPtr) cmdYaw      = atof(yPtr + 4);
        if (pPtr) cmdPitch    = atof(pPtr + 4);
        if (rPtr) cmdRoll     = atof(rPtr + 4);
    }
    // 2. Lệnh ARM / DISARM: {"cmd":"arm","val":1}
    else if (strstr(str, "\"cmd\":\"arm\"")) {
        char *valPtr = strstr(str, "\"val\":");
        if (valPtr) {
            int armVal = atoi(valPtr + 6);
            if (armVal == 1) {
                if (cmdThrottle <= (MIN_IDLE_THROTTLE + 10)) {
                    motors.setArmed(true);
                    Serial.println("[SYSTEM] >>> ĐÃ ARM ĐỘNG CƠ <<<");
                } else {
                    Serial.println("[SYSTEM] Tu choi ARM: Can ga chua ve 0!");
                }
            } else {
                motors.setArmed(false);
                Serial.println("[SYSTEM] >>> ĐÃ DISARM ĐỘNG CƠ <<<");
            }
        }
    }
    // 3. Lệnh E-STOP khẩn cấp: {"cmd":"estop"}
    else if (strstr(str, "\"cmd\":\"estop\"")) {
        motors.setArmed(false);
        motors.stopAll();
        cmdThrottle = 0;
        Serial.println("[SYSTEM] !!! EMERGENCY STOP KÍCH HOẠT !!!");
    }
    // 4. Lệnh Calib IMU: {"cmd":"calib"}
    else if (strstr(str, "\"cmd\":\"calib\"")) {
        if (!motors.isArmed()) {
            imu.calibrate(500);
        }
    }
    // 5. Lệnh Test động cơ độc lập (Stage 3): {"cmd":"test_motor","idx":1,"pwm":120}
    else if (strstr(str, "\"cmd\":\"test_motor\"")) {
        char *idxPtr = strstr(str, "\"idx\":");
        char *pwmPtr = strstr(str, "\"pwm\":");
        if (idxPtr && pwmPtr) {
            uint8_t mIdx = atoi(idxPtr + 6);
            uint16_t mPwm = atoi(pwmPtr + 6);
            motors.testSingleMotor(mIdx, mPwm);
            Serial.printf("[TEST] Motor %d chay PWM %d\n", mIdx, mPwm);
        }
    }
    // 6. Lệnh cập nhật PID: {"cmd":"pid", ...}
    else if (strstr(str, "\"cmd\":\"pid\"")) {
        char *r_ka = strstr(str, "\"r_ka\":");
        char *r_kp = strstr(str, "\"r_kp\":");
        char *r_ki = strstr(str, "\"r_ki\":");
        char *r_kd = strstr(str, "\"r_kd\":");
        if (r_ka && r_kp && r_ki && r_kd) {
            pid.setRollGains(atof(r_ka + 7), atof(r_kp + 7), atof(r_ki + 7), atof(r_kd + 7));
        }

        char *p_ka = strstr(str, "\"p_ka\":");
        char *p_kp = strstr(str, "\"p_kp\":");
        char *p_ki = strstr(str, "\"p_ki\":");
        char *p_kd = strstr(str, "\"p_kd\":");
        if (p_ka && p_kp && p_ki && p_kd) {
            pid.setPitchGains(atof(p_ka + 7), atof(p_kp + 7), atof(p_ki + 7), atof(p_kd + 7));
        }

        char *y_kp = strstr(str, "\"y_kp\":");
        char *y_ki = strstr(str, "\"y_ki\":");
        if (y_kp && y_ki) {
            pid.setYawGains(atof(y_kp + 7), atof(y_ki + 7), 0.0f);
        }
        Serial.println("[PID] Da cap nhat he so PID thanh cong qua Web!");
    }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
    switch (type) {
        case WStype_DISCONNECTED:
            Serial.printf("[WS] Client #%u ngat ket noi\n", num);
            clientConnected = false;
            motors.setArmed(false); // An toan: tu disarm khi ngat web
            break;
        case WStype_CONNECTED:
            Serial.printf("[WS] Client #%u ket noi thanh cong tu IP\n", num);
            clientConnected = true;
            lastHeartbeatTime = millis();
            break;
        case WStype_TEXT:
            parseJsonCommand(num, payload, length);
            break;
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n==========================================");
    Serial.println("   ESP32-C3 SUPERMINI FLIGHT CONTROLLER   ");
    Serial.println("==========================================");

    // 1. KHỞI TẠO ĐỘNG CƠ (LEDC PWM 20kHz)
    motors.begin();

    // 2. KHỞI TẠO CẢM BIẾN IMU MPU-6050
    Serial.print("[IMU] Dang khoi tao MPU-6050 tren SDA=6, SCL=7... ");
    imuConnected = imu.begin();
    if (!imuConnected) {
        Serial.println("THAT BAI! Kiem tra cap noi I2C tai header J3! (Drone se chay che do an toan)");
    } else {
        Serial.println("THANH CONG!");
        imu.calibrate(600);
    }

    // 3. KHỞI TẠO WI-FI ACCESS POINT
    WiFi.mode(WIFI_AP);
    IPAddress local_ip(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS, WIFI_AP_CHANNEL);
    WiFi.setSleep(false);                 // Tắt sleep sau khi AP đã bật để duy trì kết nối
    WiFi.setTxPower(WIFI_POWER_15dBm);    // Giảm nhẹ công suất phát (15dBm) để chống sụt áp nguồn (Brownout)

    Serial.printf("[WIFI] Access Point da phat: %s (Pass: %s)\n", WIFI_AP_SSID, WIFI_AP_PASS);
    Serial.printf("[WIFI] Dia chi IP truy cap Web: http://%s\n", local_ip.toString().c_str());

    // 4. KHỞI TẠO WEB SERVER & WEBSOCKET
    server.on("/", handleRoot);
    server.begin();
    webSocket.begin();
    webSocket.onEvent(webSocketEvent);

    lastLoopTime = micros();
    lastHeartbeatTime = millis();
    lastRateCheckTime = millis();
}

void loop() {
    uint32_t currentMicros = micros();

    // =========================================================================
    // VÒNG LẶP ĐIỀU KHIỂN CHÍNH (LOOP RATE 250 HZ = CHU KỲ 4000 US)
    // =========================================================================
    if ((currentMicros - lastLoopTime) >= LOOP_TIME_US) {
        float dt = (float)(currentMicros - lastLoopTime) * 0.000001f;
        lastLoopTime = currentMicros;

        // 1. Cập nhật góc đo từ IMU (nếu cảm biến sẵn sàng)
        if (imuConnected) {
            imu.update(dt);
        }
        const ImuData& imuData = imu.getData();

        // 2. Kiểm tra an toàn Failsafe
        // - Mất kết nối Web quá thời gian quy định
        if (clientConnected && (millis() - lastHeartbeatTime > FAILSAFE_TIMEOUT_MS)) {
            if (motors.isArmed()) {
                Serial.println("[FAILSAFE] Mat ket noi Web! Tu dong DISARM!");
                motors.setArmed(false);
            }
        }

        // - Chống lật ngã (Crash Tilt Protection > 60 độ)
        if (abs(imuData.rollAngle) > CRASH_TILT_LIMIT || abs(imuData.pitchAngle) > CRASH_TILT_LIMIT) {
            if (motors.isArmed()) {
                Serial.println("[FAILSAFE] Goc nghiêng qua lon! Tu dong ngat dong co!");
                motors.setArmed(false);
            }
        }

        // 3. Tính toán bộ điều khiển PID và xuất xung PWM ra 4 động cơ
        if (motors.isArmed()) {
            pid.compute(cmdRoll, cmdPitch, cmdYaw, cmdThrottle, imuData, dt,
                        currentM1, currentM2, currentM3, currentM4);
            motors.writeOutputs(currentM1, currentM2, currentM3, currentM4);
        } else if (motors.isTesting()) {
            // Đang trong chế độ kiểm tra quay thử động cơ độc lập (Stage 3)
            motors.updateTest();
        } else {
            currentM1 = currentM2 = currentM3 = currentM4 = 0;
            motors.stopAll();
        }

        loopCounter++;
    }

    // =========================================================================
    // XỬ LÝ MẠNG VÀ WEBSOCKET
    // =========================================================================
    server.handleClient();
    webSocket.loop();

    // Đo tần số loop thực tế mỗi giây
    if (millis() - lastRateCheckTime >= 1000) {
        measuredLoopRate = loopCounter;
        loopCounter = 0;
        lastRateCheckTime = millis();
    }

    // Gửi Telemetry về điện thoại mỗi 100ms (10 Hz)
    if (clientConnected && (millis() - lastTelemBroadcast >= 100)) {
        lastTelemBroadcast = millis();
        const ImuData& imuData = imu.getData();

        char jsonBuffer[128];
        snprintf(jsonBuffer, sizeof(jsonBuffer),
                 "{\"roll\":%.1f,\"pitch\":%.1f,\"armed\":%d,\"hz\":%d}",
                 imuData.rollAngle, imuData.pitchAngle,
                 motors.isArmed() ? 1 : 0, measuredLoopRate);
        webSocket.broadcastTXT(jsonBuffer);
    }
}
