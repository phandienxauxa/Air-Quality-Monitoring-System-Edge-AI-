import asyncio
import aiosqlite
from datetime import datetime, timezone
from app.config import DB_PATH, DEVICE_WARNING_TIMEOUT, DEVICE_OFFLINE_TIMEOUT

_prev_states: dict[str, str] = {}


async def _load_initial_states():
    """
    Đọc trạng thái hiện tại từ DB khi server khởi động.
    Tránh trường hợp restart server → tất cả device bị coi là 'online'
    dù thực ra đang offline.
    """
    try:
        async with aiosqlite.connect(DB_PATH) as db:
            db.row_factory = aiosqlite.Row
            cursor = await db.execute(
                "SELECT device_id, current_state FROM device_status"
            )
            rows = await cursor.fetchall()
            for row in rows:
                _prev_states[row["device_id"]] = row["current_state"]
        print(f"[Monitor] Đã load {len(_prev_states)} trạng thái thiết bị từ DB")
    except Exception as e:
        print(f"[Monitor] Không thể load trạng thái ban đầu: {e}")


async def monitor_loop():
    print("[Monitor] Status monitor đã khởi động")
    await _load_initial_states()
    while True:
        try:
            await _check_devices()
        except Exception as e:
            print(f"[Monitor] Lỗi: {e}")
        await asyncio.sleep(1)


async def _check_devices():
    now = datetime.now(timezone.utc)
    async with aiosqlite.connect(DB_PATH) as db:
        db.row_factory = aiosqlite.Row
        cursor = await db.execute(
            "SELECT device_id, last_seen, current_state FROM device_status"
        )
        devices = await cursor.fetchall()

    for device in devices:
        device_id  = device["device_id"]
        last_seen  = device["last_seen"]
        prev_state = _prev_states.get(device_id, "online")

        try:
            last_dt = datetime.fromisoformat(last_seen.replace("Z", "+00:00"))
            if last_dt.tzinfo is None:
                last_dt = last_dt.replace(tzinfo=timezone.utc)
            elapsed = (now - last_dt).total_seconds()
        except Exception:
            continue

        if elapsed >= DEVICE_OFFLINE_TIMEOUT:
            new_state = "offline"
        elif elapsed >= DEVICE_WARNING_TIMEOUT:
            new_state = "warning"
        else:
            new_state = "online"

        if new_state != prev_state:
            _prev_states[device_id] = new_state
            async with aiosqlite.connect(DB_PATH) as db:
                await db.execute(
                    "UPDATE device_status SET current_state = ? WHERE device_id = ?",
                    (new_state, device_id)
                )
                ts = datetime.now(timezone.utc).isoformat()
                if new_state == "offline":
                    await db.execute(
                        "INSERT INTO event_logs (timestamp, device_id, event_type, severity, message) "
                        "VALUES (?, ?, ?, ?, ?)",
                        (ts, device_id, "offline", "critical", "Node mất kết nối")
                    )
                    print(f"[Monitor] {device_id} → OFFLINE")
                elif new_state == "warning":
                    await db.execute(
                        "INSERT INTO event_logs (timestamp, device_id, event_type, severity, message) "
                        "VALUES (?, ?, ?, ?, ?)",
                        (ts, device_id, "warning_timeout", "warning", "Node không gửi dữ liệu")
                    )
                    print(f"[Monitor] {device_id} → WARNING")
                elif prev_state in ("offline", "warning"):
                    await db.execute(
                        "INSERT INTO event_logs (timestamp, device_id, event_type, severity, message) "
                        "VALUES (?, ?, ?, ?, ?)",
                        (ts, device_id, "reconnect", "info", "Node đã kết nối lại")
                    )
                    print(f"[Monitor] {device_id} → ONLINE")
                await db.commit()