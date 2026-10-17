#pragma once

#include <Arduino.h>

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>ESP32-C3 Flight Controller</title>
  <style>
    :root {
      --bg: #0d1117;
      --card: #161b22;
      --border: #30363d;
      --accent: #58a6ff;
      --danger: #f85149;
      --success: #3fb950;
      --warning: #d29922;
      --text: #f0f6fc;
      --text-muted: #8b949e;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; user-select: none; -webkit-user-select: none; touch-action: none; }
    body {
      background: var(--bg);
      color: var(--text);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
      overflow: hidden;
      height: 100vh;
      display: flex;
      flex-direction: column;
    }
    header {
      background: var(--card);
      border-bottom: 1px solid var(--border);
      padding: 8px 16px;
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 13px;
    }
    .status-badge {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      padding: 4px 10px;
      border-radius: 12px;
      font-weight: 600;
      font-size: 12px;
    }
    .badge-disarmed { background: rgba(210, 153, 34, 0.2); color: var(--warning); border: 1px solid var(--warning); }
    .badge-armed { background: rgba(63, 185, 80, 0.2); color: var(--success); border: 1px solid var(--success); }
    .badge-offline { background: rgba(248, 81, 73, 0.2); color: var(--danger); border: 1px solid var(--danger); }

    .telemetry-bar {
      background: #11151c;
      padding: 6px 16px;
      display: flex;
      justify-content: space-around;
      font-size: 12px;
      color: var(--text-muted);
      border-bottom: 1px solid var(--border);
    }
    .telem-item span { color: var(--text); font-weight: 600; font-family: monospace; }

    .main-controls {
      flex: 1;
      display: flex;
      position: relative;
    }
    .joystick-zone {
      flex: 1;
      height: 100%;
      position: relative;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    .joystick-base {
      width: 170px;
      height: 170px;
      border-radius: 50%;
      background: radial-gradient(circle, rgba(88,166,255,0.08) 0%, rgba(22,27,34,0.6) 100%);
      border: 2px solid var(--border);
      position: relative;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    .joystick-base::before, .joystick-base::after {
      content: ''; position: absolute; background: rgba(255,255,255,0.07);
    }
    .joystick-base::before { width: 100%; height: 1px; }
    .joystick-base::after { width: 1px; height: 100%; }
    .joystick-handle {
      width: 65px;
      height: 65px;
      border-radius: 50%;
      background: linear-gradient(145deg, #1f242c, #161a20);
      border: 2px solid var(--accent);
      box-shadow: 0 4px 15px rgba(0,0,0,0.5);
      position: absolute;
      top: 50%; left: 50%;
      transform: translate(-50%, -50%);
      pointer-events: none;
      transition: transform 0.05s ease-out;
    }

    .center-panel {
      width: 130px;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
      gap: 12px;
      z-index: 10;
    }
    .btn {
      width: 100%;
      padding: 10px;
      border-radius: 8px;
      border: none;
      font-weight: 700;
      font-size: 13px;
      cursor: pointer;
      display: flex;
      align-items: center;
      justify-content: center;
      transition: all 0.15s;
    }
    .btn-arm { background: var(--success); color: #fff; }
    .btn-disarm { background: var(--warning); color: #000; }
    .btn-estop { background: var(--danger); color: #fff; box-shadow: 0 0 10px rgba(248,81,73,0.4); }
    .btn-secondary { background: var(--card); color: var(--text); border: 1px solid var(--border); }

    .modal {
      display: none;
      position: fixed;
      top: 0; left: 0; width: 100%; height: 100%;
      background: rgba(0,0,0,0.7);
      backdrop-filter: blur(4px);
      z-index: 99;
      align-items: center;
      justify-content: center;
    }
    .modal-content {
      background: var(--card);
      border: 1px solid var(--border);
      border-radius: 12px;
      width: 90%;
      max-width: 420px;
      padding: 20px;
      max-height: 85vh;
      overflow-y: auto;
    }
    .modal-content h3 { margin-bottom: 15px; font-size: 16px; color: var(--accent); }
    .form-group { margin-bottom: 10px; }
    .form-group label { display: block; font-size: 12px; margin-bottom: 4px; color: var(--text-muted); }
    .form-group input { width: 100%; padding: 8px; background: #0d1117; border: 1px solid var(--border); color: #fff; border-radius: 6px; font-family: monospace; }
  </style>
</head>
<body>
  <header>
    <div><strong>ESP32-C3 DRONE</strong></div>
    <div id="armBadge" class="status-badge badge-disarmed">DISARMED</div>
  </header>

  <div class="telemetry-bar">
    <div class="telem-item">Roll: <span id="valRoll">0.0°</span></div>
    <div class="telem-item">Pitch: <span id="valPitch">0.0°</span></div>
    <div class="telem-item">Rate: <span id="valRate">250Hz</span></div>
    <div class="telem-item">Ping: <span id="valPing">--ms</span></div>
  </div>

  <div class="main-controls">
    <!-- Joystick Trái: Throttle & Yaw -->
    <div class="joystick-zone" id="zoneLeft">
      <div class="joystick-base" id="baseLeft">
        <div class="joystick-handle" id="handleLeft"></div>
      </div>
    </div>

    <!-- Cụm nút bấm trung tâm -->
    <div class="center-panel">
      <button class="btn btn-estop" id="btnEstop" onclick="sendEmergencyStop()">E-STOP</button>
      <button class="btn btn-arm" id="btnArm" onclick="toggleArm()">ARM</button>
      <button class="btn btn-secondary" onclick="sendCalibrate()">CALIB IMU</button>
      <button class="btn btn-secondary" onclick="openPidModal()">CẤU HÌNH PID</button>
      <button class="btn btn-secondary" onclick="openMotorTestModal()">TEST MOTOR</button>
    </div>

    <!-- Joystick Phải: Pitch & Roll -->
    <div class="joystick-zone" id="zoneRight">
      <div class="joystick-base" id="baseRight">
        <div class="joystick-handle" id="handleRight"></div>
      </div>
    </div>
  </div>

  <!-- Modal PID Tuning -->
  <div class="modal" id="pidModal">
    <div class="modal-content">
      <h3>Tinh chỉnh tham số PID live</h3>
      <div class="form-group"><label>Roll Angle Kp:</label><input id="pidRollKpA" type="number" step="0.1" value="3.2"></div>
      <div class="form-group"><label>Roll Rate Kp / Ki / Kd:</label>
        <div style="display:flex; gap:6px;">
          <input id="pidRollKpR" type="number" step="0.05" value="0.75">
          <input id="pidRollKiR" type="number" step="0.05" value="0.40">
          <input id="pidRollKdR" type="number" step="0.005" value="0.015">
        </div>
      </div>
      <div class="form-group"><label>Pitch Angle Kp:</label><input id="pidPitchKpA" type="number" step="0.1" value="3.2"></div>
      <div class="form-group"><label>Pitch Rate Kp / Ki / Kd:</label>
        <div style="display:flex; gap:6px;">
          <input id="pidPitchKpR" type="number" step="0.05" value="0.75">
          <input id="pidPitchKiR" type="number" step="0.05" value="0.40">
          <input id="pidPitchKdR" type="number" step="0.005" value="0.015">
        </div>
      </div>
      <div class="form-group"><label>Yaw Rate Kp / Ki:</label>
        <div style="display:flex; gap:6px;">
          <input id="pidYawKpR" type="number" step="0.1" value="2.20">
          <input id="pidYawKiR" type="number" step="0.05" value="0.25">
        </div>
      </div>
      <div style="display:flex; gap:10px; margin-top:15px;">
        <button class="btn btn-arm" onclick="savePidSettings()">Áp dụng</button>
        <button class="btn btn-secondary" onclick="closePidModal()">Đóng</button>
      </div>
    </div>
  </div>

  <!-- Modal Test Động Cơ (Stage 3) -->
  <div class="modal" id="motorTestModal">
    <div class="modal-content">
      <h3>Kiểm tra thứ tự & chiều quay động cơ</h3>
      <p style="font-size:12px; color:var(--warning); margin-bottom:12px;">CẢNH BÁO: Tháo toàn bộ cánh quạt trước khi thử nghiệm!</p>
      <div style="display:flex; flex-direction:column; gap:8px;">
        <button class="btn btn-secondary" onclick="testMotor(1)">Quay thử Motor 1 (Front-Left, GPIO 5)</button>
        <button class="btn btn-secondary" onclick="testMotor(2)">Quay thử Motor 2 (Front-Right, GPIO 4)</button>
        <button class="btn btn-secondary" onclick="testMotor(3)">Quay thử Motor 3 (Rear-Left, GPIO 3)</button>
        <button class="btn btn-secondary" onclick="testMotor(4)">Quay thử Motor 4 (Rear-Right, GPIO 10)</button>
      </div>
      <button class="btn btn-estop" style="margin-top:15px;" onclick="stopAllMotorsTest()">Dừng tất cả</button>
      <button class="btn btn-secondary" style="margin-top:8px;" onclick="closeMotorTestModal()">Đóng</button>
    </div>
  </div>

  <script>
    let ws;
    let isArmed = false;
    let ctrlThrottle = 0;
    let ctrlYaw = 0;
    let ctrlPitch = 0;
    let ctrlRoll = 0;
    let pingTimer = 0;

    function initWebSocket() {
      const wsUrl = "ws://" + window.location.hostname + ":81/";
      ws = new WebSocket(wsUrl);
      ws.onopen = () => {
        document.getElementById("armBadge").textContent = "DISARMED";
        document.getElementById("armBadge").className = "status-badge badge-disarmed";
      };
      ws.onclose = () => {
        document.getElementById("armBadge").textContent = "OFFLINE";
        document.getElementById("armBadge").className = "status-badge badge-offline";
        setTimeout(initWebSocket, 1500);
      };
      ws.onmessage = (event) => {
        try {
          const telem = JSON.parse(event.data);
          document.getElementById("valRoll").textContent = telem.roll.toFixed(1) + "°";
          document.getElementById("valPitch").textContent = telem.pitch.toFixed(1) + "°";
          document.getElementById("valRate").textContent = telem.hz + "Hz";
          document.getElementById("valPing").textContent = (Date.now() - pingTimer) + "ms";
          
          if (telem.armed !== isArmed) {
            isArmed = telem.armed;
            updateArmUI();
          }
        } catch(e) {}
      };
    }

    function sendControlLoop() {
      if (ws && ws.readyState === WebSocket.OPEN) {
        pingTimer = Date.now();
        ws.send(JSON.stringify({
          cmd: "ctrl",
          t: Math.round(ctrlThrottle),
          y: parseFloat(ctrlYaw.toFixed(1)),
          p: parseFloat(ctrlPitch.toFixed(1)),
          r: parseFloat(ctrlRoll.toFixed(1))
        }));
      }
    }
    setInterval(sendControlLoop, 40); // 25 Hz control streaming

    function toggleArm() {
      if (!isArmed) {
        if (ctrlThrottle > 50) {
          alert("Kéo cần ga về 0 trước khi ARM!");
          return;
        }
        if (confirm("Xác nhận ARM động cơ? Đảm bảo khu vực xung quanh an toàn!")) {
          ws.send(JSON.stringify({ cmd: "arm", val: 1 }));
        }
      } else {
        ws.send(JSON.stringify({ cmd: "arm", val: 0 }));
      }
    }

    function updateArmUI() {
      const badge = document.getElementById("armBadge");
      const btn = document.getElementById("btnArm");
      if (isArmed) {
        badge.textContent = "ARMED";
        badge.className = "status-badge badge-armed";
        btn.textContent = "DISARM";
        btn.className = "btn btn-disarm";
      } else {
        badge.textContent = "DISARMED";
        badge.className = "status-badge badge-disarmed";
        btn.textContent = "ARM";
        btn.className = "btn btn-arm";
      }
    }

    function sendEmergencyStop() {
      ctrlThrottle = 0;
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ cmd: "estop" }));
      }
      isArmed = false;
      updateArmUI();
    }

    function sendCalibrate() {
      if (isArmed) { alert("Phải DISARM trước khi calib!"); return; }
      if (confirm("Đặt drone cố định trên mặt phẳng tĩnh để hiệu chuẩn IMU?")) {
        ws.send(JSON.stringify({ cmd: "calib" }));
      }
    }

    // Joystick Touch Handler
    function setupJoystick(zoneId, handleId, onMove, onRelease, autoCenterY = true) {
      const zone = document.getElementById(zoneId);
      const handle = document.getElementById(handleId);
      const maxRadius = 55;
      let activeTouchId = null;
      let centerX = 0, centerY = 0;

      function updateCenter() {
        const rect = zone.getBoundingClientRect();
        centerX = rect.left + rect.width / 2;
        centerY = rect.top + rect.height / 2;
      }
      window.addEventListener("resize", updateCenter);
      updateCenter();

      zone.addEventListener("touchstart", (e) => {
        if (activeTouchId === null) {
          const touch = e.changedTouches[0];
          activeTouchId = touch.identifier;
          updateCenter();
          handleTouch(touch);
        }
      });

      window.addEventListener("touchmove", (e) => {
        for (let i = 0; i < e.touches.length; i++) {
          if (e.touches[i].identifier === activeTouchId) {
            handleTouch(e.touches[i]);
            break;
          }
        }
      });

      function endTouch(e) {
        for (let i = 0; i < e.changedTouches.length; i++) {
          if (e.changedTouches[i].identifier === activeTouchId) {
            activeTouchId = null;
            if (autoCenterY) {
              handle.style.transform = `translate(-50%, -50%) translate(0px, 0px)`;
              onRelease();
            } else {
              // Throttle: chỉ hồi tâm trục ngang X (Yaw), giữ nguyên trục dọc Y (Ga)
              const currentY = parseFloat(handle.dataset.y || 0);
              handle.style.transform = `translate(-50%, -50%) translate(0px, ${currentY}px)`;
              onMove(0, -currentY / maxRadius);
            }
            break;
          }
        }
      }
      window.addEventListener("touchend", endTouch);
      window.addEventListener("touchcancel", endTouch);

      function handleTouch(touch) {
        let dx = touch.clientX - centerX;
        let dy = touch.clientY - centerY;
        let dist = Math.sqrt(dx * dx + dy * dy);
        if (dist > maxRadius) {
          dx = (dx / dist) * maxRadius;
          dy = (dy / dist) * maxRadius;
        }
        handle.style.transform = `translate(-50%, -50%) translate(${dx}px, ${dy}px)`;
        handle.dataset.y = dy;
        // Chuẩn hoá [-1.0, 1.0] (Trục Y đảo dấu vì trên màn hình kéo lên là âm)
        onMove(dx / maxRadius, -dy / maxRadius);
      }
    }

    // Joystick Trái: X = Yaw [-180, 180] deg/s, Y = Throttle [0, 800]
    setupJoystick("zoneLeft", "handleLeft", (nx, ny) => {
      ctrlYaw = nx * 180.0;
      // ny nằm trong khoảng [-1, 1], đưa về [0, 1]
      let normalizedThrottle = (ny + 1.0) / 2.0;
      ctrlThrottle = normalizedThrottle * 800.0;
    }, () => {
      ctrlYaw = 0;
    }, false);

    // Joystick Phải: X = Roll [-30, 30] deg, Y = Pitch [-30, 30] deg (Tự hồi tâm cả 2 trục)
    setupJoystick("zoneRight", "handleRight", (nx, ny) => {
      ctrlRoll = nx * 30.0;
      ctrlPitch = ny * 30.0;
    }, () => {
      ctrlRoll = 0;
      ctrlPitch = 0;
    }, true);

    function openPidModal() { document.getElementById("pidModal").style.display = "flex"; }
    function closePidModal() { document.getElementById("pidModal").style.display = "none"; }
    function openMotorTestModal() { document.getElementById("motorTestModal").style.display = "flex"; }
    function closeMotorTestModal() { document.getElementById("motorTestModal").style.display = "none"; }

    function savePidSettings() {
      ws.send(JSON.stringify({
        cmd: "pid",
        r_ka: parseFloat(document.getElementById("pidRollKpA").value),
        r_kp: parseFloat(document.getElementById("pidRollKpR").value),
        r_ki: parseFloat(document.getElementById("pidRollKiR").value),
        r_kd: parseFloat(document.getElementById("pidRollKdR").value),
        p_ka: parseFloat(document.getElementById("pidPitchKpA").value),
        p_kp: parseFloat(document.getElementById("pidPitchKpR").value),
        p_ki: parseFloat(document.getElementById("pidPitchKiR").value),
        p_kd: parseFloat(document.getElementById("pidPitchKdR").value),
        y_kp: parseFloat(document.getElementById("pidYawKpR").value),
        y_ki: parseFloat(document.getElementById("pidYawKiR").value)
      }));
      closePidModal();
    }

    function testMotor(idx) {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ cmd: "test_motor", idx: idx, pwm: 120 }));
      }
    }
    function stopAllMotorsTest() {
      if (ws && ws.readyState === WebSocket.OPEN) {
        ws.send(JSON.stringify({ cmd: "test_motor", idx: 0, pwm: 0 }));
      }
    }

    window.onload = initWebSocket;
  </script>
</body>
</html>
)rawliteral";
