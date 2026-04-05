import aiosqlite
from datetime import datetime, timezone, timedelta
from app.schemas import SensorPayload
from app.config import DB_PATH


async def insert_sensor_data(payload: SensorPayload) -> int:
    async with aiosqlite.connect(DB_PATH) as db:
        cursor = await db.execute("""
            INSERT INTO sensor_data (
                timestamp, device_id, raw_tvoc, raw_eco2,
                temperature, humidity, filtered_tvoc, filtered_eco2,
                normalized_tvoc, normalized_eco2, iaq_index, voc_index, eco2_ppm,
                air_quality_label_now, air_quality_label_pred_5m,
                battery_status, network_status, error_code, etoh
            ) VALUES (
                :timestamp, :device_id, :raw_tvoc, :raw_eco2,
                :temperature, :humidity, :filtered_tvoc, :filtered_eco2,
                :normalized_tvoc, :normalized_eco2, :iaq_index, :voc_index, :eco2_ppm,
                :air_quality_label_now, :air_quality_label_pred_5m,
                :battery_status, :network_status, :error_code, :etoh
            )
        """, payload.model_dump())
        await db.commit()
        return cursor.lastrowid


async def upsert_device_status(payload: SensorPayload):
    async with aiosqlite.connect(DB_PATH) as db:
        await db.execute("""
            INSERT INTO device_status
                (device_id, last_seen, network_status, battery_status, current_state, error_code)
            VALUES
                (:device_id, :timestamp, :network_status, :battery_status, 'online', :error_code)
            ON CONFLICT(device_id) DO UPDATE SET
                last_seen      = excluded.last_seen,
                network_status = excluded.network_status,
                battery_status = excluded.battery_status,
                current_state  = 'online',
                error_code     = excluded.error_code
        """, {
            "device_id":      payload.device_id,
            "timestamp":      payload.timestamp,
            "network_status": payload.network_status,
            "battery_status": payload.battery_status,
            "error_code":     payload.error_code,
        })
        await db.commit()


async def insert_event_log(device_id, event_type, severity, message):
    async with aiosqlite.connect(DB_PATH) as db:
        ts = datetime.now(timezone.utc).isoformat()
        await db.execute("""
            INSERT INTO event_logs (timestamp, device_id, event_type, severity, message)
            VALUES (?, ?, ?, ?, ?)
        """, (ts, device_id, event_type, severity, message))
        await db.commit()


async def update_device_state(device_id: str, state: str):
    async with aiosqlite.connect(DB_PATH) as db:
        await db.execute(
            "UPDATE device_status SET current_state = ? WHERE device_id = ?",
            (state, device_id)
        )
        await db.commit()


async def get_latest_sensor(device_id: str = None):
    async with aiosqlite.connect(DB_PATH) as db:
        db.row_factory = aiosqlite.Row
        if device_id:
            cursor = await db.execute(
                "SELECT * FROM sensor_data WHERE device_id = ? ORDER BY timestamp DESC LIMIT 1",
                (device_id,)
            )
        else:
            cursor = await db.execute(
                "SELECT * FROM sensor_data ORDER BY timestamp DESC LIMIT 1"
            )
        row = await cursor.fetchone()
        return dict(row) if row else None


async def get_device_status(device_id: str = None):
    async with aiosqlite.connect(DB_PATH) as db:
        db.row_factory = aiosqlite.Row
        if device_id:
            cursor = await db.execute(
                "SELECT * FROM device_status WHERE device_id = ?", (device_id,)
            )
        else:
            cursor = await db.execute(
                "SELECT * FROM device_status ORDER BY last_seen DESC LIMIT 1"
            )
        row = await cursor.fetchone()
        return dict(row) if row else None


async def get_all_device_statuses():
    async with aiosqlite.connect(DB_PATH) as db:
        db.row_factory = aiosqlite.Row
        cursor = await db.execute(
            "SELECT * FROM device_status ORDER BY last_seen DESC"
        )
        rows = await cursor.fetchall()
        return [dict(r) for r in rows]


# Whitelist cột hợp lệ — map tên frontend → tên cột DB
# Dùng dict cứng thay vì f-string để tránh SQL injection
_METRIC_COLUMN_MAP: dict[str, str] = {
    "iaq_index":        "iaq_index",
    "voc_index":        "voc_index",
    "eco2_ppm":         "eco2_ppm",
    "temperature":      "temperature",
    "humidity":         "humidity",
    "raw_tvoc":         "raw_tvoc",
    "raw_eco2":         "raw_eco2",
    "filtered_tvoc":    "filtered_tvoc",
    "filtered_eco2":    "filtered_eco2",
    "normalized_tvoc":  "normalized_tvoc",
    "normalized_eco2":  "normalized_eco2",
    "etoh":             "etoh",
}

_RANGE_DELTA: dict[str, timedelta] = {
    "1h":  timedelta(hours=1),
    "24h": timedelta(hours=24),
    "7d":  timedelta(days=7),
    "30d": timedelta(days=30),
}

_RANGE_LIMIT: dict[str, int] = {
    "1h": 120, "24h": 288, "7d": 336, "30d": 360,
}


async def get_history(metric: str, range_key: str, device_id: str = None):
    # Hardened: dùng dict map thay vì f-string trực tiếp
    col   = _METRIC_COLUMN_MAP.get(metric, "iaq_index")
    delta = _RANGE_DELTA.get(range_key, timedelta(hours=1))
    limit = _RANGE_LIMIT.get(range_key, 120)

    # Tính since bằng Python datetime UTC → so sánh string ISO hoạt động
    # đúng với cả timestamp UTC và UTC+7 vì SQLite so sánh lexicographic
    # trên chuỗi ISO 8601 (2025-... > 2024-... luôn đúng)
    since = (datetime.now(timezone.utc) - delta).isoformat()

    # Tên cột đã được whitelist — an toàn để dùng trong f-string
    sql_base = f"SELECT timestamp, {col} AS value FROM sensor_data WHERE timestamp >= ?"

    async with aiosqlite.connect(DB_PATH) as db:
        db.row_factory = aiosqlite.Row
        if device_id:
            cursor = await db.execute(
                sql_base + " AND device_id = ? ORDER BY timestamp ASC LIMIT ?",
                (since, device_id, limit)
            )
        else:
            cursor = await db.execute(
                sql_base + " ORDER BY timestamp ASC LIMIT ?",
                (since, limit)
            )
        rows = await cursor.fetchall()

    labels = [r["timestamp"] for r in rows]
    values = [
        round(float(r["value"]), 3) if r["value"] is not None else 0.0
        for r in rows
    ]
    return labels, values


async def get_logs(limit: int = 50, severity: str = None):
    async with aiosqlite.connect(DB_PATH) as db:
        db.row_factory = aiosqlite.Row
        if severity and severity != "all":
            cursor = await db.execute(
                "SELECT * FROM event_logs WHERE severity = ? ORDER BY timestamp DESC LIMIT ?",
                (severity, limit)
            )
        else:
            cursor = await db.execute(
                "SELECT * FROM event_logs ORDER BY timestamp DESC LIMIT ?",
                (limit,)
            )
        rows = await cursor.fetchall()
        return [dict(r) for r in rows]