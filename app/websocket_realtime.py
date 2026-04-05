"""
websocket_realtime.py — WebSocket endpoint /ws/realtime
Dashboard kết nối vào đây để nhận broadcast realtime.
"""

from fastapi import WebSocket, WebSocketDisconnect
from app.connections import manager


async def ws_realtime_handler(websocket: WebSocket):
    """
    Chấp nhận kết nối từ dashboard, giữ open để nhận broadcast.
    Không cần gửi gì từ dashboard; chỉ nhận dữ liệu từ server.
    """
    await manager.connect(websocket)
    try:
        while True:
            # Chờ tin nhắn từ dashboard (ping/pong hoặc lệnh đặc biệt)
            # Hiện tại chỉ keep-alive; dashboard không cần gửi gì
            await websocket.receive_text()
    except WebSocketDisconnect:
        manager.disconnect(websocket)
    except Exception as e:
        print(f"[WS/realtime] Lỗi: {e}")
        manager.disconnect(websocket)
