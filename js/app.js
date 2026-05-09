/**
 * app.js — Điều phối chính của dashboard
 * Fix: truyền data.timestamp (ISO) trực tiếp vào Charts.updateXxx()
 *      thay vì pre-format bằng toLocaleTimeString
 * Fix: ngưỡng alert VOC theo Sensirion 0-500
 */

const CONFIG = {
  WS_URL:          'ws://127.0.0.1:8000/ws/realtime',
  API_BASE:        'http://127.0.0.1:8000/api',
  MOCK_INTERVAL:   1000,
  OFFLINE_TIMEOUT: 5000,
};

let lastDataTime = Date.now();
let offlineTimer = null;
let activeRange  = '1h';
let activeMetric = 'iaq';

// ── WebSocket ────────────────────────────────────────────────
let ws = null;

function initWebSocket() {
  ws = new WebSocket(CONFIG.WS_URL);

  ws.onopen = () => {
    console.log('[WS] Connected to backend');
    UI.updateNodeStatus(true);
    fetchCurrent();
  };

  ws.onmessage = (event) => {
    try {
      const payload = JSON.parse(event.data);
      handleIncomingData(payload);
    } catch (e) {
      console.error('[WS] Parse error:', e);
    }
  };

  ws.onclose = () => {
    console.warn('[WS] Disconnected — retrying in 3s');
    UI.updateNodeStatus(false);
    setTimeout(initWebSocket, 3000);
  };

  ws.onerror = (err) => console.error('[WS] Error:', err);
}

// ── Handler ──────────────────────────────────────────────────
function handleIncomingData(data) {
  lastDataTime = Date.now();

  UI.updateAll(data);

  // Truyền ISO timestamp trực tiếp — charts.js tự format
  Charts.updateIAQ(data.timestamp, data.iaq_index);
  if (data.etoh !== undefined) Charts.updateEToH(data.timestamp, data.etoh);
  Charts.updateVOC(data.timestamp, data.tvoc ?? data.voc_index, data.eco2_ppm);
  Charts.updateEnv(data.timestamp, data.temperature, data.humidity);

  clearTimeout(offlineTimer);
  offlineTimer = setTimeout(() => {
    UI.updateNodeStatus(false);
    UI.addLog({
      time: new Date(), device: data.device_id,
      type: 'offline', severity: 'critical',
      msg: `Node mất kết nối — timeout ${CONFIG.OFFLINE_TIMEOUT / 1000} giây`
    });
  }, CONFIG.OFFLINE_TIMEOUT);

  checkThresholds(data);
}

// ── Threshold alerts — thang Renesas 5 mức ──────────────────
const alertCooldown = {};

function checkThresholds(data) {
  const now = Date.now();

  function maybeAlert(key, cond, severity, type, msg) {
    if (cond && (!alertCooldown[key] || now - alertCooldown[key] > 30000)) {
      alertCooldown[key] = now;
      UI.addLog({ time: new Date(), device: data.device_id, type, severity, msg });
    }
  }

  // eCO₂: > 1200 ppm → critical
  maybeAlert('eco2_high', data.eco2_ppm > 1200, 'critical', 'eco2_high',
    `eCO₂ vượt ngưỡng 1200 ppm — giá trị: ${data.eco2_ppm} ppm`);

  // IAQ ≥ 5.0 → Level 5: Rất kém → critical
  maybeAlert('iaq_critical', data.iaq_index >= 5.0, 'critical', 'iaq_critical',
    `IAQ Level 5 — Rất kém (Unacceptable): ${data.iaq_index.toFixed(1)}`);

  // IAQ 4.0–4.9 → Level 4: Kém → warning
  maybeAlert('iaq_high', data.iaq_index >= 4.0 && data.iaq_index < 5.0, 'warning', 'iaq_high',
    `IAQ Level 4 — Kém (Poor): ${data.iaq_index.toFixed(1)}`);

  // IAQ 3.0–3.9 → Level 3: Trung bình → info
  maybeAlert('iaq_warn', data.iaq_index >= 3.0 && data.iaq_index < 4.0, 'info', 'iaq_warn',
    `IAQ Level 3 — Trung bình (Medium): ${data.iaq_index.toFixed(1)}`);

  // TVOC > 10.0 mg/m³ → Level 5 → critical
  maybeAlert('voc_critical', data.voc_index > 10.0, 'critical', 'voc_critical',
    `TVOC Level 5 — Rất kém: ${data.voc_index.toFixed(2)} mg/m³`);

  // TVOC 3.0–10.0 mg/m³ → Level 4 → warning
  maybeAlert('voc_high', data.voc_index > 3.0 && data.voc_index <= 10.0, 'warning', 'voc_high',
    `TVOC Level 4 — Kém: ${data.voc_index.toFixed(2)} mg/m³`);

  // eToH: > 0.3 mg/m³ → warning
  if (data.etoh !== undefined) {
    maybeAlert('etoh_high', data.etoh > 0.3, 'warning', 'etoh_high',
      `eToH bất thường — giá trị: ${data.etoh.toFixed(6)}`);
  }

  // Dự báo 5 phút
  maybeAlert('aq_very_bad_pred', data.air_quality_label_pred_5m === 'Rất kém', 'critical', 'aq_pred_very_bad',
    'Dự báo 5 phút: chất lượng sẽ RẤT KÉM (Level 5)');
  maybeAlert('aq_bad_pred', data.air_quality_label_pred_5m === 'Kém', 'warning', 'aq_pred_bad',
    'Dự báo 5 phút: chất lượng sẽ KÉM (Level 4)');
}

// ── Fetch current data from DB (fallback khi WS chưa có data) ──
async function fetchCurrent() {
  try {
    const ctrl  = new AbortController();
    const timer = setTimeout(() => ctrl.abort(), 3000);
    const res   = await fetch(`${CONFIG.API_BASE}/current`, { signal: ctrl.signal });
    clearTimeout(timer);
    if (!res.ok) return;
    const json = await res.json();
    if (json.sensor) handleIncomingData(json.sensor);
  } catch (_) { /* server chưa sẵn sàng, bỏ qua */ }
}

// ── History ──────────────────────────────────────────────────
async function loadHistory() {
  const snap = MockData.generateHistory(activeMetric, activeRange);
  Charts.updateHistory(snap.labels, snap.values, activeMetric, activeRange);

  try {
    const metric = { iaq: 'iaq_index', voc: 'voc_index', eco2: 'eco2_ppm', etoh: 'etoh', temp: 'temperature', hum: 'humidity' }[activeMetric];
    const ctrl   = new AbortController();
    const timer  = setTimeout(() => ctrl.abort(), 3000);
    const res    = await fetch(`${CONFIG.API_BASE}/history?range=${activeRange}&metric=${metric}`, { signal: ctrl.signal });
    clearTimeout(timer);
    const data   = await res.json();
    Charts.updateHistory(data.labels, data.values, activeMetric, activeRange);
  } catch (_) { /* mock data đã hiển thị */ }
}

// ── Event listeners ──────────────────────────────────────────
function bindEvents() {
  document.querySelectorAll('.filter-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.filter-btn').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      activeRange = btn.dataset.range;
      loadHistory();
    });
  });

  document.getElementById('metricSelect').addEventListener('change', (e) => {
    activeMetric = e.target.value;
    loadHistory();
  });

  document.querySelectorAll('.log-filter-btn').forEach(btn => {
    btn.addEventListener('click', () => {
      document.querySelectorAll('.log-filter-btn').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      UI.setLogFilter(btn.dataset.sev);
    });
  });
}

// ── Boot ─────────────────────────────────────────────────────
document.addEventListener('DOMContentLoaded', () => {
  Charts.initAll();
  UI.initLogs(MockData.generateInitialLogs());
  bindEvents();
  loadHistory();
  fetchCurrent();
  initWebSocket();
  console.log('[App] Dashboard initialized — WebSocket mode active');
});