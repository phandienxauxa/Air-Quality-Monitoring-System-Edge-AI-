/**
 * ui.js — Cập nhật toàn bộ DOM từ payload nhận được
 * Fixes: ngưỡng Sensirion 0-500, alert-banner warn class,
 *        donut single large, VOC/eToH thang đo mới
 */

const UI = (() => {

  const $ = id => document.getElementById(id);

  // ── Ngưỡng Sensirion chuẩn ───────────────────────────────
  // IAQ  : < 100 Tốt | 100-200 Trung bình | > 200 Kém
  // VOC  : < 150 Tốt | 150-250 Trung bình | > 250 Kém
  // eCO₂ : < 800 Tốt | 800-1200 Trung bình | > 1200 Kém
  // eToH : < 100 Tốt | 100-200 Trung bình | > 200 Kém
  // Temp : < 28°C Tốt | 28-35 Trung bình | > 35 Kém
  // Hum  : 40-70% Tốt | ngoài khoảng đó → Trung bình

  function iaqLevel(v) { return v < 100 ? 'good' : v < 200 ? 'warn' : 'bad'; }
  function vocLevel(v) { return v < 150 ? 'good' : v < 250 ? 'warn' : 'bad'; }
  function eco2Level(v){ return v < 800 ? 'good' : v < 1200 ? 'warn' : 'bad'; }
  function etohLevel(v){ return v < 100 ? 'good' : v < 200 ? 'warn' : 'bad'; }

  function levelToLabel(lvl) {
    return lvl === 'good' ? 'Tốt' : lvl === 'warn' ? 'Trung bình' : 'Kém';
  }

  function levelToColor(lvl) {
    return lvl === 'good' ? 'var(--good)' : lvl === 'warn' ? 'var(--warn)' : 'var(--bad)';
  }

  // calcAQLabel dựa trên IAQ (chỉ số tổng hợp đại diện)
  function calcAQLabel(iaq) { return levelToLabel(iaqLevel(iaq)); }

  function aqColor(label) {
    if (label === 'Tốt')        return 'var(--good)';
    if (label === 'Trung bình') return 'var(--warn)';
    if (label === 'Kém')        return 'var(--bad)';
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
    // IAQ
    if ($('val-iaq')) {
      const lvl = iaqLevel(data.iaq_index);
      $('val-iaq').textContent = data.iaq_index.toFixed(0);
      $('val-iaq').style.color = levelToColor(lvl);
    }
    Charts.updateMiniBar('miniIAQ', data.iaq_index);

    // VOC — thang Sensirion 0-500
    if ($('val-voc')) {
      const lvl = vocLevel(data.voc_index);
      $('val-voc').textContent = data.voc_index.toFixed(0);
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

    // eToH — thang Sensirion 0-500
    if ($('val-etoh') && data.etoh !== undefined) {
      const lvl = etohLevel(data.etoh);
      $('val-etoh').textContent = data.etoh.toFixed(0);
      $('val-etoh').style.color = levelToColor(lvl);
    }
    Charts.updateMiniBar('miniEToH', data.etoh ?? 0);

    // AQ summary card
    const aqLabel = calcAQLabel(data.iaq_index);
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

    // Alert banner — 3 trạng thái: bad / warn / good
    const banner = $('alertBanner');
    const textEl = $('alertText');
    if (!banner || !textEl) return;

    if (nowLabel === 'Kém' || predLabel === 'Kém') {
      banner.className = 'alert-banner bad';
      textEl.textContent = nowLabel === 'Kém'
        ? '⚠ Chất lượng không khí đang ở mức KÉM — cần thông gió ngay!'
        : '⚠ Dự báo chất lượng không khí sẽ KÉM trong 5 phút tới';
    } else if (nowLabel === 'Trung bình' || (nowLabel === 'Trung bình' && trend > 0)) {
      banner.className = 'alert-banner warn';
      textEl.textContent = trend > 0
        ? '⚠ Chất lượng không khí có xu hướng xấu đi'
        : 'Chất lượng không khí ở mức trung bình';
    } else {
      banner.className = 'alert-banner good';
      textEl.textContent = '✓ Chất lượng không khí bình thường';
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