#include <Arduino.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <time.h>

// ── Cấu hình ──────────────────────────────────────────────────
#define RXD2       16
#define TXD2       17
#define UART_BAUD  115200

const char* WIFI_SSID  = "TÊN_WIFI";
const char* WIFI_PASS  = "MẬT_KHẨU_WIFI";
const char* WS_HOST    = "192.168.x.x";   // IP PC chạy backend (dùng ipconfig để lấy)
const int   WS_PORT    = 8000;
const char* WS_PATH    = "/ws/ingest";
const char* DEVICE_ID  = "esp32_gateway_01";
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
void sendToBackend(float iaq,  float tvoc,  int   eco2,
                   float etoh, float temp,  float hum,
                   int   err,  float predIaqRaw) {
    JsonDocument doc;

    // Dự báo 5 phút: dùng predicted_iaq từ AI nếu có, ngược lại extrapolate
    float predIaq = (predIaqRaw > 0.0f)
                    ? predIaqRaw
                    : constrain(iaq + (iaq > 3.5f ? 0.3f : iaq < 2.0f ? -0.3f : 0.0f),
                                1.0f, 6.0f);

    doc["timestamp"]                 = getTimestamp();
    doc["device_id"]                 = DEVICE_ID;
    doc["raw_tvoc"]                  = (int)(tvoc * 100);
    doc["raw_eco2"]                  = eco2;
    doc["temperature"]               = round(temp * 100) / 100.0;
    doc["humidity"]                  = round(hum  * 100) / 100.0;
    doc["filtered_tvoc"]             = (int)(tvoc * 97);
    doc["filtered_eco2"]             = (int)(eco2 * 0.98f);
    doc["normalized_tvoc"]           = round(tvoc / 15.0f * 1000) / 1000.0;
    doc["normalized_eco2"]           = round(eco2 / 2000.0f * 1000) / 1000.0;
    doc["iaq_index"]                 = round(iaq  * 100) / 100.0;
    doc["voc_index"]                 = round(tvoc * 1000) / 1000.0;
    doc["eco2_ppm"]                  = eco2;
    doc["etoh"]                      = round(etoh * 1000) / 1000.0;
    doc["air_quality_label_now"]     = aqLabel(iaq);
    doc["air_quality_label_pred_5m"] = aqLabel(predIaq);
    doc["battery_status"]            = "external_power";
    doc["network_status"]            = (WiFi.RSSI() > -70) ? "good" : "weak";
    doc["error_code"]                = err;

    String json;
    serializeJson(doc, json);
    wsClient.sendTXT(json);

    Serial.printf("[TX] IAQ=%.2f TVOC=%.3f eCO2=%d eToH=%.3f T=%.1f H=%.1f Pred=%.2f\n",
                   iaq, tvoc, eco2, etoh, temp, hum, predIaq);
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
    float etoh     = doc["etoh"]          | 0.05f;
    float temp     = doc["temp"]          | 29.0f;
    float hum      = doc["hum"]           | 65.0f;
    int   err      = doc["err"]           | 0;
    int   stable   = doc["stable"]        | 0;
    float pred_iaq = doc["predicted_iaq"] | 0.0f;

    // Chỉ gửi khi ZMOD4410 đã stable (warm-up xong)
    if (!stable) {
        Serial.println("[INFO] ZMOD4410 đang warm-up — bỏ qua tick này");
        return;
    }

    sendToBackend(iaq, tvoc, (int)eco2_f, etoh, temp, hum, err, pred_iaq);
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
    delay(1500);

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
            uartBuffer += c;
        }
    }
}
