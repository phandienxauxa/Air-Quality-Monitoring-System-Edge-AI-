# Air-Quality-Monitoring-System — Edge AI

Real-time indoor air quality monitoring system using Renesas CK-RA6M5 as the sensor node, ESP32 as the IoT gateway, and a FastAPI backend with Edge AI prediction capabilities.

> **Institution:** Ho Chi Minh City University of Technology (HCMUT)

---

## Architecture

```
CK-RA6M5 (Sensor Node)          ESP32 (Gateway)          PC (Backend)
┌─────────────────────┐          ┌─────────────┐          ┌──────────────────┐
│ ZMOD4410 — IAQ/VOC  │  UART    │             │WebSocket │  FastAPI Server  │
│ HS3001   — Temp/Hum │─────────▶│  main.cpp   │─────────▶│  /ws/ingest      │
│ Edge AI  — Predict  │ 115200   │  PlatformIO │          │  SQLite DB       │
└─────────────────────┘          └─────────────┘          │  /ws/realtime    │
                                                           └────────┬─────────┘
                                                                    │ WebSocket
                                                                    ▼
                                                           ┌──────────────────┐
                                                           │  index.html      │
                                                           │  Dashboard       │
                                                           └──────────────────┘
```

---

## Features

- **Real-time monitoring** of IAQ, TVOC, eCO₂, eToH, Temperature, Humidity via WebSocket
- **5-minute AI prediction** powered by Edge AI model running on CK-RA6M5
- **Renesas 5-level IAQ rating** (Rất tốt / Tốt / Trung bình / Kém / Rất kém)
- **Interactive dashboard** with live charts, trend indicators, donut gauges, event logs
- **Alert engine** with 3 severity levels (Critical, Warning, Info)
- **Data history** with time ranges: 1h, 24h, 7d, 30d
- **Device status** monitoring (Online / Warning / Offline detection)
- **Docker support** — chạy backend không cần cài Python

---

## Project Structure

```
Air-Quality-Monitoring-System-Edge-AI-/
├── app/                        # FastAPI backend
│   ├── main.py                 # Entry point
│   ├── config.py               # Thresholds & config
│   ├── schemas.py              # Pydantic data validation
│   ├── database.py             # SQLite async setup
│   ├── crud.py                 # DB operations
│   ├── api_routes.py           # REST API endpoints
│   ├── alert_engine.py         # Alert logic
│   ├── status_monitor.py       # Online/Offline detection
│   ├── websocket_ingest.py     # ESP32 data ingestion
│   └── websocket_realtime.py   # Dashboard streaming
├── esp32/                      # ESP32 PlatformIO firmware
│   ├── src/main.cpp            # UART → WebSocket bridge
│   └── platformio.ini
├── js/                         # Frontend JavaScript
│   ├── app.js                  # WebSocket & logic
│   ├── charts.js               # Chart.js visualization
│   ├── ui.js                   # DOM updates
│   └── mock.js                 # Mock data (fallback)
├── css/                        # Styles
├── simulator/                  # Mock ESP32 data generator
│   └── send_mock_data.py
├── test/                       # Automated test suite
│   └── run_tests.py
├── data/                       # SQLite database (auto-created)
├── index.html                  # Dashboard
├── Dockerfile
├── docker-compose.yml
├── requirements.txt
└── start_server.bat            # Windows quick-start
```

---

## Tech Stack

| Layer | Technology |
|---|---|
| Sensor Node | Renesas CK-RA6M5, ZMOD4410 (IAQ/VOC), HS3001 (Temp/Hum), FSP (e2 Studio) |
| IoT Gateway | ESP32 DevKit, PlatformIO, ArduinoJson, WebSockets library |
| Backend | Python 3.11, FastAPI 0.111, Uvicorn, Pydantic v2 |
| Database | SQLite (via aiosqlite) |
| Real-time | WebSockets (dual-channel: ingest + realtime) |
| Frontend | HTML5, Vanilla JS, Chart.js |
| Container | Docker, Docker Compose |

---

## Monitored Metrics

| Metric | Unit | Scale | Description |
|---|---|---|---|
| IAQ | rating | Renesas 1.0–5.0 | Indoor Air Quality composite index |
| TVOC | mg/m³ | 0–15 | Total Volatile Organic Compounds |
| eCO₂ | ppm | 400–5000 | Equivalent CO₂ concentration |
| eToH | mg/m³ | 0–500 | Ethanol concentration |
| Temperature | °C | 15–45 | Ambient temperature (HS3001) |
| Humidity | %RH | 20–95 | Relative humidity (HS3001) |

### Renesas IAQ Rating Scale

| IAQ Rating | Level | Air Quality | TVOC (mg/m³) |
|:---:|:---:|:---:|:---:|
| ≤ 1.9 | Level 1 | Rất tốt (Very Good) | < 0.3 |
| 2.0–2.9 | Level 2 | Tốt (Good) | 0.3–1.0 |
| 3.0–3.9 | Level 3 | Trung bình (Medium) | 1.0–3.0 |
| 4.0–4.9 | Level 4 | Kém (Poor) | 3.0–10.0 |
| ≥ 5.0 | Level 5 | Rất kém (Bad) | > 10.0 |

---

## Quick Start

### Option A — Docker (recommended, không cần cài Python)

```bash
docker compose up --build
```

### Option B — Manual (Windows)

```cmd
py -3.11 -m venv .venv
.venv\Scripts\activate
pip install -r requirements.txt
uvicorn app.main:app --host 0.0.0.0 --port 8000
```

---

## API Endpoints

| Endpoint | Method | Description |
|---|---|---|
| `/health` | GET | Server health check |
| `/api/current` | GET | Latest sensor reading |
| `/api/history` | GET | Historical data (`?range=1h&metric=iaq_index`) |
| `/api/status` | GET | Device online/offline status |
| `/api/logs` | GET | Event logs (`?limit=20`) |
| `/ws/ingest` | WS | ESP32 → Server data ingestion |
| `/ws/realtime` | WS | Server → Dashboard live stream |
| `/docs` | GET | Auto-generated API documentation |

---

## Hardware Connections

| ESP32 Pin | Direction | CK-RA6M5 Pin |
|:---:|:---:|:---:|
| GPIO16 (RX2) | ← | P707 (UART3 TX) |
| GPIO17 (TX2) | → | P706 (UART3 RX) |
| GND | ↔ | GND |

---

## License

Developed for academic purposes at Ho Chi Minh City University of Technology (EE3031).
