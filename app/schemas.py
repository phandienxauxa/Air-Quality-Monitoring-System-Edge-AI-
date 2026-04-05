"""
schemas.py — Validate payload JSON từ ESP32/simulator
"""

from pydantic import BaseModel, field_validator
from datetime import datetime
from typing import Literal
from app.config import AQ_LABELS_VALID


class SensorPayload(BaseModel):
    # Bắt buộc
    timestamp:                  str
    device_id:                  str
    raw_tvoc:                   int
    raw_eco2:                   int
    temperature:                float
    humidity:                   float
    filtered_tvoc:              int
    filtered_eco2:              int
    normalized_tvoc:            float
    normalized_eco2:            float
    iaq_index:                  float
    voc_index:                  float
    eco2_ppm:                   int
    air_quality_label_now:      str
    air_quality_label_pred_5m:  str
    battery_status:             str
    network_status:             str
    error_code:                 int
    etoh:                       float

    # ── Validators ────────────────────────────────────────────

    @field_validator("timestamp")
    @classmethod
    def validate_timestamp(cls, v: str) -> str:
        try:
            datetime.fromisoformat(v.replace("Z", "+00:00"))
        except ValueError:
            raise ValueError(f"timestamp phải là ISO 8601, nhận được: '{v}'")
        return v

    @field_validator("device_id")
    @classmethod
    def validate_device_id(cls, v: str) -> str:
        if not v.strip():
            raise ValueError("device_id không được để trống")
        return v.strip()

    @field_validator("air_quality_label_now", "air_quality_label_pred_5m")
    @classmethod
    def validate_aq_label(cls, v: str) -> str:
        if v not in AQ_LABELS_VALID:
            raise ValueError(
                f"Nhãn chất lượng không hợp lệ: '{v}'. Chấp nhận: {AQ_LABELS_VALID}"
            )
        return v

    @field_validator("temperature")
    @classmethod
    def validate_temperature(cls, v: float) -> float:
        if not (-20 <= v <= 80):
            raise ValueError(f"Nhiệt độ ngoài phạm vi: {v} °C")
        return v

    @field_validator("humidity")
    @classmethod
    def validate_humidity(cls, v: float) -> float:
        if not (0 <= v <= 100):
            raise ValueError(f"Độ ẩm ngoài phạm vi: {v}%")
        return v

    @field_validator("iaq_index")
    @classmethod
    def validate_iaq(cls, v: float) -> float:
        if not (0 <= v <= 500):
            raise ValueError(f"iaq_index ngoài phạm vi [0, 500]: {v}")
        return v

    @field_validator("voc_index")
    @classmethod
    def validate_voc(cls, v: float) -> float:
        if not (0 <= v <= 500):
            raise ValueError(f"voc_index ngoài phạm vi [0, 500]: {v}")
        return v

    @field_validator("eco2_ppm")
    @classmethod
    def validate_eco2(cls, v: int) -> int:
        if not (400 <= v <= 5000):
            raise ValueError(f"eco2_ppm ngoài phạm vi [400, 5000]: {v}")
        return v

    @field_validator("etoh")
    @classmethod
    def validate_etoh(cls, v: float) -> float:
        if not (0 <= v <= 500):
            raise ValueError(f"etoh ngoài phạm vi [0, 500]: {v}")
        return v

    @field_validator("normalized_tvoc", "normalized_eco2")
    @classmethod
    def validate_normalized(cls, v: float) -> float:
        # Chuẩn mới: normalized = raw / 500 (Sensirion scale)
        # max raw_tvoc ~550 → normalized có thể ~1.1, dùng bound 1.5 để an toàn
        if not (0 <= v <= 1.5):
            raise ValueError(f"Giá trị normalized phải trong [0, 1.5], nhận: {v}")
        return round(v, 4)


# ── Response schemas ──────────────────────────────────────────
class ACKResponse(BaseModel):
    status:    Literal["ok", "error"]
    message:   str
    device_id: str | None = None
    timestamp: str | None = None


class CurrentResponse(BaseModel):
    sensor:  dict | None
    status:  dict | None


class HistoryResponse(BaseModel):
    device_id: str
    range:     str
    metric:    str
    labels:    list[str]
    values:    list[float]


class LogsResponse(BaseModel):
    logs:  list[dict]
    total: int