"""
main.py — Điểm khởi động chính của backend server.

Khởi động:
    uvicorn app.main:app --host 0.0.0.0 --port 8000 --reload
"""

import asyncio
from contextlib import asynccontextmanager
from fastapi import FastAPI, WebSocket
from fastapi.middleware.cors import CORSMiddleware

from app.database import init_db
from app.api_routes import router
from app.websocket_ingest import ws_ingest_handler
from app.websocket_realtime import ws_realtime_handler
from app.status_monitor import monitor_loop


# ── Lifespan: chạy khi server start/stop ─────────────────────
@asynccontextmanager
async def lifespan(app: FastAPI):
    # Khởi tạo database
    await init_db()

    # Khởi động status monitor chạy nền
    monitor_task = asyncio.create_task(monitor_loop())
    print("[Main] Server đã sẵn sàng")

    yield  # Server đang chạy

    # Dọn dẹp khi tắt server
    monitor_task.cancel()
    print("[Main] Server đã tắt")


# ── Tạo app ───────────────────────────────────────────────────
app = FastAPI(
    title="Edge AI Air Quality Server",
    description="Backend server cho hệ thống giám sát chất lượng không khí",
    version="1.0.0",
    lifespan=lifespan,
)

# ── CORS: cho phép dashboard truy cập API ────────────────────
# Trong production thay "*" bằng địa chỉ IP cụ thể của dashboard
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=False,
    allow_methods=["*"],
    allow_headers=["*"],
)

# ── REST API routes ───────────────────────────────────────────
app.include_router(router)

# ── WebSocket endpoints ───────────────────────────────────────
@app.websocket("/ws/ingest")
async def ws_ingest(websocket: WebSocket):
    """ESP32 / simulator kết nối vào đây để gửi dữ liệu."""
    await ws_ingest_handler(websocket)


@app.websocket("/ws/realtime")
async def ws_realtime(websocket: WebSocket):
    """Dashboard kết nối vào đây để nhận dữ liệu realtime."""
    await ws_realtime_handler(websocket)
