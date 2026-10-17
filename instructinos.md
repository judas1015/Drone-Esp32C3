# NHIỆM VỤ: ĐÁNH GIÁ TÍNH KHẢ THI VÀ TÌM FIRMWARE NỀN CHO DRONE ESP32-C3

## 1. Bối cảnh dự án

Tôi đang phát triển một drone mini tự chế. Tôi đã có sơ đồ kết nối phần cứng và một tài liệu chứa mã nguồn, tài liệu kỹ thuật tham khảo.

Các tài liệu trong workspace:

- `drone.pdf`: Sơ đồ schematic của mạch drone do tôi thiết kế. Đây là nguồn tham chiếu chính để xác định kết nối điện, GPIO, linh kiện và cấu trúc phần cứng.
- `source.md`: Tài liệu tham khảo chứa các mã nguồn, firmware hoặc thông tin kỹ thuật liên quan đến drone. Hãy đọc toàn bộ nội dung có liên quan, xác định nguồn gốc và đánh giá khả năng tái sử dụng.

Nếu workspace có thêm source code, cấu hình build, thư viện hoặc tài liệu liên quan, hãy kiểm tra chúng trước khi kết luận.

## 2. Mục tiêu chính

Mục tiêu trước mắt KHÔNG phải phát triển một flight controller hoàn chỉnh từ đầu.

Tôi muốn tìm một firmware nền có khả năng tương thích với phần cứng hiện tại, build thành công và có cơ sở kỹ thuật để tiến tới thử bay thực tế.

Quy trình phát triển mong muốn:

1. Đánh giá tính khả thi của phần cứng hiện tại.
2. Tìm và lựa chọn firmware tham khảo phù hợp nhất.
3. Đối chiếu firmware với schematic và linh kiện thực tế.
4. Xác định các sửa đổi tối thiểu cần thiết để firmware có thể hoạt động với phần cứng này.
5. Xây dựng và kiểm thử theo từng giai đoạn, bắt đầu từ kiểm tra an toàn, cảm biến và động cơ.
6. Chỉ thử bay sau khi các điều kiện an toàn và chức năng điều khiển đã được xác minh.
7. Sau khi có firmware nền hoạt động đáng tin cậy, tôi sẽ tự nghiên cứu và phát triển thêm các thuật toán điều khiển, PID và tính năng nâng cao.

Ưu tiên tái sử dụng một firmware đã tồn tại và có bằng chứng kiểm chứng. Không tự viết lại toàn bộ hệ thống nếu chưa chứng minh rằng các giải pháp hiện có không tương thích.

## 3. Giai đoạn A — Kiểm tra và lập bản đồ phần cứng

Đọc `drone.pdf` và trích xuất cấu hình phần cứng thực tế.

Hãy lập bảng gồm:

| Hạng mục | Thông tin cần xác minh |
|---|---|
| MCU | Model chính xác, kiến trúc, giới hạn tài nguyên |
| GPIO | Chức năng từng chân, chân điều khiển động cơ và cảm biến |
| IMU | Model cảm biến, giao tiếp, địa chỉ I2C/SPI và ánh xạ trục |
| Động cơ | Loại động cơ, điện áp, dòng điện và phương thức điều khiển |
| Driver | MOSFET hoặc mạch điều khiển động cơ, sơ đồ đóng cắt |
| Nguồn | Pin, điện áp, đường cấp nguồn MCU và động cơ |
| Giao tiếp | Receiver, USB, UART, I2C, SPI hoặc giao tiếp khác nếu có |
| Phần còn lại | LED, camera và các linh kiện phụ trợ nếu có |

Không được suy đoán model linh kiện chỉ từ hình dạng ký hiệu nếu schematic không thể hiện đủ thông tin.

Đối chiếu từng GPIO với net label và kết nối thực tế. Phân biệt rõ:
- Thông tin được xác nhận từ schematic.
- Thông tin suy ra từ datasheet.
- Thông tin chưa thể xác minh nếu chưa kiểm tra PCB hoặc phần cứng.

Nếu PDF có nhiều trang hoặc sơ đồ phức tạp, hãy phóng to và kiểm tra các vùng quan trọng để không bỏ sót nhãn chân, cực tính hoặc kết nối nguồn.

## 4. Giai đoạn B — Phân tích source.md và tìm firmware phù hợp

Đọc `source.md` để xác định:

- Từng dự án hoặc firmware được đề cập.
- Repository, phiên bản, giấy phép và nguồn gốc nếu có thông tin.
- MCU và phần cứng mà firmware được thiết kế để hỗ trợ.
- Flight controller architecture.
- Driver động cơ, driver IMU và thư viện phụ thuộc.
- Thuật toán ước lượng tư thế, sensor fusion, PID và motor mixing.
- Cơ chế nhận lệnh điều khiển và arm/disarm.
- Tình trạng bảo trì, khả năng build và bằng chứng thử nghiệm thực tế.

Không mặc định firmware trong `source.md` tương thích chỉ vì cùng sử dụng ESP32 hoặc có tên dự án tương tự.

Nếu cần tìm thêm trên Internet, ưu tiên repository gốc, tài liệu chính thức, datasheet và issue kỹ thuật có thể kiểm chứng. Ghi lại URL, phiên bản hoặc commit cụ thể khi có thể.

Nếu `source.md` chỉ là tài liệu trích dẫn mà không chứa mã nguồn đầy đủ, hãy nói rõ điều đó. Không được coi một đoạn code mẫu là firmware hoàn chỉnh.

## 5. Giai đoạn C — Đánh giá khả năng tương thích

Lập ma trận tương thích giữa phần cứng hiện tại và từng ứng viên firmware.

Đánh giá tối thiểu:

- MCU, kiến trúc và toolchain.
- GPIO và khả năng tạo tín hiệu PWM.
- Phương thức điều khiển động cơ.
- Driver và giao tiếp IMU.
- Tần số lấy mẫu cảm biến và chu kỳ điều khiển.
- Sensor fusion và ánh xạ trục.
- PID và motor mixing cho cấu hình bốn động cơ.
- Receiver hoặc phương thức nhập lệnh.
- Arm/disarm, failsafe và hành vi khi khởi động.
- Bộ nhớ, hiệu năng và giới hạn thời gian thực.
- Khả năng build, nạp firmware và debug.

Phân loại từng hạng mục thành:
- **PASS:** Đã có bằng chứng xác nhận tương thích.
- **PARTIAL:** Có thể tương thích nhưng cần chỉnh sửa hoặc kiểm thử.
- **FAIL:** Không tương thích theo bằng chứng hiện có.
- **UNKNOWN:** Chưa đủ dữ liệu để kết luận.

Không đánh đồng build thành công với khả năng điều khiển bay thành công.

Đặc biệt, phải xác minh firmware có hỗ trợ đúng loại động cơ và mạch driver của tôi hay không. Không được mặc định firmware dành cho động cơ brushless dùng ESC sẽ hoạt động với động cơ brushed DC điều khiển bằng MOSFET, hoặc ngược lại.

## 6. Giai đoạn D — Điều tra vấn đề an toàn hiện tại

Một vấn đề đã xuất hiện trong quá trình phát triển: cả bốn động cơ có hiện tượng chạy ngay khi cấp nguồn, kể cả khi chưa nạp firmware. Hiện tượng này cần được xác minh độc lập với firmware.

Hãy điều tra các khả năng như:

- GPIO ở trạng thái không xác định trong quá trình khởi động.
- Thiếu hoặc sai điện trở kéo xuống cổng MOSFET.
- MOSFET bị hỏng hoặc chập Drain–Source.
- Đấu nối sai giữa Gate, Drain, Source và đường nguồn.
- Đường nguồn động cơ hoặc mass chung có vấn đề.
- Mạch điều khiển động cơ không bảo đảm trạng thái OFF khi MCU chưa khởi tạo.
- Sai khác giữa schematic và mạch thực tế.

Không được kết luận nguyên nhân khi chưa có bằng chứng.

Đưa ra quy trình kiểm tra an toàn theo từng bước. Mọi thử nghiệm ban đầu phải tháo toàn bộ cánh quạt. Ưu tiên kiểm tra bằng quan sát sơ đồ, đồng hồ đo và thử nghiệm không tải trước khi cho phép động cơ hoạt động.

Firmware thử nghiệm phải bảo đảm đầu ra điều khiển động cơ ở trạng thái OFF ngay từ giai đoạn khởi tạo sớm nhất có thể. Tuy nhiên, không được coi firmware là biện pháp duy nhất để khắc phục lỗi phần cứng khiến động cơ tự chạy trước khi MCU khởi tạo.

Nếu phần cứng chưa an toàn, dừng quy trình tiến tới thử bay.

## 7. Giai đoạn E — Chọn phương án tối ưu

Sau khi hoàn tất phân tích, xếp hạng các phương án theo:

1. Mức độ tương thích với phần cứng hiện tại.
2. Bằng chứng firmware đã hoạt động hoặc được thử nghiệm.
3. Khối lượng sửa đổi cần thiết.
4. Độ phức tạp của việc build và debug.
5. Độ an toàn và khả năng kiểm thử từng thành phần.
6. Khả năng mở rộng để tôi tiếp tục phát triển firmware sau này.

Đưa ra tối đa ba phương án:

- Phương án ưu tiên: firmware có khả năng tái sử dụng cao nhất.
- Phương án dự phòng: nếu phương án ưu tiên không khả thi.
- Phương án thay đổi phần cứng: nếu phần cứng hiện tại không phù hợp với các firmware khả dụng.

Giải thích cụ thể vì sao chọn hoặc loại từng phương án.

Nếu không có firmware nào phù hợp trực tiếp, hãy chỉ rõ trở ngại nằm ở phần cứng, driver, kiến trúc phần mềm hay thiếu dữ liệu. Không ép buộc chọn một firmware chỉ để có kết quả.

## 8. Giai đoạn F — Kế hoạch triển khai có cổng kiểm chứng

Xây dựng lộ trình theo thứ tự:

**Stage 0 — Hardware safety**
- Xác minh nguồn, driver động cơ, trạng thái OFF và lỗi tự chạy động cơ.
- Không lắp cánh quạt.

**Stage 1 — Build and flash**
- Thiết lập toolchain và dependency phù hợp.
- Build firmware nguyên bản hoặc bản sửa đổi tối thiểu.
- Ghi lại lệnh build, phiên bản công cụ và lỗi còn tồn tại.
- Nạp firmware và xác minh khởi động.

**Stage 2 — IMU validation**
- Xác nhận giao tiếp cảm biến.
- Kiểm tra giá trị đo, bias, chiều trục và hướng thay đổi khi nghiêng drone.
- Chưa cho phép động cơ quay.

**Stage 3 — Motor validation**
- Kiểm tra riêng từng động cơ, đúng thứ tự M1–M4.
- Xác minh chiều quay, phản ứng với tín hiệu điều khiển và khả năng dừng.
- Đối chiếu vị trí động cơ, hướng quay và cách lắp cánh quạt theo cấu hình firmware.
- Tiếp tục tháo cánh quạt.

**Stage 4 — Control validation**
- Xác minh motor mixing, dấu của các trục và phản ứng của bộ điều khiển.
- Kiểm tra arm/disarm và failsafe.
- Chỉ chuyển bước khi có tiêu chí nghiệm thu cụ thể.

**Stage 5 — Flight readiness**
- Đánh giá lại toàn bộ hệ thống, nguồn, khối lượng, lực đẩy, độ ổn định và failsafe.
- Xác định các phép thử cần thiết trước lần bay đầu tiên.
- Chỉ thử bay khi tất cả điều kiện an toàn đã đạt.

Mỗi stage phải nêu rõ:
- Mục tiêu.
- Thay đổi dự kiến.
- Các bước thực hiện.
- Kết quả mong đợi.
- Tiêu chí PASS/FAIL.
- Điều kiện bắt buộc để được chuyển sang stage tiếp theo.
- Phương án quay lui nếu thử nghiệm thất bại.

## 9. Quy tắc làm việc trong workspace

Ở giai đoạn đánh giá hiện tại:

- Chỉ đọc và phân tích các tài liệu, mã nguồn và cấu hình cần thiết.
- Không tự ý sửa, xóa hoặc ghi đè file dự án.
- Không tự ý thay đổi firmware đang dùng.
- Không tự động build, flash, cấp nguồn hoặc chạy thử động cơ trước khi báo cáo và được tôi đồng ý.
- Không tải và đưa một firmware bên ngoài vào dự án mà chưa giải thích nguồn gốc, giấy phép, thay đổi cần thiết và rủi ro.
- Nếu cần tạo file báo cáo, hãy đề xuất tên và nội dung trước; chỉ tạo sau khi xác định rõ phạm vi.

Sau khi báo cáo đánh giá được hoàn thành, hãy chờ tôi xác nhận phương án triển khai trước khi bắt đầu chỉnh sửa code.

## 10. Định dạng báo cáo bắt buộc

Trả kết quả bằng tiếng Việt, theo cấu trúc:

1. **Executive summary:** Kết luận sơ bộ về tính khả thi.
2. **Hardware inventory:** Bảng linh kiện và kết nối được xác minh từ `drone.pdf`.
3. **Source analysis:** Các firmware và nguồn tham khảo được tìm thấy trong `source.md`.
4. **Compatibility matrix:** Đối chiếu phần cứng với từng ứng viên.
5. **Critical risks:** Các lỗi hoặc rủi ro có thể ngăn drone hoạt động an toàn.
6. **Feasibility verdict:** Phân loại:
   - Khả thi với phần cứng hiện tại.
   - Khả thi sau khi sửa đổi phần cứng hoặc firmware.
   - Chưa đủ dữ liệu để kết luận.
   - Không khả thi với các ràng buộc hiện tại.
7. **Recommended firmware:** Phương án ưu tiên và lý do lựa chọn.
8. **Implementation roadmap:** Các stage cùng tiêu chí PASS/FAIL.
9. **Unresolved questions:** Những thông tin tôi cần cung cấp hoặc đo kiểm.
10. **Next action:** Chỉ nêu bước tiếp theo có giá trị cao nhất, chưa triển khai nếu chưa được tôi chấp thuận.

Mọi kết luận quan trọng phải chỉ rõ nguồn chứng cứ: tên file, trang schematic, đường dẫn file code, tài liệu kỹ thuật hoặc URL tham khảo.

Phân biệt rõ giữa:
- Đã xác minh.
- Suy luận kỹ thuật.
- Giả định chưa kiểm chứng.

Không bịa thông số, không tự điền các chân còn thiếu và không khẳng định drone có thể bay chỉ vì code biên dịch thành công.

**Yêu cầu cuối cùng:** Hãy bắt đầu bằng việc đọc và phân tích tài liệu, đánh giá tính khả thi và đưa ra báo cáo. Chưa viết firmware mới, chưa thay đổi code và chưa thực hiện bất kỳ thao tác điều khiển phần cứng nào.