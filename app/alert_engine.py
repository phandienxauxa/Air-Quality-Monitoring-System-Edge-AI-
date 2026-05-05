"""
alert_engine.py — Kiểm tra ngưỡng cảnh báo và ghi event_logs.
Được gọi mỗi khi ingest thành công một payload hợp lệ.
"""

import time
from app.schemas import SensorPayload
from app.crud import insert_event_log
from app.config import (
    ALERT_ECO2_HIGH,
    ALERT_IAQ_CRITICAL,
    ALERT_IAQ_HIGH,
    ALERT_IAQ_WARN,
    ALERT_VOC_CRITICAL,
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

    # IAQ ≥ 5.0 → Level 5: Rất kém → critical
    await _maybe_alert(
        f"{d}:iaq_critical",
        payload.iaq_index >= ALERT_IAQ_CRITICAL,
        d, "iaq_critical", "critical",
        f"IAQ Level 5 — Rất kém (Unacceptable): {payload.iaq_index:.1f}",
    )

    # IAQ 4.0–4.9 → Level 4: Kém → warning
    await _maybe_alert(
        f"{d}:iaq_high",
        ALERT_IAQ_HIGH <= payload.iaq_index < ALERT_IAQ_CRITICAL,
        d, "iaq_high", "warning",
        f"IAQ Level 4 — Kém (Poor): {payload.iaq_index:.1f}",
    )

    # IAQ 3.0–3.9 → Level 3: Trung bình → info
    await _maybe_alert(
        f"{d}:iaq_warn",
        ALERT_IAQ_WARN <= payload.iaq_index < ALERT_IAQ_HIGH,
        d, "iaq_warn", "info",
        f"IAQ Level 3 — Trung bình (Medium): {payload.iaq_index:.1f}",
    )

    # TVOC > 10.0 mg/m³ → Level 5 → critical
    await _maybe_alert(
        f"{d}:voc_critical",
        payload.voc_index > ALERT_VOC_CRITICAL,
        d, "voc_critical", "critical",
        f"TVOC vượt ngưỡng Level 5 — giá trị: {payload.voc_index:.2f} mg/m³",
    )

    # TVOC 3.0–10.0 mg/m³ → Level 4 → warning
    await _maybe_alert(
        f"{d}:voc_high",
        ALERT_VOC_HIGH < payload.voc_index <= ALERT_VOC_CRITICAL,
        d, "voc_high", "warning",
        f"TVOC Level 4 — Kém: {payload.voc_index:.2f} mg/m³",
    )

    # eToH > 200 → warning
    await _maybe_alert(
        f"{d}:etoh_high",
        payload.etoh > ALERT_ETOH_HIGH,
        d, "etoh_high", "warning",
        f"eToH index bất thường — giá trị: {payload.etoh:.0f}",
    )

    # Label hiện tại Rất kém → critical
    await _maybe_alert(
        f"{d}:aq_very_bad_now",
        payload.air_quality_label_now == "Rất kém",
        d, "aq_very_bad_now", "critical",
        "Chất lượng không khí ở mức RẤT KÉM (Level 5)",
    )

    # Label hiện tại Kém → warning
    await _maybe_alert(
        f"{d}:aq_bad_now",
        payload.air_quality_label_now == "Kém",
        d, "aq_bad_now", "warning",
        "Chất lượng không khí ở mức KÉM (Level 4)",
    )

    # Dự báo 5 phút Rất kém → critical
    await _maybe_alert(
        f"{d}:aq_very_bad_pred",
        payload.air_quality_label_pred_5m == "Rất kém",
        d, "aq_pred_very_bad", "critical",
        "Dự báo 5 phút: chất lượng sẽ RẤT KÉM (Level 5)",
    )

    # Dự báo 5 phút Kém → warning
    await _maybe_alert(
        f"{d}:aq_bad_pred",
        payload.air_quality_label_pred_5m == "Kém",
        d, "aq_pred_bad", "warning",
        "Dự báo 5 phút: chất lượng sẽ KÉM (Level 4)",
    )

    # Device error code
    await _maybe_alert(
        f"{d}:error_code",
        payload.error_code != 0,
        d, "device_error", "warning",
        f"Thiết bị báo lỗi — error_code: {payload.error_code}",
    )