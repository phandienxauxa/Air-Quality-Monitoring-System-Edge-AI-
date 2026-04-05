# config.py — Cấu hình trung tâm của server

DB_PATH = "data/air_quality.db"

HOST = "0.0.0.0"
PORT = 8000

DEVICE_WARNING_TIMEOUT = 3
DEVICE_OFFLINE_TIMEOUT = 5

# Ngưỡng cảnh báo — đồng bộ với frontend (Sensirion 0-500 scale)
ALERT_ECO2_HIGH  = 1200   # ppm          
ALERT_IAQ_HIGH   = 200    # index        
ALERT_IAQ_WARN   = 100    # index — mức trung bình
ALERT_VOC_HIGH   = 250    # index 0-500  
ALERT_ETOH_HIGH  = 200    # index 0-500  

AQ_LABELS_VALID  = {"Tốt", "Trung bình", "Kém"}