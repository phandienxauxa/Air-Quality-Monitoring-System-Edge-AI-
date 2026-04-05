"""
websocket_ingest.py — WebSocket endpoint /ws/ingest
Nhận dữ liệu từ ESP32 (hoặc simulator), validate, lưu DB, broadcast.
"""

import json
from fastapi import WebSocket, WebSocketDisconnect
from pydantic import ValidationError

from app.schemas import SensorPayload, ACKResponse
from app.crud import insert_sensor_data, upsert_device_status
from app.alert_engine import check_thresholds
from app.connections import manager


async def ws_ingest_handler(websocket: WebSocket):
    """
    Xử lý một kết nối WebSocket từ ESP32 / simulator.
    Mỗi ESP32 duy trì 1 kết nối liên tục, gửi payload mỗi ~1 giây.
    """
    await websocket.accept()
    client = websocket.client
    print(f"[WS/ingest] Thiết bị kết nối: {client}")

    try:
        while True:
            raw = await websocket.receive_text()

            # ── Parse JSON ────────────────────────────────────
            try:
                data = json.loads(raw)
            except json.JSONDecodeError as e:
                ack = ACKResponse(
                    status="error",
                    message=f"JSON không hợp lệ: {e}",
                )
                await websocket.send_text(ack.model_dump_json())
                continue

            # ── Validate schema ───────────────────────────────
            try:
                payload = SensorPayload(**data)
            except ValidationError as e:
                errors = "; ".join(
                    f"{err['loc'][-1]}: {err['msg']}"
                    for err in e.errors()
                )
                ack = ACKResponse(
                    status="error",
                    message=f"Validation lỗi: {errors}",
                )
                await websocket.send_text(ack.model_dump_json())
                print(f"[WS/ingest] Payload lỗi từ {client}: {errors}")
                continue

            # ── Lưu DB ────────────────────────────────────────
            await insert_sensor_data(payload)
            await upsert_device_status(payload)

            # ── Kiểm tra ngưỡng cảnh báo ─────────────────────
            await check_thresholds(payload)

            # ── Broadcast sang dashboard ──────────────────────
            if manager.count > 0:
                await manager.broadcast(payload.model_dump())

            # ── ACK về cho ESP32 ──────────────────────────────
            ack = ACKResponse(
                status="ok",
                message="Đã nhận và lưu thành công",
                device_id=payload.device_id,
                timestamp=payload.timestamp,
            )
            await websocket.send_text(ack.model_dump_json())

    except WebSocketDisconnect:
        print(f"[WS/ingest] Thiết bị ngắt kết nối: {client}")
    except Exception as e:
        print(f"[WS/ingest] Lỗi không xác định: {e}")
        try:
            await websocket.close()
        except Exception:
            pass
