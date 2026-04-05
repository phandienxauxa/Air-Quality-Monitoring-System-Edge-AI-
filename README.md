# Air-Quality-Monitoring-System-Edge-AI

  Real-time indoor air quality monitoring and prediction system using ESP32 as an IoT gateway and a FastAPI backend with Edge AI capabilities.

  > **Institution:** Ho Chi Minh City University of Technology

  ---

  ## Features

  - **Real-time monitoring** of IAQ, VOC, eCO₂, eToH, Temperature, and Humidity via WebSocket
  - **5-minute predictive forecasts** powered by Edge AI models
  - **Interactive dashboard** with live charts, trend indicators, donut gauges, and event logs
  - **Alert engine** with severity levels (Critical, Warning, Info)
  - **Data history** with configurable time ranges: 1h, 24h, 7d, 30d
  - **Device status** monitoring for connected ESP32 nodes
  - **Light/Dark theme** with persistence
  - **REST API** with auto-generated docs at `/docs`

  ---

  ## Architecture

  ESP32 Sensors
       │
       │  WebSocket (/ws/ingest)
       ▼
  FastAPI Backend  ──── SQLite DB
       │
       │  WebSocket (/ws/realtime)
       ▼
  Web Dashboard (index.html)

  **Backend** (`app/`): FastAPI + Uvicorn, async SQLite via aiosqlite, WebSocket dual-channel design.
  **Frontend** (`index.html`, `js/`, `css/`): Vanilla JS + Chart.js, connects directly to the backend WebSocket.
  **Simulator** (`simulator/`): Generates mock ESP32 sensor readings for development and testing.

  ---

  ## Tech Stack

  | Layer | Technology |
  |---|---|
  | Backend | Python 3.10+, FastAPI 0.111, Uvicorn, Pydantic v2 |
  | Database | SQLite (via aiosqlite) |
  | Real-time | WebSockets |
  | Frontend | HTML5, CSS3, JavaScript, Chart.js |
  | Hardware | ESP32 (IAQ, VOC, eCO₂, eToH, Temp, Humidity sensors) |

  ---

  ## Getting Started

  ### Requirements

  - Ubuntu (native or WSL2 on Windows)
  - Python 3.10+

  ### Installation

  ```bash
  # 1. Clone the repository
  git clone https://github.com/phandienxauxa/Air-Quality-Monitoring-System-Edge-AI-.git
  cd Air-Quality-Monitoring-System-Edge-AI-

  # 2. Create and activate virtual environment
  python3 -m venv venv
  source venv/bin/activate

  # 3. Install dependencies
  pip install -r requirements.txt

  Running the Server

  uvicorn app.main:app --host 0.0.0.0 --port 8000

  The SQLite database is initialized automatically on first startup.

  On Windows, you can also use the provided batch script:
  start_server.bat

  Verify

  ┌──────────────────────────────────┬─────────────────────────────────────────┐
  │             Endpoint             │               Description               │
  ├──────────────────────────────────┼─────────────────────────────────────────┤
  │ http://localhost:8000/health     │ Health check — returns {"status": "ok"} │
  ├──────────────────────────────────┼─────────────────────────────────────────┤
  │ http://localhost:8000/docs       │ Interactive API documentation           │
  ├──────────────────────────────────┼─────────────────────────────────────────┤
  │ http://localhost:8000/index.html │ Live monitoring dashboard               │
  └──────────────────────────────────┴─────────────────────────────────────────┘

  ---
  WebSocket Endpoints

  ┌──────────────┬────────────────────┬─────────────────────────────────────┐
  │   Endpoint   │     Direction      │             Description             │
  ├──────────────┼────────────────────┼─────────────────────────────────────┤
  │ /ws/ingest   │ ESP32 → Server     │ Sensor data ingestion from hardware │
  ├──────────────┼────────────────────┼─────────────────────────────────────┤
  │ /ws/realtime │ Server → Dashboard │ Live data streaming to frontend     │
  └──────────────┴────────────────────┴─────────────────────────────────────┘

  ---
  Development

  Run the simulator in a separate terminal to generate mock sensor data without hardware:

  source venv/bin/activate
  python3 simulator/

  Tests are located in the test/ directory.

  ---
  Monitored Metrics

  ┌─────────────┬───────┬────────────────────────────────────┐
  │   Metric    │ Unit  │            Description             │
  ├─────────────┼───────┼────────────────────────────────────┤
  │ IAQ         │ index │ Indoor Air Quality composite index │
  ├─────────────┼───────┼────────────────────────────────────┤
  │ VOC         │ ppb   │ Volatile Organic Compounds         │
  ├─────────────┼───────┼────────────────────────────────────┤
  │ eCO₂        │ ppm   │ Equivalent CO₂ concentration       │
  ├─────────────┼───────┼────────────────────────────────────┤
  │ eToH        │ index │ Ethanol index                      │
  ├─────────────┼───────┼────────────────────────────────────┤
  │ Temperature │ °C    │ Ambient temperature                │
  ├─────────────┼───────┼────────────────────────────────────┤
  │ Humidity    │ % RH  │ Relative humidity                  │
  └─────────────┴───────┴────────────────────────────────────┘

  ---
  License

  This project is developed for academic purposes at Ho Chi Minh City University of Technology.

  ---
