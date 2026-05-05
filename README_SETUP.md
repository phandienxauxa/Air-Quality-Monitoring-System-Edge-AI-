# Hướng dẫn cài đặt và chạy hệ thống

## Yêu cầu hệ thống

| Thành phần | Yêu cầu |
|---|---|
| OS | Windows 10/11 |
| Backend | Docker Desktop **hoặc** Python 3.11 |
| Dashboard | Trình duyệt Chrome/Edge + VS Code Live Server |
| ESP32 firmware | VS Code + PlatformIO extension |
| RA6M5 firmware | e2 Studio + FSP (đã có sẵn) |

---

## Cấu trúc thư mục

```
Air-Quality-Monitoring-System-Edge-AI-/
├── app/                    # Backend FastAPI
├── esp32/                  # Firmware ESP32 (PlatformIO)
│   ├── src/main.cpp
│   └── platformio.ini
├── js/ css/                # Frontend
├── simulator/              # Giả lập dữ liệu (không cần hardware)
├── test/                   # Test tự động
├── data/                   # SQLite DB (tự tạo khi chạy)
├── index.html              # Dashboard
├── Dockerfile
├── docker-compose.yml
└── requirements.txt
```

---

## PHẦN 1 — Chạy Backend

Có 2 cách: **Docker** (khuyên dùng) hoặc **Python thủ công**.

---

### Cách 1 — Docker (không cần cài Python)

**Bước 1.1** — Cài [Docker Desktop](https://www.docker.com/products/docker-desktop) → khởi động.

**Bước 1.2** — Mở CMD trong thư mục project, chạy:

```cmd
docker compose up --build
```

Lần đầu mất ~2 phút. Thấy dòng sau là OK:

```
air-quality-backend  | [DB] SQLite khởi tạo thành công
air-quality-backend  | [Main] Server đã sẵn sàng
air-quality-backend  | INFO:     Uvicorn running on http://0.0.0.0:8000
```

Các lần sau chỉ cần:
```cmd
docker compose up
```

---

### Cách 2 — Python thủ công

**Bước 1.1** — Cài [Python 3.11](https://www.python.org/downloads/) → tick **"Add python.exe to PATH"**.

**Bước 1.2** — Mở CMD trong thư mục project:

```cmd
py -3.11 -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
```

**Bước 1.3** — Chạy server:

```cmd
uvicorn app.main:app --host 0.0.0.0 --port 8000
```

---

### Kiểm tra backend

Mở trình duyệt tại `http://localhost:8000/health` — phải thấy:

```json
{"status": "ok", "service": "Edge AI Air Quality Server v1"}
```

API docs tự động: `http://localhost:8000/docs`

---

## PHẦN 2 — Mở Dashboard

**Bước 2.1** — Mở thư mục project trong VS Code.

**Bước 2.2** — Cài extension **Live Server** (Ritwick Dey) nếu chưa có.

**Bước 2.3** — Click chuột phải vào `index.html` → **"Open with Live Server"**.

Dashboard sẽ mở tại `http://127.0.0.1:5500/index.html`.

> Nếu backend đang chạy, badge góc phải sẽ hiển thị **ONLINE** sau vài giây.

---

## PHẦN 3 — Kết nối phần cứng thật

### 3.1 — Lấy IP của máy PC

```cmd
ipconfig
```

Ghi lại **IPv4 Address** (ví dụ: `192.168.1.105`).

### 3.2 — Cấu hình firmware ESP32

Mở [esp32/src/main.cpp](esp32/src/main.cpp), điền 3 dòng:

```cpp
const char* WIFI_SSID  = "Tên_WiFi";
const char* WIFI_PASS  = "Mật_khẩu_WiFi";
const char* WS_HOST    = "192.168.1.105";  // IP vừa lấy
```

### 3.3 — Flash ESP32

Trong VS Code với PlatformIO:
- Cắm ESP32 vào PC qua USB
- Nhấn **Upload** (`Ctrl+Alt+U`)

Mở Serial Monitor (`Ctrl+Alt+S`) — phải thấy:

```
[Init] WiFi connecting.....
[Init] IP: 192.168.1.xxx | RSSI: -55 dBm
[Init] Ready — đang chờ dữ liệu từ CK-RA6M5 qua UART...
[WS] Connected to backend
```

### 3.4 — Kết nối dây ESP32 ↔ CK-RA6M5

| ESP32 | | CK-RA6M5 |
|:---:|:---:|:---:|
| GPIO16 (RX2) | ←── TX | P707 (UART3 TX) |
| GPIO17 (TX2) | ──→ RX | P706 (UART3 RX) |
| GND | ↔ | GND |

### 3.5 — Chạy firmware RA6M5

Build và flash qua e2 Studio. Kit sẽ tự đọc ZMOD4410 + HS3001 và gửi dữ liệu qua UART.

> **Lưu ý:** ZMOD4410 cần warm-up ~60 giây. ESP32 sẽ in `[INFO] ZMOD4410 đang warm-up` trong thời gian này — đây là bình thường.

Sau khi stable, Serial Monitor ESP32 sẽ in mỗi 3 giây:

```
[TX] IAQ=2.81 TVOC=0.742 eCO2=615 eToH=0.052 T=29.4 H=66.8 Pred=2.85
[WS] ACK: {"status":"ok","device_id":"esp32_gateway_01"}
```

---

## PHẦN 4 — Chạy Simulator (không cần phần cứng)

Dùng simulator để test dashboard mà không cần kit phần cứng.

**Terminal 1** — Backend đang chạy (Phần 1).

**Terminal 2** — Chạy simulator:

```cmd
.venv\Scripts\activate
python simulator/send_mock_data.py
```

Output mỗi giây:

```
[Simulator] ✓ Đã kết nối thành công
[Sim] tick=0001 | IAQ=  2.50 | VOC=0.80 | eCO2=  580 | Temp=29.0°C | Hum=65.0% | AQ=Tốt → Pred=Tốt
[Sim] tick=0002 | IAQ=  2.52 | VOC=0.78 | eCO2=  577 | ...
```

---

## PHẦN 5 — Chạy Test Suite

Đảm bảo backend đang chạy, sau đó:

```cmd
.venv\Scripts\activate
pip install requests
python test/run_tests.py
```

Kết quả kỳ vọng:

```
✓ TẤT CẢ TEST PASS — Hệ thống sẵn sàng demo!
```

---

## Xử lý lỗi thường gặp

| Lỗi | Nguyên nhân | Cách fix |
|---|---|---|
| `pydantic-core` build failed | Python 3.14, thiếu link.exe | Dùng Python **3.11** hoặc Docker |
| Dashboard không nhận data | Live Server reload do DB thay đổi | Đã fix trong `.vscode/settings.json` |
| ESP32 `[WS] Disconnected` | Sai IP hoặc backend chưa chạy | Kiểm tra `WS_HOST` và Phần 1 |
| Port 8000 bị từ chối từ ESP32 | Firewall Windows | Chạy lệnh bên dưới |
| `Address already in use` | Port đang được dùng | Tắt process cũ hoặc đổi port |

**Mở port 8000 trên Firewall Windows:**
```cmd
netsh advfirewall firewall add rule name="FastAPI 8000" protocol=TCP dir=in localport=8000 action=allow
```

**Tắt server đang chiếm port 8000:**
```cmd
netstat -ano | findstr :8000
taskkill /PID <PID> /F
```

**Kiểm tra Docker container:**
```cmd
docker compose ps
docker compose logs -f
```
