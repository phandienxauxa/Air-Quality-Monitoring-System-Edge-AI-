# Hướng dẫn cài đặt và chạy hệ thống

## Yêu cầu hệ thống

| Thành phần | Yêu cầu |
|---|---|
| OS | Windows 10/11 |
| Backend | **Docker Desktop** (khuyên dùng) hoặc Python 3.11 |
| Dashboard | Trình duyệt Chrome / Edge (mở `http://localhost:8000`) |
| ESP32 firmware | VS Code + PlatformIO extension |
| RA6M5 firmware | e2 Studio + FSP (đã có sẵn trong project) |

> **Không cần Live Server.** Dashboard được serve trực tiếp bởi FastAPI tại `http://localhost:8000`.

---

## Cấu trúc thư mục

```
Air-Quality-Monitoring-System-Edge-AI-/
├── app/                    # Backend FastAPI
├── js/ css/ index.html     # Frontend (serve bởi backend, không cần Live Server)
├── simulator/              # Giả lập dữ liệu ESP32 (không cần hardware)
├── test/                   # Test tự động
├── docs/                   # Tài liệu & ảnh
│   └── renesas_iaq_table.png
├── data/                   # SQLite DB (tự tạo, mount từ Docker volume)
├── Dockerfile
├── docker-compose.yml
└── requirements.txt
```

---

## PHẦN 1 — Chạy Backend

Có 2 cách: **Docker** (khuyên dùng) hoặc **Python thủ công**.

---

### Cách 1 — Docker (khuyên dùng, không cần cài Python)

**Bước 1.1** — Cài [Docker Desktop](https://www.docker.com/products/docker-desktop) → khởi động Docker Desktop.

**Bước 1.2** — Mở CMD trong thư mục project, chạy:

```cmd
docker compose up --build
```

Lần đầu mất ~2 phút để pull image và cài dependencies. Thấy các dòng sau là OK:

```
air-quality-backend  | [DB] SQLite khởi tạo thành công tại: /app/data/air_quality.db
air-quality-backend  | [Main] Server đã sẵn sàng
air-quality-backend  | INFO:     Uvicorn running on http://0.0.0.0:8000
```

Các lần sau chỉ cần:
```cmd
docker compose up
```

Tắt server:
```cmd
docker compose down
```

---

### Cách 2 — Python thủ công

**Bước 1.1** — Cài [Python 3.11](https://www.python.org/downloads/) → tick **"Add python.exe to PATH"** khi cài.

> **Lưu ý:** Không dùng Python 3.12+ (lỗi `pydantic-core` build). Dùng đúng 3.11.

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

Mở trình duyệt: `http://localhost:8000/health` — phải thấy:

```json
{"status": "ok", "service": "Edge AI Air Quality Server v1"}
```

API docs tự động (Swagger UI): `http://localhost:8000/docs`

---

## PHẦN 2 — Mở Dashboard

Dashboard được serve **trực tiếp bởi backend** — không cần Live Server hay bước cài đặt thêm.

**Bước 2.1** — Đảm bảo backend đang chạy (Phần 1).

**Bước 2.2** — Mở trình duyệt, truy cập:

```
http://localhost:8000
```

Badge góc trên phải sẽ hiển thị **ONLINE** sau vài giây khi có thiết bị gửi dữ liệu.

---

## PHẦN 3 — Kết nối phần cứng thật (ESP32 + CK-RA6M5)

### 3.1 — Lấy IP của máy PC

```cmd
ipconfig
```

Ghi lại **IPv4 Address** (ví dụ: `192.168.1.105`).

### 3.2 — Cấu hình firmware ESP32

Mở [esp32/src/main.cpp](esp32/src/main.cpp), điền 3 dòng sau:

```cpp
const char* WIFI_SSID  = "Tên_WiFi";
const char* WIFI_PASS  = "Mật_khẩu_WiFi";
const char* WS_HOST    = "192.168.1.105";  // IP vừa lấy ở bước 3.1
```

### 3.3 — Flash firmware ESP32

Trong VS Code với PlatformIO extension:
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

| ESP32 Pin | Hướng | CK-RA6M5 Pin |
|:---:|:---:|:---:|
| GPIO16 (RX2) | ←── TX | P707 (UART3 TX) |
| GPIO17 (TX2) | ──→ RX | P706 (UART3 RX) |
| GND | ↔ | GND |

> UART: 115200 baud, 8N1. Nối GND chung giữa 2 board.

### 3.5 — Chạy firmware CK-RA6M5

Build và flash qua e2 Studio. Kit sẽ tự đọc ZMOD4410 + HS3001 và gửi JSON qua UART.

> **ZMOD4410 cần warm-up ~60 giây.** ESP32 sẽ in `[INFO] ZMOD4410 đang warm-up` — đây là bình thường. Sau khi stable, Serial Monitor in mỗi 3 giây:

```
[TX] IAQ=2.81 TVOC=0.742 eCO2=615 eToH=0.052 T=29.4 H=66.8 Pred=2.85
[WS] ACK: {"status":"ok","device_id":"esp32_gateway_01"}
```

### Thang IAQ Renesas

![Renesas IAQ Rating Table](docs/renesas_iaq_table.png)

| IAQ Rating | Level | Air Quality | TVOC (mg/m³) |
|:---:|:---:|:---:|:---:|
| ≤ 1.9 | Level 1 | Rất tốt (Very Good) | < 0.3 |
| 2.0–2.9 | Level 2 | Tốt (Good) | 0.3–1.0 |
| 3.0–3.9 | Level 3 | Trung bình (Medium) | 1.0–3.0 |
| 4.0–4.9 | Level 4 | Kém (Poor) | 3.0–10.0 |
| ≥ 5.0 | Level 5 | Rất kém (Bad) | > 10.0 |

---

## PHẦN 4 — Chạy Simulator (không cần phần cứng)

Dùng simulator để test dashboard khi không có kit phần cứng.

**Terminal 1** — Backend đang chạy (Phần 1 đã làm).

**Terminal 2** — Chạy simulator:

**Nếu dùng Docker (cách 1):**
```cmd
python simulator/send_mock_data.py --url ws://localhost:8000/ws/ingest
```
> Cần Python cài trên máy host. Nếu chưa có, dùng `.venv` từ cách 2.

**Nếu dùng Python thủ công (cách 2):**
```cmd
.venv\Scripts\activate
python simulator/send_mock_data.py
```

Output mỗi giây:

```
[Simulator] ✓ Đã kết nối thành công tới ws://localhost:8000/ws/ingest
[Sim] tick=0001 | IAQ=  2.50 | VOC=0.80 | eCO2=  580 | Temp=29.0°C | Hum=65.0% | AQ=Tốt → Pred=Tốt
[Sim] tick=0002 | IAQ=  2.52 | VOC=0.78 | eCO2=  577 | ...
```

Simulator sẽ tự động tạo các bad cycle (IAQ tăng cao) và random spike để test alert engine.

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
✓ TC01 PASS — Valid payload ingestion
✓ TC02 PASS — GET /api/current
✓ TC03 PASS — GET /api/status
✓ TC04 PASS — GET /api/history
✓ TC05 PASS — GET /api/logs
✓ TC06 PASS — Invalid payload rejection
✓ TC07 PASS — Real-time broadcast
✓ TC08 PASS — Offline detection (manual)
✓ TC09 PASS — Alert engine
✓ TC10 PASS — Stability (10 payloads)

✓ TẤT CẢ TEST PASS — Hệ thống sẵn sàng demo!
```

---

## Video Nghiệm Thu

<!-- Hướng dẫn thêm video vào README:

     CÁCH 1 — YouTube (khuyên dùng):
     1. Upload video lên YouTube (có thể để "Unlisted")
     2. Copy VIDEO_ID từ URL: youtube.com/watch?v=VIDEO_ID
     3. Thay YOUR_VIDEO_ID trong README.md

     CÁCH 2 — GitHub-hosted video:
     1. Vào tab Issues của repo → New Issue
     2. Kéo file .mp4 vào ô comment → GitHub sẽ upload và tạo URL
     3. Copy URL dạng: https://github.com/user-attachments/assets/xxx.mp4
     4. Dán vào README dưới dạng:
        <video src="URL" controls width="100%"></video>
-->

[![Demo Video](https://img.youtube.com/vi/YOUR_VIDEO_ID/maxresdefault.jpg)](https://youtube.com/watch?v=YOUR_VIDEO_ID)

---

## Xử lý lỗi thường gặp

| Lỗi | Nguyên nhân | Cách fix |
|---|---|---|
| `pydantic-core` build failed | Python 3.12+, thiếu link.exe | Dùng Python **3.11** hoặc **Docker** |
| Dashboard không load | Backend chưa chạy | Kiểm tra `http://localhost:8000/health` |
| Badge **OFFLINE** mãi | Không có thiết bị gửi data | Chạy simulator (Phần 4) |
| ESP32 `[WS] Disconnected` | Sai IP hoặc backend chưa chạy | Kiểm tra `WS_HOST` và Phần 1 |
| Port 8000 bị từ chối từ ESP32 | Firewall Windows | Chạy lệnh bên dưới |
| `Address already in use` | Port đang được dùng | Tắt process cũ hoặc đổi port |
| Data mất sau khi restart Docker | Volume chưa mount đúng | Kiểm tra `./data:/app/data` trong compose |

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
