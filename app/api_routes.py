"""
api_routes.py — Toàn bộ REST API endpoint cho dashboard.

Endpoints:
  GET /health               — kiểm tra server sống
  GET /api/current          — dữ liệu cảm biến và trạng thái mới nhất
  GET /api/status           — trạng thái tất cả thiết bị
  GET /api/history          — dữ liệu lịch sử theo range và metric
  GET /api/logs             — event log gần nhất
"""

from fastapi import APIRouter, Query
from fastapi.responses import JSONResponse
from app.crud import (
    get_latest_sensor,
    get_device_status,
    get_all_device_statuses,
    get_history,
    get_logs,
)

router = APIRouter()


# ── Health check ──────────────────────────────────────────────
@router.get("/health")
async def health():
    return {"status": "ok", "service": "Edge AI Air Quality Server v1"}


# ── Current data ──────────────────────────────────────────────
@router.get("/api/current")
async def current(device_id: str = Query(default=None)):
    """
    Trả về bản ghi cảm biến mới nhất + trạng thái thiết bị.
    Dashboard gọi endpoint này khi vừa load trang.
    """
    sensor = await get_latest_sensor(device_id)
    status = await get_device_status(device_id)
    return JSONResponse({
        "sensor": sensor,
        "status": status,
    })


# ── Device status ─────────────────────────────────────────────
@router.get("/api/status")
async def device_status(device_id: str = Query(default=None)):
    """
    Nếu có device_id → trả trạng thái thiết bị đó.
    Không có → trả danh sách tất cả thiết bị.
    """
    if device_id:
        status = await get_device_status(device_id)
        return JSONResponse({"status": status})
    else:
        all_statuses = await get_all_device_statuses()
        return JSONResponse({"devices": all_statuses})


# ── History ───────────────────────────────────────────────────
@router.get("/api/history")
async def history(
    range:     str = Query(default="1h",       description="1h | 24h | 7d | 30d"),
    metric:    str = Query(default="iaq_index", description="Tên cột trong sensor_data"),
    device_id: str = Query(default=None),
):
    """
    Lấy dữ liệu lịch sử để vẽ chart.
    Dashboard gọi endpoint này khi người dùng chọn range.
    """
    labels, values = await get_history(metric, range, device_id)
    return JSONResponse({
        "device_id": device_id or "all",
        "range":     range,
        "metric":    metric,
        "labels":    labels,
        "values":    values,
        "count":     len(values),
    })


# ── Logs ──────────────────────────────────────────────────────
@router.get("/api/logs")
async def logs(
    limit:    int = Query(default=50,  ge=1, le=500),
    severity: str = Query(default=None, description="critical | warning | info | all"),
):
    """
    Lấy event log gần nhất.
    Tuỳ chọn lọc theo severity.
    """
    result = await get_logs(limit=limit, severity=severity)
    return JSONResponse({
        "logs":  result,
        "total": len(result),
    })
