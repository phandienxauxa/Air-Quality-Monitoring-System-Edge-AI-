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

  ws.onopen = () => console.log('[WS] Connected to backend');

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
  Charts.updateVOC(data.timestamp, data.voc_index, data.eco2_ppm);
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

// ── Threshold alerts (ngưỡng Sensirion chuẩn) ───────────────
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

  // IAQ: > 200 → critical
  maybeAlert('iaq_bad', data.iaq_index > 200, 'critical', 'iaq_bad',
    `IAQ vượt ngưỡng 200 — giá trị: ${data.iaq_index.toFixed(0)}`);

  // IAQ: 100–200 → warning
  maybeAlert('iaq_warn', data.iaq_index > 100 && data.iaq_index <= 200, 'warning', 'iaq_warn',
    `IAQ ở mức trung bình — giá trị: ${data.iaq_index.toFixed(0)}`);

  // VOC: > 250 → warning (Sensirion 0-500, baseline 100)
  maybeAlert('voc_high', data.voc_index > 250, 'warning', 'voc_high',
    `VOC index bất thường — giá trị: ${data.voc_index.toFixed(0)}`);

  // eToH: > 200 → warning
  if (data.etoh !== undefined) {
    maybeAlert('etoh_high', data.etoh > 200, 'warning', 'etoh_high',
      `eToH index bất thường — giá trị: ${data.etoh.toFixed(0)}`);
  }

  maybeAlert('aq_bad_pred', data.air_quality_label_pred_5m === 'Kém', 'warning', 'aq_pred_bad',
    'Dự báo chất lượng không khí sẽ KÉM trong 5 phút tới');
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
  initWebSocket();
  console.log('[App] Dashboard initialized — WebSocket mode active');
});