#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <time.h>

#define RXD2       16
#define TXD2       17
#define UART_BAUD  115200

const char* WIFI_SSID  = "OXY 24H 5G";
const char* WIFI_PASS  = "oxy24hcoffee";
const char* WS_HOST    = "192.168.5.195";   
const int   WS_PORT    = 8000;
const char* WS_PATH    = "/ws/ingest";
const char* DEVICE_ID = "ck_ra6m5_gateway_01";
const long  GMT_OFFSET = 7 * 3600;         // UTC+7

// ── State ──────────────────────────────────────────────────────
WebSocketsClient wsClient;
String uartBuffer = "";
bool   wsConnected = false;

// ── Timestamp ISO 8601 ─────────────────────────────────────────
String getTimestamp() {
    struct tm t;
    if (!getLocalTime(&t)) return "1970-01-01T00:00:00+07:00";
    char buf[30];
    strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%S+07:00", &t);
    return String(buf);
}

// ── Nhãn chất lượng 5 mức Renesas ─────────────────────────────
const char* aqLabel(float iaq) {
    if (iaq < 2.0f) return "Rất tốt";
    if (iaq < 3.0f) return "Tốt";
    if (iaq < 4.0f) return "Trung bình";
    if (iaq < 5.0f) return "Kém";
    return "Rất kém";
}

// ── Gửi lên backend WebSocket ──────────────────────────────────
void sendToBackend(float iaq,     float tvoc,  float eco2,
                   float etoh,    float temp,  float hum,
                   int   err,     float predIaqRaw,
                   float rel_iaq) {
    JsonDocument doc;

    float predIaq = (predIaqRaw > 0.0f)
                    ? predIaqRaw
                    : constrain(iaq + (iaq > 3.5f ? 0.3f : iaq < 2.0f ? -0.3f : 0.0f),
                                1.0f, 6.0f);

    #define R4(x) (round((x) * 10000.0f) / 10000.0f)

    doc["timestamp"]                 = getTimestamp();
    doc["device_id"]                 = DEVICE_ID;
    doc["raw_tvoc"]                  = (int)(tvoc * 100);
    doc["raw_eco2"]                  = R4(eco2);
    doc["temperature"]               = R4(temp);
    doc["humidity"]                  = R4(hum);
    doc["filtered_tvoc"]             = (int)(tvoc * 97);
    doc["filtered_eco2"]             = R4(eco2 * 0.98f);
    doc["normalized_tvoc"]           = R4(tvoc / 15.0f);
    doc["normalized_eco2"]           = R4(eco2 / 2000.0f);
    doc["iaq_index"]                 = R4(iaq);
    doc["voc_index"]                 = R4(tvoc);
    doc["eco2_ppm"]                  = R4(eco2);
    doc["etoh"]                      = R4(etoh);
    doc["rel_iaq"]                   = R4(rel_iaq);
    doc["predicted_iaq"]             = R4(predIaq);
    doc["air_quality_label_now"]     = aqLabel(iaq);
    doc["air_quality_label_pred_5m"] = aqLabel(predIaq);
    doc["battery_status"]            = "good";
    doc["network_status"]            = (WiFi.RSSI() > -70) ? "good" : "weak";
    doc["error_code"]                = err;

    #undef R4

    String json;
    serializeJson(doc, json);
    wsClient.sendTXT(json);

    Serial.printf("[TX] IAQ=%.4f TVOC=%.4f eCO2=%.4f eToH=%.4f T=%.2f H=%.2f RelIAQ=%.4f Pred=%.4f\n",
                   iaq, tvoc, eco2, etoh, temp, hum, rel_iaq, predIaq);
}

// ── Xử lý JSON từ UART (RA6M5) ────────────────────────────────
void processUartLine(const String& line) {
    JsonDocument doc;
    if (deserializeJson(doc, line) != DeserializationError::Ok) {
        Serial.println("[ERR] UART JSON invalid: " + line.substring(0, 60));
        return;
    }

    float iaq      = doc["iaq"]           | 2.5f;
    float tvoc     = doc["tvoc"]          | 0.8f;
    float eco2_f   = doc["eco2"]          | 600.0f;
    float etoh     = doc["etoh"]          | 0.0f;
    float temp     = doc["temp"]          | 29.0f;
    float hum      = doc["hum"]           | 65.0f;
    float rel_iaq  = doc["rel_iaq"]       | 0.0f;
    int   err      = doc["err"]           | 0;
    int   stable   = doc["stable"]        | 0;
    float pred_iaq = doc["predicted_iaq"] | 0.0f;

    // Nếu ZMOD4410 chưa stable, vẫn gửi nhưng dùng IAQ mặc định an toàn
    if (!stable) {
        Serial.println("[INFO] ZMOD4410 warm-up — gửi temp/hum, IAQ giữ mặc định");
        sendToBackend(1.0f, 0.0f, eco2_f, 0.0f, temp, hum, err, 0.0f, 0.0f);
        return;
    }
    sendToBackend(iaq, tvoc, eco2_f, etoh, temp, hum, err, pred_iaq, rel_iaq);
}

// ── WebSocket event ────────────────────────────────────────────
void onWsEvent(WStype_t type, uint8_t* payload, size_t len) {
    switch (type) {
        case WStype_CONNECTED:
            wsConnected = true;
            Serial.println("[WS] Connected to backend");
            break;
        case WStype_DISCONNECTED:
            wsConnected = false;
            Serial.println("[WS] Disconnected — retrying...");
            break;
        case WStype_TEXT:
            Serial.printf("[WS] ACK: %.*s\n", (int)len, payload);
            break;
        default: break;
    }
}

// ── Setup ──────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    Serial2.begin(UART_BAUD, SERIAL_8N1, RXD2, TXD2);

    Serial.print("[Init] WiFi connecting");
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
    Serial.printf("\n[Init] IP: %s | RSSI: %d dBm\n",
                   WiFi.localIP().toString().c_str(), WiFi.RSSI());

    configTime(GMT_OFFSET, 0, "pool.ntp.org", "time.google.com");
    { // Wait for NTP sync (up to 10s)
        time_t now = time(nullptr);
        int retry = 0;
        while (now < 8 * 3600 * 2 && retry < 20) { delay(500); now = time(nullptr); retry++; }
        if (retry >= 20) Serial.println("[WARN] NTP sync timeout — timestamp có thể sai");
    }

    wsClient.begin(WS_HOST, WS_PORT, WS_PATH);
    wsClient.onEvent(onWsEvent);
    wsClient.setReconnectInterval(3000);
    Serial.println("[Init] Ready — đang chờ dữ liệu từ CK-RA6M5 qua UART...");
}

// ── Loop ───────────────────────────────────────────────────────
void loop() {
    wsClient.loop();

    while (Serial2.available()) {
        char c = (char)Serial2.read();
        if (c == '\n') {
            uartBuffer.trim();
            if (uartBuffer.length() > 2) {
                if (wsConnected)
                    processUartLine(uartBuffer);
                else
                    Serial.println("[WARN] WS chưa kết nối, bỏ qua: " + uartBuffer.substring(0, 40));
            }
            uartBuffer = "";
        } else if (c != '\r') {
            if (uartBuffer.length() < 512)
                uartBuffer += c;
            else {
                Serial.println("[ERR] UART buffer tràn, xóa");
                uartBuffer = "";
            }
        }
    }
}
