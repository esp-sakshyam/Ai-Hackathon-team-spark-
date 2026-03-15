/**
 * ============================================================================
 * SAWARI Bus Telemetry Device - Network Handler Implementation
 * ============================================================================
 *
 * WiFi connectivity management using WiFiManager library (tzapu/WiFiManager).
 *
 * Features:
 *   - Auto-connect to saved credentials on boot
 *   - On-demand captive portal via button press (non-blocking)
 *   - Custom portal parameters: API Endpoint URL and Bus ID
 *   - Portal-configured values saved to /config.json on LittleFS
 *   - Auto-close portal when WiFi connects
 *   - 10-second WiFi availability check interval
 *   - Offline mode fallback with automatic reconnection
 *   - HTTP POST telemetry with timeout handling
 * ============================================================================
 */

#include "network_handler.h"
#include "config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// --- WiFiManager instance (persistent for on-demand portal) ---
static WiFiManager _wm;

// --- State ---
static unsigned long _lastReconnectAttempt = 0;
static bool _wasConnected = false;
static bool _portalActive = false;

// --- Portal-configurable parameters (persisted to LittleFS) ---
static char _apiEndpoint[API_ENDPOINT_MAX_LEN] = "";
static char _busIdStr[8] = "";
static int  _busId = DEFAULT_BUS_ID;

// WiFiManager custom parameter objects
static WiFiManagerParameter* _paramApi   = nullptr;
static WiFiManagerParameter* _paramBusId = nullptr;

// ============================================================================
// Configuration persistence (LittleFS JSON)
// ============================================================================

/**
 * Load API endpoint and Bus ID from /config.json on LittleFS.
 * Falls back to compile-time defaults if file doesn't exist.
 */
static void _loadConfig() {
    // Start with defaults
    strncpy(_apiEndpoint, DEFAULT_API_ENDPOINT, sizeof(_apiEndpoint) - 1);
    _apiEndpoint[sizeof(_apiEndpoint) - 1] = '\0';
    _busId = DEFAULT_BUS_ID;
    snprintf(_busIdStr, sizeof(_busIdStr), "%d", _busId);

    if (!LittleFS.exists(CONFIG_FILE)) {
        Serial.println(F("[NETWORK] No config file found — using defaults"));
        return;
    }

    File f = LittleFS.open(CONFIG_FILE, "r");
    if (!f) {
        Serial.println(F("[NETWORK] Failed to open config file — using defaults"));
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) {
        Serial.print(F("[NETWORK] Config parse error: "));
        Serial.println(err.c_str());
        return;
    }

    if (doc["api_endpoint"].is<const char*>()) {
        strncpy(_apiEndpoint, doc["api_endpoint"].as<const char*>(), sizeof(_apiEndpoint) - 1);
        _apiEndpoint[sizeof(_apiEndpoint) - 1] = '\0';
    }
    if (doc["bus_id"].is<int>()) {
        _busId = doc["bus_id"].as<int>();
        snprintf(_busIdStr, sizeof(_busIdStr), "%d", _busId);
    }

    Serial.println(F("[NETWORK] Config loaded from LittleFS:"));
    Serial.print(F("[NETWORK]   API: ")); Serial.println(_apiEndpoint);
    Serial.print(F("[NETWORK]   Bus ID: ")); Serial.println(_busId);
}

/**
 * Save current API endpoint and Bus ID to /config.json on LittleFS.
 */
static void _saveConfig() {
    JsonDocument doc;
    doc["api_endpoint"] = _apiEndpoint;
    doc["bus_id"] = _busId;

    File f = LittleFS.open(CONFIG_FILE, "w");
    if (!f) {
        Serial.println(F("[NETWORK] ERROR: Failed to save config file"));
        return;
    }
    serializeJson(doc, f);
    f.close();

    Serial.println(F("[NETWORK] Config saved to LittleFS"));
}

/**
 * WiFiManager save-params callback.
 * Called when the user submits the portal form — reads custom fields and persists.
 */
static void _onSaveParams() {
    if (_paramApi) {
        strncpy(_apiEndpoint, _paramApi->getValue(), sizeof(_apiEndpoint) - 1);
        _apiEndpoint[sizeof(_apiEndpoint) - 1] = '\0';
    }
    if (_paramBusId) {
        strncpy(_busIdStr, _paramBusId->getValue(), sizeof(_busIdStr) - 1);
        _busIdStr[sizeof(_busIdStr) - 1] = '\0';
        _busId = atoi(_busIdStr);
        if (_busId <= 0) _busId = DEFAULT_BUS_ID;
        snprintf(_busIdStr, sizeof(_busIdStr), "%d", _busId);
    }

    Serial.println(F("[NETWORK] Portal parameters saved:"));
    Serial.print(F("[NETWORK]   API: ")); Serial.println(_apiEndpoint);
    Serial.print(F("[NETWORK]   Bus ID: ")); Serial.println(_busId);

    _saveConfig();
}

// ============================================================================
// WiFiManager setup helpers
// ============================================================================

/**
 * Create (or recreate) the custom parameter objects and attach to WiFiManager.
 */
static void _setupPortalParams() {
    // Clean up previous parameter objects if any
    delete _paramApi;
    delete _paramBusId;

    _paramApi   = new WiFiManagerParameter("api_url", "API Endpoint URL", _apiEndpoint, API_ENDPOINT_MAX_LEN);
    _paramBusId = new WiFiManagerParameter("bus_id",  "Bus ID",           _busIdStr,    7);

    _wm.addParameter(_paramApi);
    _wm.addParameter(_paramBusId);
    _wm.setSaveParamsCallback(_onSaveParams);
}

/**
 * Initialize WiFi using WiFiManager with captive portal support.
 * Loads saved config from LittleFS before connecting.
 * This call is BLOCKING during AP mode — it waits for the user to
 * configure WiFi via the captive portal, up to AP_TIMEOUT seconds.
 */
bool networkInit() {
    // Load persisted API / Bus ID config (or defaults)
    _loadConfig();

    _wm.setConfigPortalTimeout(AP_TIMEOUT);
    _wm.setConnectTimeout(15);
    _wm.setCleanConnect(true);

    // Add custom parameters to the portal form
    _setupPortalParams();

    Serial.println(F("[NETWORK] Starting WiFiManager..."));
    Serial.print(F("[NETWORK] AP Name: "));
    Serial.println(AP_NAME);

    bool connected = _wm.autoConnect(AP_NAME);

    if (connected) {
        Serial.println(F("[NETWORK] WiFi connected successfully"));
        Serial.print(F("[NETWORK] SSID: "));
        Serial.println(WiFi.SSID());
        Serial.print(F("[NETWORK] IP Address: "));
        Serial.println(WiFi.localIP());
        Serial.print(F("[NETWORK] RSSI: "));
        Serial.print(WiFi.RSSI());
        Serial.println(F(" dBm"));
        _wasConnected = true;
    } else {
        Serial.println(F("[NETWORK] WiFi connection failed / portal timed out"));
        Serial.println(F("[NETWORK] Operating in offline mode"));
    }

    return connected;
}

/**
 * Check if WiFi is currently connected.
 */
bool networkIsConnected() {
    return WiFi.status() == WL_CONNECTED;
}

/**
 * Periodically attempt WiFi reconnection if disconnected.
 * Called every WIFI_CHECK_INTERVAL (10 seconds).
 */
void networkCheckReconnect() {
    bool currentlyConnected = networkIsConnected();

    if (_wasConnected && !currentlyConnected) {
        Serial.println(F("[NETWORK] WiFi connection LOST — switching to offline mode"));
        _wasConnected = false;
    } else if (!_wasConnected && currentlyConnected) {
        Serial.println(F("[NETWORK] WiFi RECONNECTED"));
        Serial.print(F("[NETWORK] IP: "));
        Serial.println(WiFi.localIP());
        _wasConnected = true;
        return;
    }

    if (!currentlyConnected && !_portalActive) {
        unsigned long now = millis();
        if (now - _lastReconnectAttempt >= WIFI_RECONNECT_INTERVAL) {
            _lastReconnectAttempt = now;
            Serial.println(F("[NETWORK] Attempting WiFi reconnect..."));
            WiFi.disconnect();
            WiFi.reconnect();
        }
    }
}

/**
 * Start the WiFiManager captive portal on demand (non-blocking mode).
 * Called when the user presses the BOOT button.
 */
bool networkStartPortal() {
    if (_portalActive) {
        Serial.println(F("[NETWORK] Portal already active"));
        return true;
    }

    Serial.println(F("[NETWORK] Starting on-demand WiFi portal..."));
    Serial.print(F("[NETWORK] AP Name: "));
    Serial.println(AP_NAME);

    // Refresh custom parameter values before showing the portal
    _setupPortalParams();

    // Use non-blocking portal so GPS and other tasks continue running
    _wm.setConfigPortalBlocking(false);
    _wm.setConfigPortalTimeout(AP_TIMEOUT);

    _wm.startConfigPortal(AP_NAME);
    _portalActive = true;

    Serial.println(F("[NETWORK] Portal started at 192.168.4.1"));
    return true;
}

/**
 * Process portal requests — call in loop while portal is active.
 * Returns true if WiFi got connected through the portal.
 */
bool networkPortalLoop() {
    if (!_portalActive) return false;

    _wm.process();

    // Check if we got connected during portal
    if (networkIsConnected()) {
        Serial.println(F("[NETWORK] WiFi connected via portal!"));
        Serial.print(F("[NETWORK] SSID: "));
        Serial.println(WiFi.SSID());
        Serial.print(F("[NETWORK] IP: "));
        Serial.println(WiFi.localIP());
        _wasConnected = true;
        networkStopPortal();
        return true;
    }

    return false;
}

/**
 * Stop the captive portal.
 */
void networkStopPortal() {
    if (_portalActive) {
        _wm.stopConfigPortal();
        _portalActive = false;
        Serial.println(F("[NETWORK] WiFi portal stopped"));
    }
}

/**
 * Check if the captive portal is currently active.
 */
bool networkIsPortalActive() {
    return _portalActive;
}

/**
 * Send a JSON payload to the configured API endpoint via HTTP POST.
 * Uses the portal-configured API URL (saved in LittleFS).
 *
 * @param json  The JSON string to POST
 * @return true if server responded with HTTP 2xx
 */
bool networkSendData(const String& json) {
    if (!networkIsConnected()) {
        Serial.println(F("[NETWORK] Cannot send — WiFi not connected"));
        return false;
    }

    HTTPClient http;
    http.begin(_apiEndpoint);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(HTTP_TIMEOUT);

    Serial.print(F("[NETWORK] POST → "));
    Serial.println(_apiEndpoint);
    Serial.print(F("[NETWORK] Payload ("));
    Serial.print(json.length());
    Serial.println(F(" bytes)"));

    int httpCode = http.POST(json);

    if (httpCode > 0) {
        String responseBody = http.getString();

        if (httpCode >= 200 && httpCode < 300) {
            Serial.print(F("[NETWORK] ✓ POST success (HTTP "));
            Serial.print(httpCode);
            Serial.println(F(")"));
            if (responseBody.length() > 0) {
                Serial.print(F("[NETWORK] Response: "));
                Serial.println(responseBody.substring(0, 200));
            }
            http.end();
            return true;
        } else {
            Serial.print(F("[NETWORK] ✗ POST rejected (HTTP "));
            Serial.print(httpCode);
            Serial.println(F(")"));
            Serial.print(F("[NETWORK] Response: "));
            Serial.println(responseBody.substring(0, 200));
        }
    } else {
        Serial.print(F("[NETWORK] ✗ Connection error: "));
        Serial.println(http.errorToString(httpCode));

        switch (httpCode) {
            case HTTPC_ERROR_CONNECTION_REFUSED:
                Serial.println(F("[NETWORK]   → Server refused connection. Check URL/port."));
                break;
            case HTTPC_ERROR_SEND_HEADER_FAILED:
            case HTTPC_ERROR_SEND_PAYLOAD_FAILED:
                Serial.println(F("[NETWORK]   → Send failed. WiFi may have dropped."));
                break;
            case HTTPC_ERROR_CONNECTION_LOST:
                Serial.println(F("[NETWORK]   → Connection lost mid-transfer."));
                break;
            case HTTPC_ERROR_READ_TIMEOUT:
                Serial.println(F("[NETWORK]   → Server did not respond in time."));
                break;
            default:
                break;
        }
    }

    http.end();
    return false;
}

/**
 * Get the device's current IP address as a string.
 */
String networkGetIP() {
    if (networkIsConnected()) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

/**
 * Get the portal AP IP address.
 */
String networkGetPortalIP() {
    return "192.168.4.1";
}

/**
 * Get WiFi signal strength in dBm.
 */
int networkGetRSSI() {
    if (networkIsConnected()) {
        return WiFi.RSSI();
    }
    return -100;
}

/**
 * Get the SSID of the currently connected WiFi network.
 */
String networkGetSSID() {
    if (networkIsConnected()) {
        return WiFi.SSID();
    }
    return "";
}

/**
 * Get the currently configured API endpoint URL.
 */
const char* networkGetApiEndpoint() {
    return _apiEndpoint;
}

/**
 * Get the currently configured Bus ID.
 */
int networkGetBusId() {
    return _busId;
}
