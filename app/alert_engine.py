"""
alert_engine.py — Kiểm tra ngưỡng cảnh báo và ghi event_logs.
Được gọi mỗi khi ingest thành công một payload hợp lệ.
"""

import time
from app.schemas import SensorPayload
from app.crud import insert_event_log
from app.config import (
    ALERT_ECO2_HIGH,
    ALERT_IAQ_HIGH,
    ALERT_IAQ_WARN,
    ALERT_VOC_HIGH,
    ALERT_ETOH_HIGH,
)

# Cooldown: không spam cùng một cảnh báo trong vòng N giây
_last_alert: dict[str, float] = {}
COOLDOWN_SECONDS = 30


async def _maybe_alert(
    key: str,
    condition: bool,
    device_id: str,
    event_type: str,
    severity: str,
    message: str,
):
    if not condition:
        return
    now = time.time()
    if now - _last_alert.get(key, 0) < COOLDOWN_SECONDS:
        return
    _last_alert[key] = now
    await insert_event_log(device_id, event_type, severity, message)
    print(f"[Alert] [{severity.upper()}] {message}")


async def check_thresholds(payload: SensorPayload):
    d = payload.device_id

    # eCO₂ > 1200 ppm → critical
    await _maybe_alert(
        f"{d}:eco2_high",
        payload.eco2_ppm > ALERT_ECO2_HIGH,
        d, "eco2_high", "critical",
        f"eCO₂ vượt ngưỡng {ALERT_ECO2_HIGH} ppm — giá trị: {payload.eco2_ppm} ppm",
    )

    # IAQ > 200 → critical
    await _maybe_alert(
        f"{d}:iaq_high",
        payload.iaq_index > ALERT_IAQ_HIGH,
        d, "iaq_high", "critical",
        f"IAQ vượt ngưỡng {ALERT_IAQ_HIGH} — giá trị: {payload.iaq_index:.0f}",
    )

    # IAQ 100–200 → warning (trung bình, xu hướng xấu)
    await _maybe_alert(
        f"{d}:iaq_warn",
        ALERT_IAQ_WARN < payload.iaq_index <= ALERT_IAQ_HIGH,
        d, "iaq_warn", "warning",
        f"IAQ ở mức trung bình — giá trị: {payload.iaq_index:.0f}",
    )

    # VOC > 250 → warning (Sensirion 0-500)
    await _maybe_alert(
        f"{d}:voc_high",
        payload.voc_index > ALERT_VOC_HIGH,
        d, "voc_high", "warning",
        f"VOC index bất thường — giá trị: {payload.voc_index:.0f}",
    )

    # eToH > 200 → warning (mới)
    await _maybe_alert(
        f"{d}:etoh_high",
        payload.etoh > ALERT_ETOH_HIGH,
        d, "etoh_high", "warning",
        f"eToH index bất thường — giá trị: {payload.etoh:.0f}",
    )

    # Chất lượng không khí hiện tại KÉM → critical
    await _maybe_alert(
        f"{d}:aq_bad_now",
        payload.air_quality_label_now == "Kém",
        d, "aq_bad_now", "critical",
        "Chất lượng không khí hiện tại ở mức KÉM",
    )

    # Dự báo 5 phút KÉM → warning
    await _maybe_alert(
        f"{d}:aq_bad_pred",
        payload.air_quality_label_pred_5m == "Kém",
        d, "aq_pred_bad", "warning",
        "Dự báo chất lượng không khí sẽ KÉM trong 5 phút tới",
    )

    # Device error code
    await _maybe_alert(
        f"{d}:error_code",
        payload.error_code != 0,
        d, "device_error", "warning",
        f"Thiết bị báo lỗi — error_code: {payload.error_code}",
    )