import aiosqlite
import os
from app.config import DB_PATH


async def get_db():
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    db = await aiosqlite.connect(DB_PATH)
    db.row_factory = aiosqlite.Row
    await db.execute("PRAGMA journal_mode=WAL")
    await db.execute("PRAGMA foreign_keys=ON")
    return db


async def init_db():
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    async with aiosqlite.connect(DB_PATH) as db:
        db.row_factory = aiosqlite.Row

        # ── sensor_data — etoh được khai báo ngay trong CREATE TABLE ──
        await db.execute("""
            CREATE TABLE IF NOT EXISTS sensor_data (
                id                        INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp                 TEXT    NOT NULL,
                device_id                 TEXT    NOT NULL,
                raw_tvoc                  INTEGER,
                raw_eco2                  INTEGER,
                temperature               REAL,
                humidity                  REAL,
                filtered_tvoc             INTEGER,
                filtered_eco2             INTEGER,
                normalized_tvoc           REAL,
                normalized_eco2           REAL,
                iaq_index                 REAL,
                voc_index                 REAL,
                eco2_ppm                  INTEGER,
                air_quality_label_now     TEXT,
                air_quality_label_pred_5m TEXT,
                battery_status            TEXT,
                network_status            TEXT,
                error_code                INTEGER DEFAULT 0,
                etoh                      REAL    DEFAULT 0
            )
        """)

        # Migration an toàn: nếu DB cũ chưa có cột etoh thì ALTER TABLE
        # Lệnh này sẽ bị bỏ qua (OperationalError) nếu cột đã tồn tại
        try:
            await db.execute(
                "ALTER TABLE sensor_data ADD COLUMN etoh REAL DEFAULT 0"
            )
            print("[DB] Migration: đã thêm cột etoh vào sensor_data")
        except Exception:
            pass  # Cột đã tồn tại — bình thường

        await db.execute("""
            CREATE TABLE IF NOT EXISTS device_status (
                id            INTEGER PRIMARY KEY AUTOINCREMENT,
                device_id     TEXT    NOT NULL UNIQUE,
                last_seen     TEXT    NOT NULL,
                network_status TEXT,
                battery_status TEXT,
                current_state TEXT    DEFAULT 'online',
                error_code    INTEGER DEFAULT 0
            )
        """)

        await db.execute("""
            CREATE TABLE IF NOT EXISTS event_logs (
                id         INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp  TEXT NOT NULL,
                device_id  TEXT NOT NULL,
                event_type TEXT NOT NULL,
                severity   TEXT NOT NULL,
                message    TEXT NOT NULL
            )
        """)

        # Indexes
        await db.execute(
            "CREATE INDEX IF NOT EXISTS idx_sensor_timestamp ON sensor_data(timestamp DESC)"
        )
        await db.execute(
            "CREATE INDEX IF NOT EXISTS idx_sensor_device ON sensor_data(device_id, timestamp DESC)"
        )
        await db.execute(
            "CREATE INDEX IF NOT EXISTS idx_logs_timestamp ON event_logs(timestamp DESC)"
        )
        await db.execute(
            "CREATE INDEX IF NOT EXISTS idx_logs_severity ON event_logs(severity, timestamp DESC)"
        )

        await db.commit()
    print("[DB] SQLite khởi tạo thành công tại: " + DB_PATH)