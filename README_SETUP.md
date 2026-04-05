# Hướng dẫn cài đặt và chạy Backend Server

## Yêu cầu hệ thống
- Ubuntu (VM hoặc WSL2 trên Windows)
- Python 3.10 trở lên

---

## Bước 1 — Cài Python 3 và pip

Mở terminal Ubuntu/WSL2, chạy lần lượt:

```bash
sudo apt update
sudo apt install -y python3 python3-pip python3-venv
```

Kiểm tra cài thành công:

```bash
python3 --version
pip3 --version
```

---

## Bước 2 — Sao chép thư mục server vào máy

Giải nén file zip, bạn sẽ thấy thư mục `server/` với cấu trúc:

```
server/
├── app/
│   ├── main.py
│   ├── config.py
│   ├── schemas.py
│   ├── database.py
│   ├── crud.py
│   ├── connections.py
│   ├── alert_engine.py
│   ├── status_monitor.py
│   ├── api_routes.py
│   ├── websocket_ingest.py
│   └── websocket_realtime.py
├── simulator/
│   └── send_mock_data.py
├── data/               ← SQLite sẽ tự tạo ở đây
└── requirements.txt
```

Di chuyển vào thư mục server:

```bash
cp -r /mnt/hgfs/server ~/
cd ~/server
```

---

## Bước 3 — Tạo môi trường ảo Python

```bash
python3 -m venv venv
source venv/bin/activate
```

> Sau lệnh này, terminal sẽ hiển thị `(venv)` ở đầu dòng — nghĩa là đã vào môi trường ảo.

---

## Bước 4 — Cài thư viện

```bash
pip install -r requirements.txt
```

---

## Bước 5 — Chạy server

```bash
uvicorn app.main:app --host 0.0.0.0 --port 8000 --reload
```

Kết quả mong đợi:

```
[DB] SQLite khởi tạo thành công tại: data/air_quality.db
[Monitor] Status monitor đã khởi động
[Main] Server đã sẵn sàng
INFO:     Uvicorn running on http://0.0.0.0:8000
```

Mở trình duyệt, truy cập: **http://localhost:8000/health**

Phải thấy:
```json
{"status": "ok", "service": "Edge AI Air Quality Server v1"}
```

---

## Bước 6 — Chạy simulator (terminal mới)

Mở thêm một terminal Ubuntu/WSL2, vào lại thư mục server:

```bash
cd  /mnt/hgfs/server
source venv/bin/activate
python simulator/send_mock_data.py
```

Terminal sẽ in dữ liệu mỗi giây:

```
[Simulator] ✓ Đã kết nối thành công
[Sim] tick=0001 | IAQ=  70.3 | VOC=2.01 | eCO2=  583 | Temp=29.1°C | Hum=64.8% | AQ=Trung bình  → Pred=Trung bình
[Sim] tick=0002 | IAQ=  71.8 | VOC=2.05 | eCO2=  591 | ...
```

---

## Bước 7 — Kiểm tra API

Mở trình duyệt hoặc dùng curl:

```bash
# Dữ liệu mới nhất
curl http://localhost:8000/api/current

# Lịch sử 1 giờ
curl "http://localhost:8000/api/history?range=1h&metric=iaq_index"

# Trạng thái thiết bị
curl http://localhost:8000/api/status

# Log sự kiện
curl http://localhost:8000/api/logs
```

---

## Bước 8 — Kết nối Dashboard với backend thật

Mở file `dashboard/js/app.js`, tìm và sửa:

```javascript
// Comment dòng này lại:
// initMockRealtime();

// Bỏ comment dòng này:
initWebSocket();
```

Sau đó mở lại dashboard bằng Live Server — dữ liệu sẽ đến từ backend thật.

---

## Xử lý lỗi thường gặp

**Lỗi `ModuleNotFoundError`:**
Kiểm tra đã vào `(venv)` chưa, nếu chưa chạy lại `source venv/bin/activate`

**Lỗi `Address already in use`:**
```bash
fuser -k 8000/tcp
```

**Simulator báo "Không kết nối được":**
Kiểm tra server đang chạy chưa, đúng port chưa

**Dashboard không nhận data:**
Mở DevTools (F12) → Console — kiểm tra lỗi WebSocket

---

## Tài liệu API tự động

FastAPI tự sinh docs tại: **http://localhost:8000/docs**
