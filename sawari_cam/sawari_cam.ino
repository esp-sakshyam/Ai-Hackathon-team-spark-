/*
 * ============================================================================
 *  SAWARI ESP32-CAM — Passenger Occupancy Detection
 * ============================================================================
 *
 *  Features
 *  ────────
 *  1. Captures a JPEG image every 30 seconds.
 *  2. POSTs the image as multipart/form-data to the Sawari passenger API.
 *  3. Blinks the on-board flash LED while the upload is in progress.
 *  4. Hold the BOOT button (GPIO 0) for 3 seconds to launch WiFiManager
 *     captive portal for on-the-fly WiFi + API URL + Vehicle ID config.
 *  5. API URL and Vehicle ID are configurable via the portal and saved
 *     to SPIFFS so they persist across reboots.
 *
 *  Hardware
 *  ────────
 *  • AI-Thinker ESP32-CAM module
 *  • GPIO 4  — Flash LED  (blinks during upload)
 *  • GPIO 33 — Status LED  (heartbeat / WiFi status, active LOW)
 *  • GPIO 0  — BOOT button (long-press → WiFi config portal)
 *
 *  Libraries (install via Arduino Library Manager)
 *  ────────
 *  • WiFiManager by tzapu  (tested ≥ 2.0.17)
 *  • ArduinoJson by Benoît Blanchon (tested ≥ 7.x)
 *
 *  Board Settings (Arduino IDE)
 *  ────────
 *  Board        : AI Thinker ESP32-CAM
 *  Partition     : Huge APP (3 MB No OTA / 1 MB SPIFFS)
 *  Upload Speed  : 115200
 *
 * ============================================================================
 */

#include "config.h"

// ─── ESP32 / Camera core ────────────────────────────────────────────────────
#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>

// ─── WiFiManager (captive portal for WiFi config) ───────────────────────────
#include <WiFiManager.h>

// ─── ArduinoJson (parse server response + config persistence) ───────────────
#include <ArduinoJson.h>

// ─── SPIFFS (persistent storage for custom portal parameters) ───────────────
#include <SPIFFS.h>

// ═══════════════════════════════════════════════════════════════════════════
//  GLOBALS
// ═══════════════════════════════════════════════════════════════════════════

WiFiManager       wifiManager;
unsigned long     lastCaptureTime    = 0;
unsigned long     lastWiFiCheckTime  = 0;
volatile bool     portalRequested    = false;   // Set by button logic
unsigned long     buttonPressStart   = 0;       // Tracks BOOT hold duration
bool              buttonHeld         = false;

// LED blink state (non-blocking blink during upload)
unsigned long     ledBlinkTimer      = 0;
bool              ledBlinkState      = false;
static const int  LED_BLINK_INTERVAL = 150;     // ms

// ─── Runtime config (loaded from SPIFFS, editable via portal) ───────────────
char cfgServerUrl[MAX_URL_LEN]  = "";
char cfgVehicleId[MAX_VID_LEN]  = "";
bool shouldSaveConfig           = false;        // Flag set by WiFiManager callback

// ═══════════════════════════════════════════════════════════════════════════
//  FORWARD DECLARATIONS
// ═══════════════════════════════════════════════════════════════════════════

bool     initCamera();
void     captureAndUpload();
bool     uploadImage(camera_fb_t *fb);
void     checkBootButton();
void     startConfigPortal();
void     loadConfig();
void     saveConfig();
void     saveConfigCallback();
void     statusLedOn();
void     statusLedOff();
void     flashLedOn();
void     flashLedOff();
void     blinkFlashLed();

// ═══════════════════════════════════════════════════════════════════════════
//  SETUP
// ═══════════════════════════════════════════════════════════════════════════

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.println(F("===================================="));
    Serial.println(F(" SAWARI CAM — Passenger Detection"));
    Serial.println(F("===================================="));

    // ── Pin modes ───────────────────────────────────────────────────────
    pinMode(PIN_FLASH_LED,   OUTPUT);
    pinMode(PIN_STATUS_LED,  OUTPUT);
    pinMode(PIN_BOOT_BUTTON, INPUT_PULLUP);

    flashLedOff();
    statusLedOff();

    // ── SPIFFS + load saved config ──────────────────────────────────────
    if (!SPIFFS.begin(true)) {
        Serial.println(F("[SPIFFS] Mount failed — using defaults"));
    } else {
        Serial.println(F("[SPIFFS] Mounted OK"));
    }
    loadConfig();

    Serial.printf("[CFG] API URL   : %s\n", cfgServerUrl);
    Serial.printf("[CFG] Vehicle ID: %s\n", cfgVehicleId);

    // ── Camera init ─────────────────────────────────────────────────────
    if (!initCamera()) {
        Serial.println(F("[ERROR] Camera init failed — restarting in 5 s"));
        delay(5000);
        ESP.restart();
    }
    Serial.println(F("[OK] Camera initialised"));

    // ── WiFi ────────────────────────────────────────────────────────────
    wifiManager.setConfigPortalTimeout(WIFI_PORTAL_TIMEOUT);
    wifiManager.setConnectTimeout(20);

    // Add custom fields to the portal: API URL + Vehicle ID
    WiFiManagerParameter paramApiUrl("api_url", "API Endpoint URL", cfgServerUrl, MAX_URL_LEN);
    WiFiManagerParameter paramVehicle("vehicle_id", "Vehicle / Bus ID", cfgVehicleId, MAX_VID_LEN);
    wifiManager.addParameter(&paramApiUrl);
    wifiManager.addParameter(&paramVehicle);
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    // Try auto-connect with saved credentials; on first boot opens portal
    Serial.println(F("[WIFI] Connecting to saved network..."));
    statusLedOn();   // LED on while connecting

    if (!wifiManager.autoConnect(WIFI_AP_NAME, WIFI_AP_PASS)) {
        Serial.println(F("[WIFI] Failed to connect — restarting"));
        delay(3000);
        ESP.restart();
    }

    statusLedOff();

    // Save custom params if they were changed in the portal
    if (shouldSaveConfig) {
        strncpy(cfgServerUrl, paramApiUrl.getValue(), MAX_URL_LEN - 1);
        strncpy(cfgVehicleId, paramVehicle.getValue(), MAX_VID_LEN - 1);
        saveConfig();
        Serial.println(F("[CFG] New settings saved to SPIFFS"));
    }

    Serial.print(F("[WIFI] Connected — IP: "));
    Serial.println(WiFi.localIP());

    // Quick double-blink to signal "ready"
    for (int i = 0; i < 2; i++) {
        flashLedOn();  delay(100);
        flashLedOff(); delay(100);
    }

    Serial.println(F("[READY] Starting capture loop (30 s interval)"));
    Serial.println();
}

// ═══════════════════════════════════════════════════════════════════════════
//  LOOP
// ═══════════════════════════════════════════════════════════════════════════

void loop() {
    unsigned long now = millis();

    // ── 1. Check BOOT button for WiFi portal request ────────────────────
    checkBootButton();

    if (portalRequested) {
        portalRequested = false;
        startConfigPortal();
    }

    // ── 2. Reconnect WiFi if dropped ────────────────────────────────────
    if (WiFi.status() != WL_CONNECTED && (now - lastWiFiCheckTime >= WIFI_RECONNECT_MS)) {
        lastWiFiCheckTime = now;
        Serial.println(F("[WIFI] Disconnected — attempting reconnect..."));
        WiFi.reconnect();
    }

    // ── 3. Capture & upload on interval ─────────────────────────────────
    if (now - lastCaptureTime >= CAPTURE_INTERVAL_MS) {
        lastCaptureTime = now;

        if (WiFi.status() == WL_CONNECTED) {
            captureAndUpload();
        } else {
            Serial.println(F("[SKIP] No WiFi — skipping capture"));
        }
    }

    // ── 4. Heartbeat on status LED (slow blink = idle, fast = no WiFi) ─
    static unsigned long hbTimer = 0;
    int hbInterval = (WiFi.status() == WL_CONNECTED) ? 2000 : 500;
    if (now - hbTimer >= (unsigned long)hbInterval) {
        hbTimer = now;
        static bool hbState = false;
        hbState = !hbState;
        hbState ? statusLedOn() : statusLedOff();
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  CAMERA INITIALISATION
// ═══════════════════════════════════════════════════════════════════════════

bool initCamera() {
    camera_config_t config;

    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer   = LEDC_TIMER_0;
    config.pin_d0       = Y2_GPIO_NUM;
    config.pin_d1       = Y3_GPIO_NUM;
    config.pin_d2       = Y4_GPIO_NUM;
    config.pin_d3       = Y5_GPIO_NUM;
    config.pin_d4       = Y6_GPIO_NUM;
    config.pin_d5       = Y7_GPIO_NUM;
    config.pin_d6       = Y8_GPIO_NUM;
    config.pin_d7       = Y9_GPIO_NUM;
    config.pin_xclk     = XCLK_GPIO_NUM;
    config.pin_pclk     = PCLK_GPIO_NUM;
    config.pin_vsync    = VSYNC_GPIO_NUM;
    config.pin_href     = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn     = PWDN_GPIO_NUM;
    config.pin_reset    = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.frame_size   = CAM_FRAME_SIZE;
    config.jpeg_quality = CAM_JPEG_QUALITY;
    config.fb_count     = CAM_FB_COUNT;
    config.grab_mode    = CAMERA_GRAB_LATEST;
    config.fb_location  = CAMERA_FB_IN_PSRAM;

    // Use PSRAM if available, else fall back to smaller frame
    if (!psramFound()) {
        Serial.println(F("[CAM] No PSRAM — falling back to QVGA"));
        config.frame_size  = FRAMESIZE_QVGA;
        config.jpeg_quality = 15;
        config.fb_location  = CAMERA_FB_IN_DRAM;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("[CAM] Init error 0x%x\n", err);
        return false;
    }

    // Warm-up: discard first few frames (auto-exposure settling)
    for (int i = 0; i < 3; i++) {
        camera_fb_t *fb = esp_camera_fb_get();
        if (fb) esp_camera_fb_return(fb);
        delay(100);
    }

    return true;
}

// ═══════════════════════════════════════════════════════════════════════════
//  CAPTURE & UPLOAD
// ═══════════════════════════════════════════════════════════════════════════

void captureAndUpload() {
    Serial.println(F("[CAP] Capturing image..."));

    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println(F("[CAP] Capture failed"));
        return;
    }

    Serial.printf("[CAP] Image captured — %u bytes\n", fb->len);

    bool ok = uploadImage(fb);
    esp_camera_fb_return(fb);

    if (ok) {
        Serial.println(F("[UPLOAD] Success"));
    } else {
        Serial.println(F("[UPLOAD] Failed"));
    }

    flashLedOff();   // Ensure LED is off after upload
    Serial.println();
}

// ═══════════════════════════════════════════════════════════════════════════
//  MULTIPART IMAGE UPLOAD  (POST to passenger.php)
// ═══════════════════════════════════════════════════════════════════════════

bool uploadImage(camera_fb_t *fb) {
    if (!fb || fb->len == 0) return false;

    Serial.println(F("[UPLOAD] Starting upload..."));

    HTTPClient http;
    http.begin(cfgServerUrl);
    http.setTimeout(HTTP_TIMEOUT_MS);

    // ── Build multipart body ────────────────────────────────────────────
    String boundary = "----SawariBoundary" + String(millis());
    String contentType = "multipart/form-data; boundary=" + boundary;
    http.addHeader("Content-Type", contentType);

    // Part 1: vehicle_id field
    String bodyStart = "--" + boundary + "\r\n"
                       "Content-Disposition: form-data; name=\"vehicle_id\"\r\n\r\n"
                       + String(cfgVehicleId) + "\r\n";

    // Part 2: image file
    String fileHeader = "--" + boundary + "\r\n"
                        "Content-Disposition: form-data; name=\"image\"; filename=\"capture.jpg\"\r\n"
                        "Content-Type: image/jpeg\r\n\r\n";

    String bodyEnd = "\r\n--" + boundary + "--\r\n";

    // Total content length
    uint32_t totalLen = bodyStart.length() + fileHeader.length() + fb->len + bodyEnd.length();

    // ── Allocate single buffer and assemble ─────────────────────────────
    uint8_t *payload = (uint8_t *)malloc(totalLen);
    if (!payload) {
        Serial.println(F("[UPLOAD] malloc failed — not enough memory"));
        http.end();
        return false;
    }

    uint32_t offset = 0;

    memcpy(payload + offset, bodyStart.c_str(), bodyStart.length());
    offset += bodyStart.length();

    memcpy(payload + offset, fileHeader.c_str(), fileHeader.length());
    offset += fileHeader.length();

    memcpy(payload + offset, fb->buf, fb->len);
    offset += fb->len;

    memcpy(payload + offset, bodyEnd.c_str(), bodyEnd.length());
    offset += bodyEnd.length();

    // ── Blink LED during transmission ───────────────────────────────────
    ledBlinkTimer = millis();
    ledBlinkState = false;

    // Start blinking (we'll blink in a tight pattern before POST)
    flashLedOn();

    // Use a custom stream to blink while sending — but HTTPClient is
    // blocking, so we blink before + after.  For visible feedback we
    // blink 3 times before and keep LED on during send.
    for (int i = 0; i < 3; i++) {
        flashLedOn();  delay(LED_BLINK_INTERVAL);
        flashLedOff(); delay(LED_BLINK_INTERVAL);
    }
    flashLedOn();   // Stay on during actual transfer

    int httpCode = http.POST(payload, totalLen);

    flashLedOff();  // Transfer done — LED off
    free(payload);

    // ── Blink result: 2 quick blinks = success, 5 slow blinks = error ──
    if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_CREATED) {
        String response = http.getString();
        Serial.printf("[UPLOAD] HTTP %d — Response: %s\n", httpCode, response.c_str());

        // Parse server response
        JsonDocument doc;
        DeserializationError jsonErr = deserializeJson(doc, response);
        if (!jsonErr && doc["success"].as<bool>()) {
            int passengers  = doc["passengers"].as<int>();
            const char *conf = doc["confidence"].as<const char *>();
            Serial.printf("[RESULT] Passengers: %d  Confidence: %s\n", passengers, conf ? conf : "N/A");
        }

        // Success blink: 2 quick
        for (int i = 0; i < 2; i++) {
            flashLedOn();  delay(80);
            flashLedOff(); delay(80);
        }

        http.end();
        return true;
    } else {
        Serial.printf("[UPLOAD] HTTP error: %d  %s\n", httpCode, http.errorToString(httpCode).c_str());

        // Error blink: 5 slow
        for (int i = 0; i < 5; i++) {
            flashLedOn();  delay(300);
            flashLedOff(); delay(300);
        }

        http.end();
        return false;
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  BOOT BUTTON — 3-SECOND HOLD TO OPEN WIFI PORTAL
// ═══════════════════════════════════════════════════════════════════════════

void checkBootButton() {
    bool pressed = (digitalRead(PIN_BOOT_BUTTON) == LOW);   // Active LOW

    if (pressed && !buttonHeld) {
        // Button just pressed — start timer
        buttonPressStart = millis();
        buttonHeld = true;
    }

    if (pressed && buttonHeld) {
        // Check if held long enough
        if ((millis() - buttonPressStart) >= BOOT_HOLD_MS) {
            Serial.println(F("[BTN] 3 s hold detected — opening WiFi portal"));
            portalRequested = true;
            buttonHeld = false;   // Reset so we don't re-trigger

            // Visual feedback: rapid blink while holding
            for (int i = 0; i < 6; i++) {
                flashLedOn();  delay(60);
                flashLedOff(); delay(60);
            }
        }
    }

    if (!pressed) {
        buttonHeld = false;       // Released before 3 s — reset
    }
}

// ═══════════════════════════════════════════════════════════════════════════
//  WIFI CONFIG PORTAL (on-demand)
// ═══════════════════════════════════════════════════════════════════════════

void startConfigPortal() {
    Serial.println(F("[WIFI] Starting config portal..."));
    Serial.println(F("[WIFI] Connect to AP: " WIFI_AP_NAME));
    Serial.println(F("[WIFI] Then open 192.168.4.1 in browser"));

    // Add custom fields with current values
    WiFiManagerParameter paramApiUrl("api_url", "API Endpoint URL", cfgServerUrl, MAX_URL_LEN);
    WiFiManagerParameter paramVehicle("vehicle_id", "Vehicle / Bus ID", cfgVehicleId, MAX_VID_LEN);

    wifiManager.addParameter(&paramApiUrl);
    wifiManager.addParameter(&paramVehicle);
    wifiManager.setSaveConfigCallback(saveConfigCallback);

    statusLedOn();   // Solid status LED while portal is active
    shouldSaveConfig = false;

    // startConfigPortal blocks until connected or timeout
    bool connected = wifiManager.startConfigPortal(WIFI_AP_NAME, WIFI_AP_PASS);

    if (connected) {
        Serial.print(F("[WIFI] New credentials saved — IP: "));
        Serial.println(WiFi.localIP());
    } else {
        Serial.println(F("[WIFI] Portal timed out — continuing with old config"));
    }

    // Save custom params if user submitted the form
    if (shouldSaveConfig) {
        strncpy(cfgServerUrl, paramApiUrl.getValue(), MAX_URL_LEN - 1);
        strncpy(cfgVehicleId, paramVehicle.getValue(), MAX_VID_LEN - 1);
        saveConfig();
        Serial.printf("[CFG] Updated — URL: %s  VID: %s\n", cfgServerUrl, cfgVehicleId);
    }

    statusLedOff();
}

// ═══════════════════════════════════════════════════════════════════════════
//  CONFIG PERSISTENCE (SPIFFS)
// ═══════════════════════════════════════════════════════════════════════════

void saveConfigCallback() {
    shouldSaveConfig = true;
}

void loadConfig() {
    // Start with defaults
    strncpy(cfgServerUrl, DEFAULT_SERVER_URL, MAX_URL_LEN - 1);
    strncpy(cfgVehicleId, DEFAULT_VEHICLE_ID, MAX_VID_LEN - 1);

    if (!SPIFFS.exists(CONFIG_FILE)) {
        Serial.println(F("[CFG] No saved config — using defaults"));
        return;
    }

    File file = SPIFFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println(F("[CFG] Failed to open config file"));
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, file);
    file.close();

    if (err) {
        Serial.printf("[CFG] JSON parse error: %s\n", err.c_str());
        return;
    }

    if (doc["api_url"].is<const char *>()) {
        strncpy(cfgServerUrl, doc["api_url"].as<const char *>(), MAX_URL_LEN - 1);
    }
    if (doc["vehicle_id"].is<const char *>()) {
        strncpy(cfgVehicleId, doc["vehicle_id"].as<const char *>(), MAX_VID_LEN - 1);
    }

    Serial.println(F("[CFG] Loaded saved config from SPIFFS"));
}

void saveConfig() {
    JsonDocument doc;
    doc["api_url"]    = cfgServerUrl;
    doc["vehicle_id"] = cfgVehicleId;

    File file = SPIFFS.open(CONFIG_FILE, "w");
    if (!file) {
        Serial.println(F("[CFG] Failed to create config file"));
        return;
    }

    serializeJson(doc, file);
    file.close();
    Serial.println(F("[CFG] Config saved to SPIFFS"));
}

// ═══════════════════════════════════════════════════════════════════════════
//  LED HELPERS
// ═══════════════════════════════════════════════════════════════════════════

void flashLedOn()  { digitalWrite(PIN_FLASH_LED,  HIGH); }
void flashLedOff() { digitalWrite(PIN_FLASH_LED,  LOW);  }
void statusLedOn() { digitalWrite(PIN_STATUS_LED, LOW);  }   // Active LOW
void statusLedOff(){ digitalWrite(PIN_STATUS_LED, HIGH); }   // Active LOW
