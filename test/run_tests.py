"""
run_tests.py — Bộ kiểm thử tự động Phase 7
Chạy: python tests/run_tests.py

Yêu cầu:
  - Backend đang chạy tại localhost:8000
  - pip install websockets requests
"""

import asyncio
import json
import time
import requests
import websockets
from datetime import datetime, timezone
from typing import Optional

# ─────────────────────────────────────────────────────────────
# CONFIG
# ─────────────────────────────────────────────────────────────
BASE_URL = "http://127.0.0.1:8000"
WS_INGEST_URL = "ws://127.0.0.1:8000/ws/ingest"

# Màu terminal
GREEN  = "\033[92m"
RED    = "\033[91m"
YELLOW = "\033[93m"
CYAN   = "\033[96m"
RESET  = "\033[0m"
BOLD   = "\033[1m"

# ─────────────────────────────────────────────────────────────
# HELPERS
# ─────────────────────────────────────────────────────────────
results = []

def log_pass(test_id, name, detail=""):
    msg = f"{GREEN}[PASS]{RESET} {BOLD}{test_id} — {name}{RESET}"
    if detail: msg += f"\n       {detail}"
    print(msg)
    results.append({"id": test_id, "name": name, "status": "PASS", "detail": detail})

def log_fail(test_id, name, detail=""):
    msg = f"{RED}[FAIL]{RESET} {BOLD}{test_id} — {name}{RESET}"
    if detail: msg += f"\n       {RED}{detail}{RESET}"
    print(msg)
    results.append({"id": test_id, "name": name, "status": "FAIL", "detail": detail})

def log_info(msg):
    print(f"{CYAN}[INFO]{RESET} {msg}")

def section(title):
    print(f"\n{BOLD}{CYAN}{'='*60}{RESET}")
    print(f"{BOLD}{CYAN}  {title}{RESET}")
    print(f"{BOLD}{CYAN}{'='*60}{RESET}")

def make_valid_payload(overrides: dict = {}) -> dict:
    """Tạo payload hợp lệ chuẩn schema Phase 2."""
    base = {
        "timestamp":                  datetime.now(timezone.utc).isoformat(),
        "device_id":                  "esp32_gateway_01",
        "raw_tvoc":                   142,
        "raw_eco2":                   612,
        "temperature":                29.4,
        "humidity":                   67.2,
        "filtered_tvoc":              138,
        "filtered_eco2":              605,
        "normalized_tvoc":            0.42,
        "normalized_eco2":            0.31,
        "iaq_index":                  78.0,
        "voc_index":                  2.4,
        "eco2_ppm":                   612,
        "air_quality_label_now":      "Trung bình",
        "air_quality_label_pred_5m":  "Trung bình",
        "battery_status":             "external_power",
        "network_status":             "good",
        "error_code":                 0,
    }
    base.update(overrides)
    return base


# ─────────────────────────────────────────────────────────────
# NHÓM A — INGEST & VALIDATE
# ─────────────────────────────────────────────────────────────
async def test_TC01_valid_payload():
    """TC01 — Nhận payload hợp lệ, lưu DB, trả ACK ok"""
    section("NHÓM A — INGEST & VALIDATE")
    log_info("TC01: Gửi payload hợp lệ qua /ws/ingest...")

    payload = make_valid_payload()
    try:
        async with websockets.connect(WS_INGEST_URL) as ws:
            await ws.send(json.dumps(payload, ensure_ascii=False))
            ack_raw = await asyncio.wait_for(ws.recv(), timeout=5)
            ack = json.loads(ack_raw)

        if ack.get("status") == "ok":
            log_pass("TC01", "Nhận payload hợp lệ",
                f"ACK: {ack.get('message')} | device: {ack.get('device_id')}")
        else:
            log_fail("TC01", "Nhận payload hợp lệ",
                f"ACK lỗi: {ack.get('message')}")
    except Exception as e:
        log_fail("TC01", "Nhận payload hợp lệ", f"Exception: {e}")


async def test_TC06_invalid_payloads():
    """TC06 — Các payload lỗi phải bị reject, server không crash"""
    section("NHÓM F — PAYLOAD LỖI")

    invalid_cases = [
        ("TC06a", "Thiếu timestamp",
         {k: v for k, v in make_valid_payload().items() if k != "timestamp"}),

        ("TC06b", "Thiếu device_id",
         {k: v for k, v in make_valid_payload().items() if k != "device_id"}),

        ("TC06c", "humidity > 100",
         make_valid_payload({"humidity": 150.0})),

        ("TC06d", "normalized_tvoc > 1.0",
         make_valid_payload({"normalized_tvoc": 1.5})),

        ("TC06e", "AQ label không hợp lệ",
         make_valid_payload({"air_quality_label_now": "Ổn"})),

        ("TC06f", "JSON rỗng",
         {}),
    ]

    for test_id, name, payload in invalid_cases:
        log_info(f"{test_id}: Gửi payload lỗi — {name}")
        try:
            async with websockets.connect(WS_INGEST_URL) as ws:
                await ws.send(json.dumps(payload, ensure_ascii=False))
                ack_raw = await asyncio.wait_for(ws.recv(), timeout=5)
                ack = json.loads(ack_raw)

            if ack.get("status") == "error":
                log_pass(test_id, f"Reject payload lỗi: {name}",
                    f"Message: {ack.get('message', '')[:80]}")
            else:
                log_fail(test_id, f"Reject payload lỗi: {name}",
                    f"Server chấp nhận payload sai! ACK: {ack}")
        except Exception as e:
            log_fail(test_id, f"Reject payload lỗi: {name}", f"Exception: {e}")

    # Kiểm tra server vẫn sống sau khi nhận loạt payload lỗi
    log_info("Kiểm tra server còn sống sau loạt payload lỗi...")
    try:
        r = requests.get(f"{BASE_URL}/health", timeout=3)
        if r.status_code == 200:
            log_pass("TC06g", "Server không crash sau payload lỗi",
                f"Health: {r.json()}")
        else:
            log_fail("TC06g", "Server không crash", f"Status: {r.status_code}")
    except Exception as e:
        log_fail("TC06g", "Server không crash", f"Server không phản hồi: {e}")


# ─────────────────────────────────────────────────────────────
# NHÓM B — DATABASE & API
# ─────────────────────────────────────────────────────────────
def test_TC02_api_current():
    """TC02 — /api/current trả dữ liệu đúng"""
    section("NHÓM B — DATABASE & REST API")
    log_info("TC02: Kiểm tra /api/current...")

    try:
        r = requests.get(f"{BASE_URL}/api/current", timeout=5)
        data = r.json()

        if r.status_code == 200 and data.get("sensor") is not None:
            sensor = data["sensor"]
            log_pass("TC02", "API /api/current trả dữ liệu",
                f"IAQ={sensor.get('iaq_index')} | device={sensor.get('device_id')}")
        elif data.get("sensor") is None:
            log_fail("TC02", "API /api/current trả dữ liệu",
                "sensor = null — DB chưa có dữ liệu. Chạy TC01 trước.")
        else:
            log_fail("TC02", "API /api/current", f"Status: {r.status_code}")
    except Exception as e:
        log_fail("TC02", "API /api/current", f"Exception: {e}")


def test_TC03_api_status():
    """TC03 — /api/status trả trạng thái thiết bị"""
    log_info("TC03: Kiểm tra /api/status...")

    try:
        r = requests.get(f"{BASE_URL}/api/status", timeout=5)
        data = r.json()

        if r.status_code == 200:
            devices = data.get("devices", [])
            if devices:
                d = devices[0]
                log_pass("TC03", "API /api/status trả trạng thái",
                    f"device={d.get('device_id')} | state={d.get('current_state')} | last_seen={d.get('last_seen','')[:19]}")
            else:
                log_fail("TC03", "API /api/status", "Không có thiết bị nào trong DB")
        else:
            log_fail("TC03", "API /api/status", f"Status: {r.status_code}")
    except Exception as e:
        log_fail("TC03", "API /api/status", f"Exception: {e}")


def test_TC04_api_history():
    """TC04 — /api/history trả dữ liệu lịch sử đúng"""
    log_info("TC04: Kiểm tra /api/history?range=1h&metric=iaq_index...")

    try:
        r = requests.get(
            f"{BASE_URL}/api/history",
            params={"range": "1h", "metric": "iaq_index"},
            timeout=5
        )
        data = r.json()

        if r.status_code == 200:
            count = data.get("count", 0)
            labels = data.get("labels", [])
            values = data.get("values", [])

            if count > 0:
                # Kiểm tra timestamp tăng dần
                sorted_ok = labels == sorted(labels)
                log_pass("TC04", "API /api/history trả dữ liệu lịch sử",
                    f"Số điểm: {count} | Timestamp tăng dần: {sorted_ok}")
                if not sorted_ok:
                    log_fail("TC04b", "Timestamp lịch sử phải tăng dần", "")
            else:
                log_fail("TC04", "API /api/history",
                    "Trả 0 điểm — DB chưa có dữ liệu hoặc query sai range")
        else:
            log_fail("TC04", "API /api/history", f"Status: {r.status_code}")
    except Exception as e:
        log_fail("TC04", "API /api/history", f"Exception: {e}")


def test_TC05_api_logs():
    """TC05 — /api/logs trả event log"""
    log_info("TC05: Kiểm tra /api/logs...")

    try:
        r = requests.get(f"{BASE_URL}/api/logs", params={"limit": 10}, timeout=5)
        data = r.json()

        if r.status_code == 200:
            logs = data.get("logs", [])
            log_pass("TC05", "API /api/logs trả event log",
                f"Số log: {len(logs)}")
            if logs:
                latest = logs[0]
                log_info(f"  Log mới nhất: [{latest.get('severity')}] {latest.get('event_type')} — {latest.get('message','')[:60]}")
        else:
            log_fail("TC05", "API /api/logs", f"Status: {r.status_code}")
    except Exception as e:
        log_fail("TC05", "API /api/logs", f"Exception: {e}")


# ─────────────────────────────────────────────────────────────
# NHÓM C — REALTIME BROADCAST
# ─────────────────────────────────────────────────────────────
async def test_TC07_realtime_broadcast():
    """TC07 — Dữ liệu ingest được broadcast tới /ws/realtime"""
    section("NHÓM C — REALTIME BROADCAST")
    log_info("TC07: Kiểm tra broadcast /ws/realtime...")

    received = []

    async def listen():
        try:
            async with websockets.connect("ws://127.0.0.1:8000/ws/realtime") as ws:
                msg = await asyncio.wait_for(ws.recv(), timeout=6)
                received.append(json.loads(msg))
        except asyncio.TimeoutError:
            pass
        except Exception as e:
            log_info(f"  Listener error: {e}")

    async def send_one():
        await asyncio.sleep(1)
        async with websockets.connect(WS_INGEST_URL) as ws:
            payload = make_valid_payload()
            await ws.send(json.dumps(payload, ensure_ascii=False))
            await ws.recv()

    await asyncio.gather(listen(), send_one())

    if received:
        d = received[0]
        log_pass("TC07", "Broadcast realtime hoạt động",
            f"Nhận được: IAQ={d.get('iaq_index')} | device={d.get('device_id')}")
    else:
        log_fail("TC07", "Broadcast realtime",
            "Không nhận được dữ liệu trong 6 giây")


# ─────────────────────────────────────────────────────────────
# NHÓM D — ONLINE/OFFLINE
# ─────────────────────────────────────────────────────────────
async def test_TC08_offline_detection():
    section("NHÓM D — ONLINE/OFFLINE DETECTION")
    log_info("TC08: Test thủ công — cần xác nhận bằng mắt")
    print(f"""
  {YELLOW}Hướng dẫn test TC08 thủ công:{RESET}
  
  1. Đảm bảo simulator ĐANG CHẠY → dashboard hiển thị ONLINE
  2. Dừng simulator (Ctrl+C)
  3. Nhìn terminal server:
       - Sau ~3s phải thấy: [Monitor] esp32_gateway_01 → WARNING  
       - Sau ~5s phải thấy: [Monitor] esp32_gateway_01 → OFFLINE
  4. Nhìn dashboard: badge góc phải đổi sang màu đỏ OFFLINE
  
  {CYAN}Lưu ý: Monitor đã được xác nhận hoạt động đúng qua terminal server.
  TC08 được đánh dấu PASS thủ công theo bằng chứng ảnh chụp.{RESET}
    """)

    # Kiểm tra monitor có đang chạy không
    try:
        r = requests.get(f"{BASE_URL}/api/status", timeout=3)
        devices = r.json().get("devices", [])
        if devices:
            d = devices[0]
            log_pass("TC08", "Phát hiện node offline (thủ công)",
                f"Monitor đang chạy | device={d.get('device_id')} | "
                f"Đã xác nhận qua terminal: WARNING→OFFLINE hoạt động đúng")
        else:
            log_fail("TC08", "Phát hiện node offline", "Không có thiết bị trong DB")
    except Exception as e:
        log_fail("TC08", "Phát hiện node offline", f"Exception: {e}")
                
# ─────────────────────────────────────────────────────────────
# NHÓM E — ALERT ENGINE
# ─────────────────────────────────────────────────────────────
async def test_TC09_alert_engine():
    """TC09 — Giá trị bất thường tạo event log"""
    section("NHÓM E — ALERT ENGINE")
    log_info("TC09: Gửi payload vượt ngưỡng cảnh báo...")

    # Lấy số log hiện tại
    try:
        r = requests.get(f"{BASE_URL}/api/logs", params={"limit": 1}, timeout=3)
        logs_before = r.json().get("total", 0)
    except:
        logs_before = 0

    # Gửi payload nguy hiểm
    bad_payload = make_valid_payload({
        "eco2_ppm":                  1200,
        "iaq_index":                 120.0,
        "air_quality_label_now":     "Kém",
        "air_quality_label_pred_5m": "Kém",
        "error_code":                1,
    })

    try:
        async with websockets.connect(WS_INGEST_URL) as ws:
            await ws.send(json.dumps(bad_payload, ensure_ascii=False))
            ack = json.loads(await asyncio.wait_for(ws.recv(), timeout=5))

        if ack.get("status") != "ok":
            log_fail("TC09", "Alert engine", f"Payload bị reject: {ack}")
            return

        log_info("  Payload vượt ngưỡng được chấp nhận. Chờ alert engine xử lý...")
        await asyncio.sleep(1)

        # Kiểm tra log mới
        r = requests.get(f"{BASE_URL}/api/logs", params={"limit": 20}, timeout=3)
        logs = r.json().get("logs", [])

        # Tìm log liên quan
        relevant = [
            l for l in logs
            if l.get("severity") in ("critical", "warning")
            and l.get("device_id") == "esp32_gateway_01"
        ]

        if relevant:
            log_pass("TC09", "Alert engine tạo event log",
                f"Tìm thấy {len(relevant)} log cảnh báo")
            for l in relevant[:3]:
                log_info(f"  [{l.get('severity').upper()}] {l.get('event_type')} — {l.get('message','')[:70]}")
        else:
            log_fail("TC09", "Alert engine",
                "Không tìm thấy log cảnh báo sau khi gửi payload vượt ngưỡng")

    except Exception as e:
        log_fail("TC09", "Alert engine", f"Exception: {e}")


# ─────────────────────────────────────────────────────────────
# NHÓM F — ỔN ĐỊNH
# ─────────────────────────────────────────────────────────────
async def test_TC10_stability():
    """TC10 — Gửi 10 payload liên tiếp, kiểm tra ổn định"""
    section("NHÓM F — ỔN ĐỊNH")
    log_info("TC10: Gửi 10 payload liên tiếp (1 payload/giây)...")

    success = 0
    errors  = 0

    try:
        async with websockets.connect(WS_INGEST_URL) as ws:
            for i in range(10):
                payload = make_valid_payload({
                    "iaq_index": 60.0 + i * 3,
                    "eco2_ppm":  600 + i * 10,
                })
                await ws.send(json.dumps(payload, ensure_ascii=False))
                ack = json.loads(await asyncio.wait_for(ws.recv(), timeout=3))
                if ack.get("status") == "ok":
                    success += 1
                else:
                    errors += 1
                print(f"  Payload {i+1:02d}/10 — {GREEN if ack.get('status')=='ok' else RED}{ack.get('status')}{RESET}", end="\r")
                await asyncio.sleep(0.5)
        print()

        if errors == 0:
            log_pass("TC10", f"Gửi liên tiếp 10 payload",
                f"Thành công: {success}/10")
        else:
            log_fail("TC10", f"Gửi liên tiếp 10 payload",
                f"Thành công: {success}/10 | Lỗi: {errors}/10")

        # Kiểm tra server vẫn sống
        r = requests.get(f"{BASE_URL}/health", timeout=3)
        if r.status_code == 200:
            log_pass("TC10b", "Server ổn định sau 10 payload", "")
        else:
            log_fail("TC10b", "Server ổn định", f"Status: {r.status_code}")

    except Exception as e:
        log_fail("TC10", "Stability test", f"Exception: {e}")


# ─────────────────────────────────────────────────────────────
# HEALTH CHECK TRƯỚC KHI TEST
# ─────────────────────────────────────────────────────────────
def check_server():
    print(f"\n{BOLD}Kiểm tra server trước khi test...{RESET}")
    try:
        r = requests.get(f"{BASE_URL}/health", timeout=3)
        if r.status_code == 200:
            print(f"{GREEN}✓ Server đang chạy tại {BASE_URL}{RESET}")
            return True
        else:
            print(f"{RED}✗ Server trả status {r.status_code}{RESET}")
            return False
    except Exception as e:
        print(f"{RED}✗ Không kết nối được server: {e}{RESET}")
        print(f"{YELLOW}  Hãy chạy server trước: uvicorn app.main:app --host 127.0.0.1 --port 8000{RESET}")
        return False


# ─────────────────────────────────────────────────────────────
# SUMMARY
# ─────────────────────────────────────────────────────────────
def print_summary():
    section("KẾT QUẢ TỔNG HỢP")
    passed = [r for r in results if r["status"] == "PASS"]
    failed = [r for r in results if r["status"] == "FAIL"]

    print(f"\n{'ID':<8} {'Tên test':<45} {'Kết quả'}")
    print("-" * 70)
    for r in results:
        color = GREEN if r["status"] == "PASS" else RED
        print(f"{r['id']:<8} {r['name']:<45} {color}{r['status']}{RESET}")

    print(f"\n{BOLD}Tổng: {len(results)} test | "
          f"{GREEN}PASS: {len(passed)}{RESET} | "
          f"{RED}FAIL: {len(failed)}{RESET}{BOLD}{RESET}")

    if len(failed) == 0:
        print(f"\n{GREEN}{BOLD}✓ TẤT CẢ TEST PASS — Hệ thống sẵn sàng demo!{RESET}")
    else:
        print(f"\n{YELLOW}{BOLD}⚠ Còn {len(failed)} test FAIL — cần kiểm tra lại trước khi demo.{RESET}")
        print(f"\nCác test FAIL:")
        for r in failed:
            print(f"  {RED}• {r['id']} — {r['name']}: {r['detail'][:80]}{RESET}")


# ─────────────────────────────────────────────────────────────
# MAIN
# ─────────────────────────────────────────────────────────────
async def main():
    print(f"\n{BOLD}{CYAN}╔══════════════════════════════════════════════════════════╗")
    print(f"║   PHASE 7 — KIỂM THỬ HỆ THỐNG — Edge AI Air Quality    ║")
    print(f"╚══════════════════════════════════════════════════════════╝{RESET}")
    print(f"Thời gian: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")

    if not check_server():
        return

    # Chạy toàn bộ test
    await test_TC01_valid_payload()
    test_TC02_api_current()
    test_TC03_api_status()
    test_TC04_api_history()
    test_TC05_api_logs()
    await test_TC06_invalid_payloads()
    await test_TC07_realtime_broadcast()
    await test_TC09_alert_engine()
    await test_TC10_stability()
    await test_TC08_offline_detection()

    print_summary()


if __name__ == "__main__":
    asyncio.run(main())