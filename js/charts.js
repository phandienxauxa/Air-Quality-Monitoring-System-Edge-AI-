/**
 * charts.js — Chart.js: realtime, history, mini charts, donut
 * Fixes: WANT→WARN typo, fmtTime receives ISO directly,
 *        minichart fixed colors, single large donut with tab selector,
 *        gradient repatch on theme change, alert-glow animation
 */

const Charts = (() => {

  const ACCENT   = '#a78bfa';
  const GOOD     = '#4ade80';
  const WARN     = '#fb923c';
  const BAD      = '#f87171';
  const INFO     = '#60a5fa';
  const GRID     = 'rgba(45,35,71,0.8)';
  const GRID_LIGHT = 'rgba(167,139,250,0.10)';
  const TEXT_SEC = '#8b7fb8';

  // Fixed colors per metric (minichart & large chart border)
  const METRIC_COLOR = {
    iaq:  ACCENT,
    voc:  WARN,
    eco2: INFO,
    etoh: GOOD,
  };

  function getGridColor() {
    return document.documentElement.classList.contains('light') ? GRID_LIGHT : GRID;
  }

  function getDonutEmpty() {
    return document.documentElement.classList.contains('light')
      ? 'rgba(167,139,250,0.18)'
      : 'rgba(45,35,71,0.5)';
  }

  const MAX_REALTIME_POINTS = 50;
  const MINI_MAX_POINTS     = 20;

  Chart.defaults.color = TEXT_SEC;
  Chart.defaults.font.family = "'DM Sans', sans-serif";
  Chart.defaults.font.size   = 11;

  function makeGradient(ctx, color, height = 200) {
    const g = ctx.createLinearGradient(0, 0, 0, height);
    g.addColorStop(0, color + '40');
    g.addColorStop(1, color + '00');
    return g;
  }

  // Nhận ISO 8601 string → "HH:MM:SS"
  function fmtTime(isoStr, showSec = true) {
    if (!isoStr) return '';
    const d = new Date(isoStr);
    if (isNaN(d)) return isoStr;
    const hh = String(d.getHours()).padStart(2, '0');
    const mm = String(d.getMinutes()).padStart(2, '0');
    const ss = String(d.getSeconds()).padStart(2, '0');
    return showSec ? `${hh}:${mm}:${ss}` : `${hh}:${mm}`;
  }

  function fmtHistoryLabel(raw, range) {
    if (!raw) return '';
    const d = new Date(raw);
    if (isNaN(d)) return raw;
    const hh  = String(d.getHours()).padStart(2, '0');
    const mm  = String(d.getMinutes()).padStart(2, '0');
    const day = `${d.getDate()}/${d.getMonth() + 1}`;
    if (range === '1h' || range === '24h') return `${hh}:${mm}`;
    return `${day} ${hh}:${mm}`;
  }

  function sharedOptions(yLabel = '') {
    return {
      responsive: true,
      maintainAspectRatio: false,
      animation: { duration: 300 },
      interaction: { mode: 'index', intersect: false },
      plugins: {
        legend: {
          display: true, position: 'top', align: 'end',
          labels: { boxWidth: 10, boxHeight: 10, padding: 12, color: TEXT_SEC, font: { size: 11 } }
        },
        tooltip: {
          backgroundColor: '#1a1625', borderColor: '#2d2547', borderWidth: 1,
          titleColor: '#e2d9f3', bodyColor: TEXT_SEC, padding: 10,
        }
      },
      scales: {
        x: { grid: { color: GRID }, ticks: { color: TEXT_SEC, maxTicksLimit: 8, maxRotation: 0 } },
        y: { grid: { color: GRID }, ticks: { color: TEXT_SEC }, title: { display: !!yLabel, text: yLabel, color: TEXT_SEC, font: { size: 10 } } }
      }
    };
  }

  // ── MINI BAR CHARTS ───────────────────────────────────────
  const miniCharts = {};

  // color: fixed per metric
  function initMiniBar(canvasId, color) {
    const canvas = document.getElementById(canvasId);
    if (!canvas) return null;
    const ctx = canvas.getContext('2d');
    const chart = new Chart(ctx, {
      type: 'bar',
      data: {
        labels: Array(MINI_MAX_POINTS).fill(''),
        datasets: [{
          data: Array(MINI_MAX_POINTS).fill(0),
          backgroundColor: color + 'bb',
          borderWidth: 0, borderRadius: 2,
          barPercentage: 0.7, categoryPercentage: 0.8,
        }]
      },
      options: {
        responsive: true, maintainAspectRatio: false, animation: false,
        plugins: { legend: { display: false }, tooltip: { enabled: false } },
        scales: {
          x: { display: false, grid: { display: false } },
          y: { display: false, grid: { display: false }, min: 0 },
        }
      }
    });
    miniCharts[canvasId] = { chart, color };
    return chart;
  }

  function updateMiniBar(canvasId, newVal) {
    const entry = miniCharts[canvasId];
    if (!entry) return;
    const ds = entry.chart.data.datasets[0];
    ds.data.push(newVal);
    entry.chart.data.labels.push('');
    if (ds.data.length > MINI_MAX_POINTS) { ds.data.shift(); entry.chart.data.labels.shift(); }
    entry.chart.update('none');
  }

  function initAllMiniCharts() {
    initMiniBar('miniIAQ',  METRIC_COLOR.iaq);
    initMiniBar('miniVOC',  METRIC_COLOR.voc);
    initMiniBar('miniECO2', METRIC_COLOR.eco2);
    initMiniBar('miniEToH', METRIC_COLOR.etoh);
  }

  // ── 4 SMALL DONUTS (2×2 grid) ────────────────────────────
  const _donutCharts  = {};
  const _donutCurrent = { iaq: 0, voc: 0, eco2: 0, etoh: 0 };

  const DONUT_META = {
    iaq:  { label: 'IAQ Index', max: 500,  unit: '',     good: 100, warn: 200,  canvasId: 'donutIAQ',  valId: 'donut-val-iaq',  badgeId: 'donut-badge-iaq'  },
    voc:  { label: 'VOC Index', max: 500,  unit: '',     good: 150, warn: 250,  canvasId: 'donutVOC',  valId: 'donut-val-voc',  badgeId: 'donut-badge-voc'  },
    eco2: { label: 'eCO₂',     max: 2000, unit: ' ppm', good: 800, warn: 1200, canvasId: 'donutECO2', valId: 'donut-val-eco2', badgeId: 'donut-badge-eco2' },
    etoh: { label: 'eToH',     max: 500,  unit: '',     good: 100, warn: 200,  canvasId: 'donutEToH', valId: 'donut-val-etoh', badgeId: 'donut-badge-etoh' },
  };

  function getDonutColor(metric, val) {
    const m = DONUT_META[metric];
    if (!m) return WARN;
    if (val < m.good) return GOOD;
    if (val < m.warn) return WARN;
    return BAD;
  }

  function _initOneDonut(metric) {
    const meta   = DONUT_META[metric];
    const canvas = document.getElementById(meta.canvasId);
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    _donutCharts[metric] = new Chart(ctx, {
      type: 'doughnut',
      data: {
        datasets: [{
          data: [0, 100],
          backgroundColor: [WARN, getDonutEmpty()],
          borderWidth: 0,
          hoverOffset: 0,
        }]
      },
      options: {
        responsive: true, maintainAspectRatio: false, cutout: '74%',
        animation: { duration: 600, easing: 'easeInOutQuart' },
        plugins: { legend: { display: false }, tooltip: { enabled: false } },
      }
    });
  }

  function initDonuts() {
    Object.keys(DONUT_META).forEach(m => _initOneDonut(m));
  }

  function updateDonut(metric, val) {
    _donutCurrent[metric] = val;
    const chart = _donutCharts[metric];
    const meta  = DONUT_META[metric];
    if (!chart || !meta) return;

    const pct   = Math.min(1, val / meta.max);
    const color = getDonutColor(metric, val);

    chart.data.datasets[0].data            = [pct * 100, (1 - pct) * 100];
    chart.data.datasets[0].backgroundColor = [color, getDonutEmpty()];
    chart.update();

    const valEl   = document.getElementById(meta.valId);
    const badgeEl = document.getElementById(meta.badgeId);
    if (valEl)   valEl.textContent = metric === 'eco2' ? Math.round(val) : val.toFixed(metric === 'etoh' ? 2 : 1);
    if (badgeEl) {
      const lvl = val < meta.good ? 'TỐT' : val < meta.warn ? 'TRUNG BÌNH' : 'KÉM';
      const cls = val < meta.good ? '' : val < meta.warn ? 'warn' : 'bad';
      badgeEl.className   = 'donut-badge' + (cls ? ' ' + cls : '');
      badgeEl.textContent = lvl;
    }
  }

  // ── IAQ REALTIME ──────────────────────────────────────────
  let chartIAQ = null;

  function initIAQ() {
    const ctx  = document.getElementById('chartIAQ').getContext('2d');
    const grad = makeGradient(ctx, METRIC_COLOR.iaq);
    chartIAQ = new Chart(ctx, {
      type: 'line',
      data: { labels: [], datasets: [{ label: 'IAQ Index', data: [], borderColor: METRIC_COLOR.iaq, backgroundColor: grad, borderWidth: 2, pointRadius: 0, pointHoverRadius: 4, tension: 0.4, fill: true }] },
      options: sharedOptions('IAQ'),
    });
  }

  function updateIAQ(isoTs, iaqVal) {
    if (!chartIAQ) return;
    const ds = chartIAQ.data;
    ds.labels.push(fmtTime(isoTs));
    ds.datasets[0].data.push(+iaqVal.toFixed(1));
    // border color vẫn dynamic theo giá trị (chart lớn)
    ds.datasets[0].borderColor = iaqVal < 100 ? GOOD : iaqVal < 200 ? WARN : BAD;
    if (ds.labels.length > MAX_REALTIME_POINTS) { ds.labels.shift(); ds.datasets[0].data.shift(); }
    chartIAQ.update('none');
  }

  // ── eToH REALTIME ────────────────────────────────────────
  let chartEToH = null;

  function initEToH() {
    const ctx  = document.getElementById('chartEToH').getContext('2d');
    const grad = makeGradient(ctx, METRIC_COLOR.etoh);
    chartEToH = new Chart(ctx, {
      type: 'line',
      data: { labels: [], datasets: [{ label: 'eToH Index', data: [], borderColor: METRIC_COLOR.etoh, backgroundColor: grad, borderWidth: 2, pointRadius: 0, pointHoverRadius: 4, tension: 0.4, fill: true }] },
      options: sharedOptions('eToH'),
    });
  }

  function updateEToH(isoTs, etohVal) {
    if (!chartEToH) return;
    const ds = chartEToH.data;
    ds.labels.push(fmtTime(isoTs));
    ds.datasets[0].data.push(+etohVal.toFixed(1));
    ds.datasets[0].borderColor = etohVal < 100 ? GOOD : etohVal < 200 ? WARN : BAD;
    if (ds.labels.length > MAX_REALTIME_POINTS) { ds.labels.shift(); ds.datasets[0].data.shift(); }
    chartEToH.update('none');
  }

  // ── VOC/ECO2 REALTIME ─────────────────────────────────────
  let chartVOC = null;

  function initVOC() {
    const ctx = document.getElementById('chartVOC').getContext('2d');
    chartVOC = new Chart(ctx, {
      type: 'line',
      data: {
        labels: [],
        datasets: [
          { label: 'VOC Index', data: [], borderColor: METRIC_COLOR.voc, backgroundColor: 'transparent', borderWidth: 2, pointRadius: 0, tension: 0.4, yAxisID: 'yVOC' },
          { label: 'eCO₂ (ppm)', data: [], borderColor: METRIC_COLOR.eco2, backgroundColor: 'transparent', borderWidth: 2, pointRadius: 0, tension: 0.4, yAxisID: 'yCO2', borderDash: [4, 2] }
        ]
      },
      options: {
        responsive: true, maintainAspectRatio: false, animation: { duration: 300 },
        interaction: { mode: 'index', intersect: false },
        plugins: {
          legend: { display: true, position: 'top', align: 'end', labels: { boxWidth: 10, boxHeight: 10, padding: 12, color: TEXT_SEC, font: { size: 11 } } },
          tooltip: { backgroundColor: '#1a1625', borderColor: '#2d2547', borderWidth: 1, titleColor: '#e2d9f3', bodyColor: TEXT_SEC, padding: 10 }
        },
        scales: {
          x:    { grid: { color: GRID }, ticks: { color: TEXT_SEC, maxTicksLimit: 8, maxRotation: 0 } },
          yVOC: { position: 'left',  grid: { color: GRID }, ticks: { color: METRIC_COLOR.voc }, title: { display: true, text: 'VOC', color: METRIC_COLOR.voc, font: { size: 10 } } },
          yCO2: { position: 'right', grid: { display: false }, ticks: { color: METRIC_COLOR.eco2 }, title: { display: true, text: 'ppm', color: METRIC_COLOR.eco2, font: { size: 10 } } },
        }
      }
    });
  }

  function updateVOC(isoTs, voc, eco2) {
    if (!chartVOC) return;
    const ds = chartVOC.data;
    ds.labels.push(fmtTime(isoTs));
    ds.datasets[0].data.push(+voc.toFixed(1));
    ds.datasets[1].data.push(Math.round(eco2));
    if (ds.labels.length > MAX_REALTIME_POINTS) { ds.labels.shift(); ds.datasets.forEach(d => d.data.shift()); }
    chartVOC.update('none');
  }

  // ── ENV REALTIME ──────────────────────────────────────────
  let chartEnv = null;

  function initEnv() {
    const ctx = document.getElementById('chartEnv').getContext('2d');
    chartEnv = new Chart(ctx, {
      type: 'line',
      data: {
        labels: [],
        datasets: [
          { label: 'Nhiệt độ (°C)', data: [], borderColor: BAD,    backgroundColor: 'transparent', borderWidth: 2, pointRadius: 0, tension: 0.4, yAxisID: 'yTemp' },
          { label: 'Độ ẩm (%)',     data: [], borderColor: ACCENT, backgroundColor: 'transparent', borderWidth: 2, pointRadius: 0, tension: 0.4, yAxisID: 'yHum', borderDash: [4, 2] }
        ]
      },
      options: {
        responsive: true, maintainAspectRatio: false, animation: { duration: 300 },
        interaction: { mode: 'index', intersect: false },
        plugins: {
          legend: { display: true, position: 'top', align: 'end', labels: { boxWidth: 10, boxHeight: 10, padding: 12, color: TEXT_SEC, font: { size: 11 } } },
          tooltip: { backgroundColor: '#1a1625', borderColor: '#2d2547', borderWidth: 1, titleColor: '#e2d9f3', bodyColor: TEXT_SEC, padding: 10 }
        },
        scales: {
          x:     { grid: { color: GRID }, ticks: { color: TEXT_SEC, maxTicksLimit: 8, maxRotation: 0 } },
          yTemp: { position: 'left',  grid: { color: GRID }, ticks: { color: BAD    }, title: { display: true, text: '°C', color: BAD,    font: { size: 10 } } },
          yHum:  { position: 'right', grid: { display: false }, ticks: { color: ACCENT }, title: { display: true, text: '%',  color: ACCENT, font: { size: 10 } } },
        }
      }
    });
  }

  function updateEnv(isoTs, temp, hum) {
    if (!chartEnv) return;
    const ds = chartEnv.data;
    ds.labels.push(fmtTime(isoTs));
    ds.datasets[0].data.push(+temp.toFixed(1));
    ds.datasets[1].data.push(+hum.toFixed(1));
    if (ds.labels.length > MAX_REALTIME_POINTS) { ds.labels.shift(); ds.datasets.forEach(d => d.data.shift()); }
    chartEnv.update('none');
  }

  // ── HISTORY ───────────────────────────────────────────────
  let chartHistory      = null;
  let _currentRange     = '1h';
  let _historyRawLabels = [];

  const metricMeta = {
    iaq:  { label: 'IAQ Index', color: METRIC_COLOR.iaq,  unit: '' },
    voc:  { label: 'VOC Index', color: METRIC_COLOR.voc,  unit: '' },
    eco2: { label: 'eCO₂',     color: METRIC_COLOR.eco2, unit: ' ppm' },
    etoh: { label: 'eToH',     color: METRIC_COLOR.etoh, unit: '' },
    temp: { label: 'Nhiệt độ', color: BAD,               unit: ' °C' },
    hum:  { label: 'Độ ẩm',   color: ACCENT,             unit: ' %' },
  };

  function downsample(labels, values, n) {
    if (labels.length <= n) return { labels, values };
    const step = (labels.length - 1) / (n - 1);
    const sl = [], sv = [];
    for (let i = 0; i < n; i++) {
      const idx = Math.round(i * step);
      sl.push(labels[idx]);
      sv.push(values[idx]);
    }
    return { labels: sl, values: sv };
  }

  function initHistory(labels, values, metric, range) {
    _currentRange = range || _currentRange;
    const ctx  = document.getElementById('chartHistory').getContext('2d');
    const meta = metricMeta[metric] || metricMeta.iaq;
    const grad = makeGradient(ctx, meta.color);

    const targetPoints = _currentRange === '1h' ? 120 : _currentRange === '24h' ? 144 : _currentRange === '7d' ? 168 : 180;
    const ds = downsample(labels, values, targetPoints);
    labels = ds.labels;
    values = ds.values;

    const gapThreshold = { '1h': 180, '24h': 2700, '7d': 10800, '30d': 64800 };
    const maxGap = gapThreshold[_currentRange] || 60;
    for (let i = labels.length - 1; i > 0; i--) {
      const diff = (new Date(labels[i]) - new Date(labels[i - 1])) / 1000;
      if (diff > maxGap) { labels.splice(i, 0, labels[i]); values.splice(i, 0, null); }
    }

    _historyRawLabels = labels;
    const fmtLabels   = labels.map(l => fmtHistoryLabel(l, _currentRange));
    const gridColor   = getGridColor();

    if (chartHistory) {
      chartHistory.data.labels = fmtLabels;
      const dataset = chartHistory.data.datasets[0];
      dataset.data            = values;
      dataset.label           = meta.label;
      dataset.borderColor     = meta.color;
      dataset.backgroundColor = grad;
      chartHistory.options.scales.x.grid.color = gridColor;
      chartHistory.options.scales.y.grid.color = gridColor;
      chartHistory.options.scales.y.title.text  = meta.label + meta.unit;
      chartHistory.update();
      return;
    }

    chartHistory = new Chart(ctx, {
      type: 'line',
      data: {
        labels: fmtLabels,
        datasets: [{ label: meta.label, data: values, borderColor: meta.color, backgroundColor: grad, borderWidth: 2, pointRadius: 0, pointHoverRadius: 4, tension: 0.4, fill: true }]
      },
      options: {
        responsive: true, maintainAspectRatio: false, animation: { duration: 400 },
        interaction: { mode: 'index', intersect: false },
        plugins: {
          legend: { display: true, position: 'top', align: 'end', labels: { boxWidth: 10, boxHeight: 10, padding: 12, color: TEXT_SEC, font: { size: 11 } } },
          tooltip: {
            backgroundColor: '#1a1625', borderColor: '#2d2547', borderWidth: 1,
            titleColor: '#e2d9f3', bodyColor: TEXT_SEC, padding: 10,
            callbacks: {
              title: (items) => {
                const raw = _historyRawLabels[items[0].dataIndex];
                if (!raw) return items[0].label;
                const d = new Date(raw);
                if (isNaN(d)) return items[0].label;
                return d.toLocaleString('vi-VN', { day: '2-digit', month: '2-digit', hour: '2-digit', minute: '2-digit', second: '2-digit' });
              }
            }
          }
        },
        scales: {
          x: { grid: { color: gridColor, drawBorder: false }, ticks: { color: TEXT_SEC, maxRotation: 0, autoSkip: true, autoSkipPadding: 40, maxTicksLimit: 8 } },
          y: { grid: { color: gridColor, drawBorder: false }, ticks: { color: TEXT_SEC }, title: { display: true, text: meta.label + meta.unit, color: TEXT_SEC, font: { size: 10 } } }
        }
      }
    });
  }

  function updateHistory(labels, values, metric, range) {
    initHistory(labels, values, metric, range);
  }

  // ── THEME ─────────────────────────────────────────────────
  function applyTheme(isLight) {
    const gridColor   = isLight ? 'rgba(167,139,250,0.15)' : 'rgba(45,35,71,0.8)';
    const tooltipBg   = isLight ? '#ffffff'  : '#1a1625';
    const tooltipBord = isLight ? '#ddd6fe'  : '#2d2547';
    const tooltipTit  = isLight ? '#1e1a2e'  : '#e2d9f3';
    const tooltipBody = isLight ? '#5b4d8a'  : '#8b7fb8';
    const emptyColor  = isLight ? 'rgba(167,139,250,0.18)' : 'rgba(45,35,71,0.5)';

    Chart.defaults.color = tooltipBody;

    function patchChart(c) {
      if (!c) return;
      if (c.options.scales) {
        Object.values(c.options.scales).forEach(sc => {
          if (sc.grid && sc.grid.color !== undefined) sc.grid.color = gridColor;
        });
      }
      if (c.options.plugins?.tooltip) {
        const t = c.options.plugins.tooltip;
        t.backgroundColor = tooltipBg;
        t.borderColor     = tooltipBord;
        t.titleColor      = tooltipTit;
        t.bodyColor       = tooltipBody;
      }
      if (c.options.plugins?.legend?.labels) {
        c.options.plugins.legend.labels.color = tooltipBody;
      }
      // Re-patch gradient datasets
      c.data.datasets.forEach(ds => {
        if (ds.fill && ds.borderColor && typeof ds.backgroundColor !== 'string') {
          // backgroundColor là gradient object → rebuild
          const canvas = c.canvas;
          const ctx2   = canvas.getContext('2d');
          ds.backgroundColor = makeGradient(ctx2, ds.borderColor, canvas.clientHeight || 200);
        }
      });
      c.update();
    }

    [chartIAQ, chartEToH, chartVOC, chartEnv, chartHistory].forEach(patchChart);

    // Donut empty segment — patch all 4 donuts
    Object.values(_donutCharts).forEach(c => {
      if (!c) return;
      const ds2 = c.data.datasets[0];
      if (Array.isArray(ds2.backgroundColor) && ds2.backgroundColor.length >= 2) {
        ds2.backgroundColor[1] = emptyColor;
      }
      c.update();
    });

    // Mini charts: just update color
    Object.values(miniCharts).forEach(entry => {
      if (!entry) return;
      entry.chart.update();
    });
  }

  // ── INIT ALL ──────────────────────────────────────────────
  function initAll() {
    initAllMiniCharts();
    initDonuts();
    initIAQ();
    initEToH();
    initVOC();
    initEnv();
    const { labels, values } = MockData.generateHistory('iaq', '1h');
    initHistory(labels, values, 'iaq', '1h');
  }

  return {
    initAll,
    updateIAQ, updateEToH, updateVOC, updateEnv, updateHistory,
    updateMiniBar, updateDonut,
    applyTheme,
  };
})();