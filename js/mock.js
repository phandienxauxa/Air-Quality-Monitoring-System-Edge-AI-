/**
 * mock.js — Giả lập dữ liệu ESP32 / SHT40-SGP40
 * Update: VOC & eToH dùng thang Sensirion 0-500 (baseline=100)
 */

const MockData = (() => {

  const state = {
    iaq:        2.5,   // Renesas IAQ Rating 1.0–5.0+
    voc:        0.8,   // TVOC mg/m³ (Renesas)
    eco2:       650,
    temp:       29.4,
    hum:        67.2,
    etoh:       95,    // eToH index 0-500
    tvoc_raw:   142,
    eco2_raw:   650,
    aqNow:      'Tốt',
    aqPred:     'Tốt',
    tick:       0,
  };

  // 5 mức theo Renesas IAQ Rating
  function calcAQLabel(iaq) {
    if (iaq < 2.0) return 'Rất tốt';   // Level 1: ≤ 1.9
    if (iaq < 3.0) return 'Tốt';        // Level 2: 2.0–2.9
    if (iaq < 4.0) return 'Trung bình'; // Level 3: 3.0–3.9
    if (iaq < 5.0) return 'Kém';        // Level 4: 4.0–4.9
    return 'Rất kém';                   // Level 5: ≥ 5.0
  }

  function calcPredLabel(iaq, trend) {
    const predicted = iaq + trend * 0.5;
    return calcAQLabel(Math.max(0, Math.min(6, predicted)));
  }

  function walk(val, step, min, max) {
    return Math.min(max, Math.max(min, val + (Math.random() - 0.48) * step));
  }

  function generatePayload() {
    state.tick++;
    const badCycle = Math.sin(state.tick / 30) > 0.7;

    state.iaq  = walk(state.iaq,  badCycle ? 0.4 : 0.1,  1.0,  6.0);
    state.voc  = walk(state.voc,  badCycle ? 0.5 : 0.1,  0.0,  15.0);
    state.eco2 = walk(state.eco2, badCycle ? 25  : 10,   400,  2000);
    state.temp = walk(state.temp, 0.2,                    15,   45);
    state.hum  = walk(state.hum,  0.8,                    20,   95);
    state.etoh = walk(state.etoh, badCycle ? 12  : 4,    0,    500);

    state.tvoc_raw = Math.round(state.voc * 100 + Math.random() * 5);
    state.eco2_raw = Math.round(state.eco2 + Math.random() * 10 - 5);

    const trend     = state.iaq > 3.5 ? 1 : state.iaq < 2.0 ? -1 : 0;
    state.aqNow     = calcAQLabel(state.iaq);
    state.aqPred    = calcPredLabel(state.iaq, trend);

    return {
      timestamp:                 new Date().toISOString(),
      device_id:                 'esp32_gateway_01',
      raw_tvoc:                  state.tvoc_raw,
      raw_eco2:                  state.eco2_raw,
      temperature:               +state.temp.toFixed(1),
      humidity:                  +state.hum.toFixed(1),
      filtered_tvoc:             Math.round(state.tvoc_raw * 0.97),
      filtered_eco2:             Math.round(state.eco2_raw * 0.98),
      normalized_tvoc:           +(state.tvoc_raw / 1000).toFixed(3),
      normalized_eco2:           +(state.eco2_raw / 2000).toFixed(3),
      iaq_index:                 +state.iaq.toFixed(2),
      voc_index:                 +state.voc.toFixed(2),
      eco2_ppm:                  Math.round(state.eco2),
      etoh:                      +state.etoh.toFixed(1),
      air_quality_label_now:     state.aqNow,
      air_quality_label_pred_5m: state.aqPred,
      battery_status:            'external_power',
      network_status:            'good',
      error_code:                0,
      _trend:                    trend,
    };
  }

  function generateHistory(metric, rangeKey) {
    const counts = { '1h': 60, '24h': 96, '7d': 168, '30d': 120 };
    const n = counts[rangeKey] || 60;

    const metricConfig = {
      iaq:  { base: 2.5,  step: 0.15, min: 1.0,  max: 6.0 },
      voc:  { base: 0.8,  step: 0.1,  min: 0.0,  max: 15.0 },
      eco2: { base: 650,  step: 25,   min: 400,  max: 2000 },
      etoh: { base: 90,   step: 12,   min: 0,    max: 500 },
      temp: { base: 29,   step: 0.3,  min: 15,   max: 45 },
      hum:  { base: 65,   step: 1.5,  min: 20,   max: 95 },
    };

    const cfg = metricConfig[metric] || metricConfig.iaq;
    let val   = cfg.base;
    const now = Date.now();
    const intervalMs = { '1h': 60000, '24h': 900000, '7d': 3600000, '30d': 3600000 * 6 }[rangeKey] || 60000;

    const labels = [], values = [];
    for (let i = n - 1; i >= 0; i--) {
      labels.push(new Date(now - i * intervalMs).toISOString());
      val = Math.min(cfg.max, Math.max(cfg.min, val + (Math.random() - 0.48) * cfg.step));
      values.push(+val.toFixed(2));
    }

    return { labels, values };
  }

  function generateInitialLogs() {
    const now = Date.now();
    return [
      { time: new Date(now - 2  * 60000), device: 'esp32_gateway_01', type: 'eco2_high',    severity: 'critical', msg: 'eCO₂ vượt ngưỡng 1200 ppm — giá trị: 1253 ppm' },
      { time: new Date(now - 5  * 60000), device: 'esp32_gateway_01', type: 'iaq_warn',     severity: 'info',     msg: 'IAQ Level 3 — Trung bình (Medium): 3.2' },
      { time: new Date(now - 8  * 60000), device: 'esp32_gateway_01', type: 'aq_degrading', severity: 'warning',  msg: 'IAQ Level 4 — Kém (Poor): 4.1' },
      { time: new Date(now - 12 * 60000), device: 'esp32_gateway_01', type: 'reconnect',    severity: 'info',     msg: 'Node đã kết nối lại sau 3 giây offline' },
      { time: new Date(now - 18 * 60000), device: 'esp32_gateway_01', type: 'offline',      severity: 'critical', msg: 'Node mất kết nối — timeout 5 giây' },
      { time: new Date(now - 25 * 60000), device: 'esp32_gateway_01', type: 'iaq_normal',   severity: 'info',     msg: 'IAQ trở về mức bình thường — giá trị: 72' },
      { time: new Date(now - 40 * 60000), device: 'esp32_gateway_01', type: 'voc_high',     severity: 'warning',  msg: 'VOC index bất thường — giá trị: 278' },
      { time: new Date(now - 55 * 60000), device: 'esp32_gateway_01', type: 'startup',      severity: 'info',     msg: 'Hệ thống khởi động — backend FastAPI v1.0' },
    ];
  }

  return { generatePayload, generateHistory, generateInitialLogs };
})();