"""
send_mock_data.py — Giả lập ESP32 gửi dữ liệu lên server qua WebSocket.

Chạy:
    python simulator/send_mock_data.py

Tuỳ chọn:
    python simulator/send_mock_data.py --url ws://192.168.1.100:8000/ws/ingest
    python simulator/send_mock_data.py --interval 2 --bad-cycle
"""

import asyncio
import json
import math
import random
import argparse
from datetime import datetime, timezone
import websockets


# ── Config mặc định ───────────────────────────────────────────
DEFAULT_URL      = "ws://localhost:8000/ws/ingest"
DEFAULT_INTERVAL = 1.0   # giây
DEVICE_ID        = "esp32_gateway_01"


# ── State của simulator ───────────────────────────────────────
class SimState:
    def __init__(self):
        self.iaq   = 70.0
        self.voc   = 2.0
        self.eco2  = 580.0
        self.temp  = 29.0
        self.hum   = 65.0
        self.tick  = 0

    def walk(self, val, step, lo, hi):
        return min(hi, max(lo, val + (random.random() - 0.48) * step))

    def next(self, force_bad: bool = False):
        self.tick += 1

        # Tạo chu kỳ xấu dần mỗi 60 tick (~1 phút)
        bad_cycle = force_bad or math.sin(self.tick / 60 * math.pi) > 0.6
        # Thỉnh thoảng spike ngẫu nhiên
        spike = random.random() < 0.03

        iaq_step = 6 if (bad_cycle or spike) else 2
        self.iaq  = self.walk(self.iaq,  iaq_step, 0,   200)
        self.voc  = self.walk(self.voc,  0.2,      0,   10)
        self.eco2 = self.walk(self.eco2, 25 if bad_cycle else 10, 400, 2000)
        self.temp = self.walk(self.temp, 0.2,      15,  45)
        self.hum  = self.walk(self.hum,  0.8,      20,  95)

        tvoc_raw = round(self.iaq * 1.8 + random.uniform(0, 5))
        eco2_raw = round(self.eco2 + random.uniform(-5, 5))

        def aq_label(iaq):
            if iaq < 50:  return "Tốt"
            if iaq < 100: return "Trung bình"
            return "Kém"

        trend = 1 if self.iaq > 90 else (-1 if self.iaq < 40 else 0)
        pred_iaq = max(0, min(200, self.iaq + trend * 12))

        # Thỉnh thoảng mô phỏng lỗi thiết bị
        error_code = 1 if random.random() < 0.02 else 0

        return {
            "timestamp":                  datetime.now(timezone.utc).isoformat(),
            "device_id":                  DEVICE_ID,
            "raw_tvoc":                   tvoc_raw,
            "raw_eco2":                   eco2_raw,
            "temperature":                round(self.temp, 1),
            "humidity":                   round(self.hum, 1),
            "filtered_tvoc":              round(tvoc_raw * 0.97),
            "filtered_eco2":              round(eco2_raw * 0.98),
            "normalized_tvoc":            round(min(1.0, tvoc_raw / 300), 4),
            "normalized_eco2":            round(min(1.0, eco2_raw / 2000), 4),
            "iaq_index":                  round(self.iaq, 1),
            "voc_index":                  round(self.voc, 2),
            "eco2_ppm":                   round(self.eco2),
            "air_quality_label_now":      aq_label(self.iaq),
            "air_quality_label_pred_5m":  aq_label(pred_iaq),
            "battery_status":             "external_power",
            "network_status":             "good",
            "error_code":                 error_code,
        }


# ── Main loop ─────────────────────────────────────────────────
async def run(url: str, interval: float, force_bad: bool):
    state = SimState()
    print(f"[Simulator] Kết nối tới: {url}")
    print(f"[Simulator] Interval: {interval}s | Device: {DEVICE_ID}")
    print("[Simulator] Nhấn Ctrl+C để dừng\n")

    retry_delay = 2

    while True:
        try:
            async with websockets.connect(url) as ws:
                print(f"[Simulator] ✓ Đã kết nối thành công")
                retry_delay = 2  # reset khi kết nối thành công

                while True:
                    payload = state.next(force_bad)
                    await ws.send(json.dumps(payload, ensure_ascii=False))

                    # In trạng thái hiện tại
                    aq = payload["air_quality_label_now"]
                    pred = payload["air_quality_label_pred_5m"]
                    print(
                        f"[Sim] tick={state.tick:04d} | "
                        f"IAQ={payload['iaq_index']:6.1f} | "
                        f"VOC={payload['voc_index']:4.2f} | "
                        f"eCO2={payload['eco2_ppm']:5d} | "
                        f"Temp={payload['temperature']:4.1f}°C | "
                        f"Hum={payload['humidity']:4.1f}% | "
                        f"AQ={aq:10s} → Pred={pred}"
                    )

                    # Nhận ACK từ server
                    try:
                        ack_raw = await asyncio.wait_for(ws.recv(), timeout=3)
                        ack = json.loads(ack_raw)
                        if ack.get("status") == "error":
                            print(f"[Sim] ⚠ Server báo lỗi: {ack.get('message')}")
                    except asyncio.TimeoutError:
                        print("[Sim] ⚠ Không nhận được ACK (timeout)")

                    await asyncio.sleep(interval)

        except (ConnectionRefusedError, OSError):
            print(f"[Simulator] ✗ Không kết nối được — thử lại sau {retry_delay}s...")
            await asyncio.sleep(retry_delay)
            retry_delay = min(retry_delay * 2, 30)  # exponential backoff

        except Exception as e:
            print(f"[Simulator] Lỗi: {e} — thử lại sau {retry_delay}s...")
            await asyncio.sleep(retry_delay)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="ESP32 Simulator")
    parser.add_argument("--url",      default=DEFAULT_URL, help="WebSocket URL của server")
    parser.add_argument("--interval", type=float, default=DEFAULT_INTERVAL, help="Giây giữa mỗi gửi")
    parser.add_argument("--bad-cycle", action="store_true", help="Ép chất lượng KK luôn xấu")
    args = parser.parse_args()

    asyncio.run(run(args.url, args.interval, args.bad_cycle))
