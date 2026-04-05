"""
connections.py — Quản lý danh sách client dashboard đang kết nối WebSocket.
Backend dùng manager này để broadcast dữ liệu mới tới tất cả dashboard.
"""

import json
from fastapi import WebSocket
import asyncio


class ConnectionManager:
    def __init__(self):
        # Danh sách WebSocket của các dashboard đang mở
        self._clients: list[WebSocket] = []

    async def connect(self, ws: WebSocket):
        await ws.accept()
        self._clients.append(ws)
        print(f"[WS/realtime] Dashboard kết nối — tổng: {len(self._clients)}")

    def disconnect(self, ws: WebSocket):
        if ws in self._clients:
            self._clients.remove(ws)
        print(f"[WS/realtime] Dashboard ngắt kết nối — còn: {len(self._clients)}")

    async def broadcast(self, data: dict):
        """Gửi cùng một payload tới tất cả dashboard đang kết nối."""
        if not self._clients:
            return

        message = json.dumps(data, ensure_ascii=False)
        dead: list[WebSocket] = []

        for ws in self._clients:
            try:
                await ws.send_text(message)
            except Exception:
                dead.append(ws)

        # Dọn dẹp các kết nối đã chết
        for ws in dead:
            self.disconnect(ws)

    @property
    def count(self) -> int:
        return len(self._clients)


# Singleton dùng chung toàn app
manager = ConnectionManager()
