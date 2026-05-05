/**
 * ui.js — Cập nhật toàn bộ DOM từ payload nhận được
 * Fixes: ngưỡng Sensirion 0-500, alert-banner warn class,
 *        donut single large, VOC/eToH thang đo mới
 */

const UI = (() => {

  const $ = id => document.getElementById(id);

  // ── Ngưỡng Renesas 5 mức ─────────────────────────────────
  // IAQ  : < 2.0 Rất tốt | 2.0–2.9 Tốt | 3.0–3.9 Trung bình | 4.0–4.9 Kém | ≥ 5.0 Rất kém
  // TVOC : < 0.3 Rất tốt | 0.3–1.0 Tốt | 1.0–3.0 Trung bình | 3.0–10.0 Kém | > 10.0 Rất kém
  // eCO₂ : < 800 Tốt | 800–1200 Trung bình | > 1200 Kém
  // eToH : < 100 Tốt | 100–200 Trung bình | > 200 Kém

  // IAQ: Level 1-2 → green | Level 3 → yellow | Level 4-5 → red
  function iaqLevel(v) { return v < 3.0 ? 'good' : v < 4.0 ? 'warn' : 'bad'; }
  // TVOC mg/m³: Level 1-2 → green | Level 3 → yellow | Level 4-5 → red
  function vocLevel(v) { return v < 1.0 ? 'good' : v < 3.0 ? 'warn' : 'bad'; }
  function eco2Level(v){ return v < 800 ? 'good' : v < 1200 ? 'warn' : 'bad'; }
  function etohLevel(v){ return v < 100 ? 'good' : v < 200 ? 'warn' : 'bad'; }

  function levelToColor(lvl) {
    return lvl === 'good' ? 'var(--good)' : lvl === 'warn' ? 'var(--warn)' : 'var(--bad)';
  }

  // 5 nhãn Renesas — dùng trực tiếp từ payload
  function aqColor(label) {
    if (label === 'Rất tốt')    return 'var(--good)';
    if (label === 'Tốt')        return 'var(--good)';
    if (label === 'Trung bình') return 'var(--warn)';
    if (label === 'Kém')        return 'var(--bad)';
    if (label === 'Rất kém')    return 'var(--bad)';
    return 'var(--accent)';
  }

  function tempBarPct(temp) { return Math.min(100, ((temp - 15) / 30) * 100); }
  function humBarPct(hum)   { return Math.min(100, hum); }

  function setBar(barId, pct, color) {
    const el = $(barId);
    if (!el) return;
    el.style.width = pct + '%';
    el.style.background = color;
  }

  function formatTs(isoStr) {
    if (!isoStr) return '—';
    return new Date(isoStr).toLocaleTimeString('vi-VN', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  }

  // ── Badge helper ──────────────────────────────────────────
  function setBadge(id, lvl) {
    const el = $(id);
    if (!el) return;
    el.className = 'donut-badge' + (lvl === 'good' ? '' : lvl === 'warn' ? ' warn' : ' bad');
    el.textContent = levelToLabel(lvl);
  }

  // ── KPI cards ─────────────────────────────────────────────
  function updateKPI(data) {
    // IAQ — Renesas 1.0–5.0, hiển thị 2 chữ số thập phân
    if ($('val-iaq')) {
      const lvl = iaqLevel(data.iaq_index);
      $('val-iaq').textContent = data.iaq_index.toFixed(2);
      $('val-iaq').style.color = levelToColor(lvl);
    }
    Charts.updateMiniBar('miniIAQ', data.iaq_index);

    // VOC — TVOC mg/m³, hiển thị 2 chữ số thập phân
    if ($('val-voc')) {
      const lvl = vocLevel(data.voc_index);
      $('val-voc').textContent = data.voc_index.toFixed(2);
      $('val-voc').style.color = levelToColor(lvl);
    }
    Charts.updateMiniBar('miniVOC', data.voc_index);

    // ECO2
    if ($('val-eco2')) {
      const lvl = eco2Level(data.eco2_ppm);
      $('val-eco2').textContent = data.eco2_ppm;
      $('val-eco2').style.color = levelToColor(lvl);
    }
    Charts.updateMiniBar('miniECO2', data.eco2_ppm);

    // Temp
    if ($('val-temp')) {
      $('val-temp').textContent = data.temperature.toFixed(1);
      $('val-temp').style.color = 'var(--bad)';
    }
    setBar('bar-temp', tempBarPct(data.temperature), 'var(--bad)');

    // Hum
    if ($('val-hum')) {
      $('val-hum').textContent = data.humidity.toFixed(1);
      $('val-hum').style.color = 'var(--accent)';
    }
    setBar('bar-hum', humBarPct(data.humidity), 'var(--accent)');

    // eToH — Sensirion 0-500
    if ($('val-etoh') && data.etoh !== undefined) {
      const lvl = etohLevel(data.etoh);
      $('val-etoh').textContent = data.etoh.toFixed(0);
      $('val-etoh').style.color = levelToColor(lvl);
    }
    Charts.updateMiniBar('miniEToH', data.etoh ?? 0);

    // AQ summary card — dùng nhãn trực tiếp từ payload (5 mức Renesas)
    const aqLabel = data.air_quality_label_now || 'Tốt';
    if ($('val-aq'))  { $('val-aq').textContent = aqLabel; $('val-aq').style.color = aqColor(aqLabel); }
    if ($('aq-icon')) $('aq-icon').style.color = aqColor(aqLabel);
  }

  // ── Single large donut ────────────────────────────────────
  function updateDonuts(data) {
    Charts.updateDonut('iaq',  data.iaq_index);
    Charts.updateDonut('voc',  data.voc_index);
    Charts.updateDonut('eco2', data.eco2_ppm);
    Charts.updateDonut('etoh', data.etoh ?? 0);
  }

  // ── Prediction / trend ────────────────────────────────────
  function updatePrediction(data) {
    const nowLabel  = data.air_quality_label_now;
    const predLabel = data.air_quality_label_pred_5m;
    const trend     = data._trend || 0;

    const predNow    = $('predNow');
    const predFuture = $('predFuture');
    if (predNow)    { predNow.textContent    = nowLabel;   predNow.style.color    = aqColor(nowLabel); }
    if (predFuture) { predFuture.textContent = predLabel;  predFuture.style.color = aqColor(predLabel); }

    let trendIcon = '→', trendText = 'Ổn định', trendColor = 'var(--text-secondary)';
    if (trend > 0) { trendIcon = '↑'; trendText = 'Đang xấu đi';    trendColor = 'var(--bad)'; }
    if (trend < 0) { trendIcon = '↓'; trendText = 'Đang cải thiện'; trendColor = 'var(--good)'; }
    if ($('trendIcon')) { $('trendIcon').textContent = trendIcon; $('trendIcon').style.color = trendColor; }
    if ($('trendText')) { $('trendText').textContent = trendText; $('trendText').style.color = trendColor; }

    // Alert banner — 5 mức Renesas
    const banner = $('alertBanner');
    const textEl = $('alertText');
    if (!banner || !textEl) return;

    if (nowLabel === 'Rất kém' || predLabel === 'Rất kém') {
      banner.className = 'alert-banner bad';
      textEl.textContent = nowLabel === 'Rất kém'
        ? '⛔ Không khí RẤT KÉM (Level 5) — cần rời khỏi khu vực ngay!'
        : '⛔ Dự báo không khí sẽ RẤT KÉM trong 5 phút tới';
    } else if (nowLabel === 'Kém' || predLabel === 'Kém') {
      banner.className = 'alert-banner bad';
      textEl.textContent = nowLabel === 'Kém'
        ? '⚠ Chất lượng không khí KÉM (Level 4) — cần thông gió ngay!'
        : '⚠ Dự báo chất lượng không khí sẽ KÉM trong 5 phút tới';
    } else if (nowLabel === 'Trung bình') {
      banner.className = 'alert-banner warn';
      textEl.textContent = trend > 0
        ? '⚠ Không khí ở mức Trung bình (Level 3) và đang xấu đi'
        : 'Chất lượng không khí ở mức Trung bình (Level 3)';
    } else {
      banner.className = 'alert-banner good';
      textEl.textContent = nowLabel === 'Rất tốt'
        ? '✓ Không khí Rất tốt (Level 1) — Clean Hygienic Air'
        : '✓ Chất lượng không khí Tốt (Level 2)';
    }
  }

  // ── Detail stats ──────────────────────────────────────────
  function updateDetail(data) {
    if ($('sd-etoh') && data.etoh !== undefined) {
      $('sd-etoh').textContent = data.etoh.toFixed(0);
      setBadge('sd-etoh-badge', etohLevel(data.etoh));
    }
    if ($('sd-tvoc-raw'))  $('sd-tvoc-raw').textContent  = data.raw_tvoc;
    if ($('sd-tvoc-fil'))  $('sd-tvoc-fil').textContent  = data.filtered_tvoc;
    if ($('sd-eco2-raw'))  $('sd-eco2-raw').textContent  = data.raw_eco2 + ' ppm';
    if ($('sd-eco2-fil'))  $('sd-eco2-fil').textContent  = data.filtered_eco2 + ' ppm';
    if ($('sd-tvoc-norm')) $('sd-tvoc-norm').textContent = data.normalized_tvoc?.toFixed(3) ?? '—';
    if ($('sd-eco2-norm')) $('sd-eco2-norm').textContent = data.normalized_eco2?.toFixed(3) ?? '—';
    if ($('sd-battery'))   $('sd-battery').textContent   = data.battery_status;
    if ($('sd-network'))   $('sd-network').textContent   = data.network_status;
    if ($('sd-error'))     $('sd-error').textContent     = data.error_code === 0 ? 'OK' : `ERR ${data.error_code}`;
  }

  // ── Node status ───────────────────────────────────────────
  function updateNodeStatus(online, deviceId, timestamp) {
    const badge = $('nodeBadge');
    const dot   = $('nodeDot');
    const label = $('nodeLabel');

    if (online) {
      if (badge) badge.className = 'node-badge';
      if (dot)   { dot.style.background = 'var(--good)'; dot.style.boxShadow = '0 0 6px var(--good)'; }
      if (label) label.textContent = 'ONLINE';
    } else {
      if (badge) badge.className = 'node-badge offline';
      if (dot)   { dot.style.background = 'var(--bad)'; dot.style.boxShadow = '0 0 6px var(--bad)'; }
      if (label) label.textContent = 'OFFLINE';
    }

    if (deviceId  && $('deviceId'))   $('deviceId').textContent   = deviceId;
    if (timestamp && $('lastUpdate')) $('lastUpdate').textContent = formatTs(timestamp);
  }

  // ── Update all ────────────────────────────────────────────
  function updateAll(data) {
    updateKPI(data);
    updateDonuts(data);
    updatePrediction(data);
    updateDetail(data);
    updateNodeStatus(true, data.device_id, data.timestamp);
  }

  // ── Logs ──────────────────────────────────────────────────
  let allLogs = [];
  let activeSev = 'all';

  function formatLogTime(date) {
    return date.toLocaleTimeString('vi-VN', { hour: '2-digit', minute: '2-digit', second: '2-digit' });
  }

  function renderLogs() {
    const tbody = $('logTableBody');
    if (!tbody) return;
    const filtered = activeSev === 'all' ? allLogs : allLogs.filter(l => l.severity === activeSev);
    tbody.innerHTML = '';
    filtered.forEach(log => {
      const tr = document.createElement('tr');
      tr.innerHTML = `
        <td>${formatLogTime(log.time)}</td>
        <td>${log.device}</td>
        <td>${log.type}</td>
        <td><span class="sev-badge ${log.severity}">${log.severity.toUpperCase()}</span></td>
        <td>${log.msg}</td>
      `;
      tbody.appendChild(tr);
    });
  }

  function initLogs(logs)  { allLogs = [...logs]; renderLogs(); }
  function addLog(log)     { allLogs.unshift(log); if (allLogs.length > 100) allLogs.pop(); renderLogs(); }
  function setLogFilter(s) { activeSev = s; renderLogs(); }

  return { updateAll, updateNodeStatus, initLogs, addLog, setLogFilter };
})();