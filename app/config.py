# config.py — Cấu hình trung tâm của server

DB_PATH = "data/air_quality.db"

HOST = "0.0.0.0"
PORT = 8000

DEVICE_WARNING_TIMEOUT = 3
DEVICE_OFFLINE_TIMEOUT = 5

# Ngưỡng cảnh báo IAQ — thang Renesas (1.0–5.0+)
ALERT_IAQ_CRITICAL = 5.0   # Level 5: Rất kém  (Bad)
ALERT_IAQ_HIGH     = 4.0   # Level 4: Kém       (Poor)
ALERT_IAQ_WARN     = 3.0   # Level 3: Trung bình (Medium)

# Ngưỡng TVOC — thang Renesas (mg/m³)
ALERT_VOC_CRITICAL = 10.0  # Level 5: > 10 mg/m³
ALERT_VOC_HIGH     = 3.0   # Level 4: 3.0–10.0 mg/m³

# eCO₂ (ppm)
ALERT_ECO2_HIGH = 1200

# eToH index (Sensirion 0-500)
ALERT_ETOH_HIGH = 200

# Nhãn hợp lệ — 5 mức Renesas
AQ_LABELS_VALID = {"Rất tốt", "Tốt", "Trung bình", "Kém", "Rất kém"}