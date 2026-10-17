# HƯỚNG DẪN BUILD VÀ VẬN HÀNH FIRMWARE DRONE ESP32-C3 SUPERMINI

Firmware được thiết kế chuẩn hoá riêng cho bo mạch vi điều khiển **ESP32-C3 SuperMini** (RISC-V 32-bit @ 160MHz) kết hợp 4 động cơ chổi than DC (Brushed Coreless Motors) điều khiển qua MOSFET logic-level, cảm biến góc nghiêng **MPU-6050** qua I2C và điều khiển trực tiếp bằng điện thoại thông minh qua Wi-Fi Web Controller.

---

## 1. Cấu trúc thư mục dự án

```text
firmware/
├── platformio.ini         # Cấu hình PlatformIO cho ESP32-C3 SuperMini
├── include/
│   ├── config.h          # Ánh xạ chân GPIO, thông số PWM, PID và Wi-Fi
│   ├── imu.h             # Giao tiếp MPU-6050 & bộ lọc góc Complementary Filter
│   ├── motors.h          # Driver xung LEDC PWM 20kHz cho 4 MOSFET
│   ├── pid.h             # Bộ điều khiển góc phân tầng (Angle & Rate PID)
│   └── web_page.h        # Giao diện Web Controller cảm ứng Joystick trên điện thoại
└── src/
    ├── main.cpp          # Vòng lặp điều khiển chính (250Hz) & WebSocket Server
    ├── imu.cpp
    ├── motors.cpp
    └── pid.cpp
```

---

## 2. Bảng ánh xạ chân GPIO (Khớp 100% sơ đồ `drone.pdf`)

| Ký hiệu trên mạch | Chân ESP32-C3 | Chức năng phần cứng |
|---|---|---|
| `T5` (Gate) | **GPIO 5** (nhãn `GIO5`) | Động cơ 1: Phía trước bên trái (Front-Left, CW) |
| `T6` (Gate) | **GPIO 4** | Động cơ 2: Phía trước bên phải (Front-Right, CCW) |
| `T7` (Gate) | **GPIO 3** | Động cơ 3: Phía sau bên trái (Rear-Left, CCW) |
| `T8` (Gate) | **GPIO 10** | Động cơ 4: Phía sau bên phải (Rear-Right, CW) |
| `J3 Pin 4` | **GPIO 6** | I2C SDA (Module MPU-6050) |
| `J3 Pin 3` | **GPIO 7** | I2C SCL (Module MPU-6050) |
| `5V` (SuperMini) | Đường nguồn sau LDO | Cấp nguồn pin qua LDO ($3.7V - 4.2V$) |

---

## 3. Hướng dẫn Build và Nạp Firmware bằng PlatformIO

### Bước 1: Mở dự án trong VS Code
1. Mở **Visual Studio Code**, đảm bảo đã cài extension **PlatformIO IDE**.
2. Chọn **File -> Open Folder...** và trỏ đến thư mục:
   `/Users/judas/Downloads/drone-32c3/firmware`
3. PlatformIO sẽ tự động nhận diện file `platformio.ini` và tải các gói thư viện cần thiết (`espressif32`, `WebSockets`).

### Bước 2: Build & Flash
1. Cắm bo mạch **ESP32-C3 SuperMini** vào máy tính qua cáp USB Type-C (cáp truyền được dữ liệu).
2. Nhấn biểu tượng dấu **✓ (Build)** ở thanh trạng thái dưới cùng của PlatformIO để biên dịch.
3. Nhấn biểu tượng mũi tên **→ (Upload)** để nạp code vào ESP32-C3.
4. Mở **Serial Monitor** (biểu tượng ổ cắm hoặc phím tắt `Ctrl+Alt+S`), đặt baudrate `115200`. Bạn sẽ thấy log khởi động:
   ```text
   ==========================================
      ESP32-C3 SUPERMINI FLIGHT CONTROLLER   
   ==========================================
   [MOTORS] LEDC PWM khoi tao thanh cong o 20kHz, 10-bit.
   [IMU] Dang khoi tao MPU-6050 tren SDA=6, SCL=7... THANH CONG!
   [WIFI] Access Point da phat: ESP32-C3-DRONE
   [WIFI] Dia chi IP truy cap Web: http://192.168.4.1
   ```

---

## 4. Hướng dẫn kết nối và điều khiển bằng điện thoại

1. **Bật Wi-Fi trên điện thoại:** Tìm mạng Wi-Fi tên **`ESP32-C3-DRONE`**, nhập mật khẩu: **`12345678`**.
2. **Mở trình duyệt web:** (Safari, Chrome hoặc Cốc Cốc), nhập địa chỉ:
   ```text
   http://192.168.4.1
   ```
3. Giao diện điều khiển cảm ứng chuyên dụng sẽ hiện ra:
   - **Joystick Trái:** Điều khiển Ga (Throttle - kéo lên/xuống) và Xoay hướng (Yaw - gạt trái/phải).
   - **Joystick Phải:** Điều khiển Tiến/Lùi (Pitch) và Nghiêng trái/phải (Roll).
   - **Nút ARM:** Kích hoạt quay động cơ (yêu cầu cần ga đang ở mức 0).
   - **Nút E-STOP:** Dừng khẩn cấp toàn bộ động cơ lập tức.
   - **Nút CALIB IMU:** Tự động cân bằng và lấy lại mốc 0 độ cho cảm biến khi đặt trên bàn phẳng.
   - **Nút TEST MOTOR:** Mở bảng test từng động cơ độc lập để xác minh thứ tự và chiều quay.
   - **Nút CẤU HÌNH PID:** Cho phép tinh chỉnh trực tiếp các hệ số $K_p, K_i, K_d$ ngay trên màn hình.

---

## 5. Các bước kiểm tra nghiệm thu (Thực hiện đúng thứ tự)

### Stage 1: Build & Flash
- Nạp thành công code, Serial Monitor báo khởi tạo IMU và Wi-Fi thành công.

### Stage 2: IMU Validation
- Vào trang web `http://192.168.4.1`.
- Quan sát hai thông số **Roll** và **Pitch** trên thanh hiển thị:
  - Khi nghiêng mũi drone chúc xuống: Pitch thay đổi giá trị.
  - Khi nghiêng drone sang phải: Roll thay đổi giá trị.
  - Khi đặt phẳng trên bàn: Nhấn **CALIB IMU** để đưa cả 2 góc về $0.0^\circ$.

### Stage 3: Motor Validation (TUYỆT ĐỐI THÁO CÁNH QUẠT)
- Nhấn nút **TEST MOTOR** trên giao diện:
  - Bấm *Quay thử Motor 1*: Động cơ góc trước-trái (GPIO 5) phải quay nhẹ.
  - Bấm *Quay thử Motor 2*: Động cơ góc trước-phải (GPIO 4) phải quay nhẹ.
  - Bấm *Quay thử Motor 3*: Động cơ góc sau-trái (GPIO 3) phải quay nhẹ.
  - Bấm *Quay thử Motor 4*: Động cơ góc sau-phải (GPIO 10) phải quay nhẹ.
  - Xác nhận chiều quay của từng trục motor:
    * M1 (Front-Left): Quay theo chiều kim đồng hồ (CW)
    * M2 (Front-Right): Quay ngược chiều kim đồng hồ (CCW)
    * M3 (Rear-Left): Quay ngược chiều kim đồng hồ (CCW)
    * M4 (Rear-Right): Quay theo chiều kim đồng hồ (CW)
  *(Nếu chiều quay bị ngược, chỉ cần đổi 2 dây cấp nguồn của motor đó)*.

### Stage 4: Closed-Loop Control & Failsafe Validation
- Vẫn tháo cánh quạt, cầm drone trên tay.
- Bấm **ARM**. Động cơ quay nhẹ ở chế độ cầm chừng.
- Tăng nhẹ cần ga lên $\approx 20\%$.
- Nghiêng drone sang trái: Hai động cơ bên trái (M1, M3) phải tự động quay mạnh hơn để chống lại góc nghiêng.
- Thử nghiệm ngắt an toàn: Tắt Wi-Fi điện thoại $\rightarrow$ Động cơ phải tự ngắt về 0 trong vòng $0.6$ giây.

### Stage 5: Flight Readiness
- Khi tất cả 4 bước trên đều **PASS**, lắp cánh quạt đúng chiều (cánh CW vào motor CW, cánh CCW vào motor CCW).
- Tiến hành bay thử ở độ cao thấp ($30cm$) trên bề mặt đệm mềm trong phòng kín.
