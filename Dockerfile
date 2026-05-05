# ── Stage 1: cài dependencies ─────────────────────────────────
FROM python:3.11-slim AS builder

WORKDIR /app

# Cài deps trước để tận dụng Docker layer cache
# (chỉ rebuild layer này khi requirements.txt thay đổi)
COPY requirements.txt .
RUN pip install --no-cache-dir --upgrade pip \
 && pip install --no-cache-dir -r requirements.txt

# ── Stage 2: image chạy thực tế ───────────────────────────────
FROM python:3.11-slim

WORKDIR /app

# Copy thư viện đã cài từ builder
COPY --from=builder /usr/local/lib/python3.11/site-packages /usr/local/lib/python3.11/site-packages
COPY --from=builder /usr/local/bin /usr/local/bin

# Copy chỉ code backend (không cần frontend, test, simulator)
COPY app/ ./app/

# Tạo thư mục chứa SQLite database (sẽ được mount từ host)
RUN mkdir -p data

EXPOSE 8000

# Biến môi trường
ENV PYTHONUNBUFFERED=1 \
    PYTHONDONTWRITEBYTECODE=1

CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]
