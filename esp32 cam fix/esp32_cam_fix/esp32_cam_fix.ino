#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SD_MMC.h>
#include <Preferences.h>
#include <time.h>
#include "esp32-hal-cpu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

// OV3660 + AI Thinker ESP32-CAM pins
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define FLASH_LED_PIN 4

static const char *AP_SSID = "bus#1";
static const char *AP_PASSWORD = "";

static const IPAddress AP_IP(1, 2, 3, 4);
static const IPAddress AP_GATEWAY(1, 2, 3, 4);
static const IPAddress AP_SUBNET(255, 255, 255, 0);
IPAddress portalIP = AP_IP;

static const char *DEFAULT_API_URL = "https://example.com/api/upload";

WebServer server(80);
DNSServer dnsServer;
WiFiServer streamServer(81);
Preferences prefs;
SemaphoreHandle_t camMutex = nullptr;
SemaphoreHandle_t recMutex = nullptr;
TaskHandle_t streamTaskHandle = nullptr;
TaskHandle_t recordTaskHandle = nullptr;
TaskHandle_t uploadTaskHandle = nullptr;

String internetSsid;
String internetPassword;
String apiEndpoint;
String apiToken;

volatile bool liveModeEnabled = true;
volatile bool playbackActive = false;
volatile bool streamActive = false;
volatile bool recordingEnabled = false;
volatile bool uploadRequested = false;

bool sdReady = false;
bool internetConnected = false;
String lastUploadStatus = "Idle";
String lastRecordTime = "Never";

unsigned long lastUploadMs = 0;
unsigned long lastScanRetryMs = 0;
unsigned long lastFlickerMs = 0;
unsigned long lastSegmentStartMs = 0;
unsigned long lastRecordFrameMs = 0;

uint32_t segmentMs = 10000;
uint16_t recordFrameMs = 200; // ~5fps
uint8_t cleanupPercent = 90;
uint16_t minKeepSegments = 12;

uint64_t cachedSdTotalMB = 0;
uint64_t cachedSdUsedMB = 0;
unsigned long lastSdStatsMs = 0;

int flashDuty = 0;
int flickerDuty = 5;
int flickerIntervalMs = 25;
bool flickerState = false;

String activeVideoPath;
uint32_t activeVideoFrames = 0;
File activeVideoFile;

static const size_t DEBUG_RING_SIZE = 80;
String debugRing[DEBUG_RING_SIZE];
uint32_t debugSeq[DEBUG_RING_SIZE];
uint32_t debugWrite = 0;
uint32_t debugCounter = 0;

String escapeJson(const String &s) {
  String out;
  out.reserve(s.length() + 8);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    if (c == '"') out += "\\\"";
    else if (c == '\\') out += "\\\\";
    else if (c == '\n') out += "\\n";
    else if (c == '\r') out += "\\r";
    else out += c;
  }
  return out;
}

String nowString() {
  time_t now = time(nullptr);
  if (now < 100000) return "NoTime";
  struct tm t;
  localtime_r(&now, &t);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
  return String(buf);
}

void logDebug(const String &msg) {
  String line = "[" + nowString() + "] " + msg;
  Serial.println(line);
  debugRing[debugWrite] = line;
  debugSeq[debugWrite] = ++debugCounter;
  debugWrite = (debugWrite + 1) % DEBUG_RING_SIZE;

  if (sdReady) {
    File f = SD_MMC.open("/debug.log", FILE_APPEND);
    if (f) {
      f.println(line);
      f.close();
    }
  }
}

void setFlash(int duty) {
  duty = constrain(duty, 0, 255);
  flashDuty = duty;
  ledcWrite(FLASH_LED_PIN, duty);
}

camera_fb_t *grabFrame(uint32_t timeoutMs) {
  if (!camMutex) return nullptr;
  if (xSemaphoreTake(camMutex, pdMS_TO_TICKS(timeoutMs)) != pdTRUE) return nullptr;
  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) xSemaphoreGive(camMutex);
  return fb;
}

void returnFrame(camera_fb_t *fb) {
  if (fb) esp_camera_fb_return(fb);
  if (camMutex) xSemaphoreGive(camMutex);
}

bool initCamera() {
  camera_config_t c;
  c.ledc_channel = LEDC_CHANNEL_0;
  c.ledc_timer = LEDC_TIMER_0;
  c.pin_d0 = Y2_GPIO_NUM;
  c.pin_d1 = Y3_GPIO_NUM;
  c.pin_d2 = Y4_GPIO_NUM;
  c.pin_d3 = Y5_GPIO_NUM;
  c.pin_d4 = Y6_GPIO_NUM;
  c.pin_d5 = Y7_GPIO_NUM;
  c.pin_d6 = Y8_GPIO_NUM;
  c.pin_d7 = Y9_GPIO_NUM;
  c.pin_xclk = XCLK_GPIO_NUM;
  c.pin_pclk = PCLK_GPIO_NUM;
  c.pin_vsync = VSYNC_GPIO_NUM;
  c.pin_href = HREF_GPIO_NUM;
  c.pin_sccb_sda = SIOD_GPIO_NUM;
  c.pin_sccb_scl = SIOC_GPIO_NUM;
  c.pin_pwdn = PWDN_GPIO_NUM;
  c.pin_reset = RESET_GPIO_NUM;
  c.xclk_freq_hz = 20000000;
  c.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    c.frame_size = FRAMESIZE_VGA;
    c.jpeg_quality = 12;
    c.fb_count = 2;
    c.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    c.frame_size = FRAMESIZE_CIF;
    c.jpeg_quality = 15;
    c.fb_count = 1;
    c.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  }

  esp_err_t err = esp_camera_init(&c);
  if (err != ESP_OK) return false;

  sensor_t *s = esp_camera_sensor_get();
  if (s) s->set_vflip(s, 1);
  return true;
}

bool initSD() {
  if (!SD_MMC.begin("/sdcard", true)) return false;
  if (SD_MMC.cardType() == CARD_NONE) return false;
  return true;
}

void saveSettings() {
  prefs.begin("fix", false);
  prefs.putString("ssid", internetSsid);
  prefs.putString("pass", internetPassword);
  prefs.putString("api", apiEndpoint);
  prefs.putString("token", apiToken);
  prefs.putUInt("seg", segmentMs);
  prefs.putUInt("rf", recordFrameMs);
  prefs.putUInt("cln", cleanupPercent);
  prefs.putUInt("keep", minKeepSegments);
  prefs.end();
}

void loadSettings() {
  prefs.begin("fix", true);
  internetSsid = prefs.getString("ssid", "");
  internetPassword = prefs.getString("pass", "");
  apiEndpoint = prefs.getString("api", DEFAULT_API_URL);
  apiToken = prefs.getString("token", "");
  segmentMs = prefs.getUInt("seg", 10000);
  recordFrameMs = prefs.getUInt("rf", 200);
  cleanupPercent = (uint8_t)prefs.getUInt("cln", 90);
  minKeepSegments = (uint16_t)prefs.getUInt("keep", 12);
  prefs.end();

  segmentMs = constrain(segmentMs, 4000, 60000);
  recordFrameMs = constrain(recordFrameMs, 120, 1000);
  cleanupPercent = (uint8_t)constrain(cleanupPercent, 60, 98);
  minKeepSegments = (uint16_t)constrain(minKeepSegments, 2, 500);
}

void updateSdStatsCache() {
  if (!sdReady) {
    cachedSdTotalMB = 0;
    cachedSdUsedMB = 0;
    return;
  }
  unsigned long nowMs = millis();
  if (nowMs - lastSdStatsMs < 5000) return;
  lastSdStatsMs = nowMs;
  cachedSdTotalMB = SD_MMC.totalBytes() / (1024ULL * 1024ULL);
  cachedSdUsedMB = SD_MMC.usedBytes() / (1024ULL * 1024ULL);
}

bool connectInternet() {
  if (internetSsid.length() == 0) {
    internetConnected = false;
    return false;
  }

  int n = WiFi.scanNetworks(false, true, false, 250);
  bool found = false;
  for (int i = 0; i < n; i++) {
    if (WiFi.SSID(i) == internetSsid) {
      found = true;
      break;
    }
  }
  WiFi.scanDelete();
  if (!found) {
    internetConnected = false;
    return false;
  }

  WiFi.begin(internetSsid.c_str(), internetPassword.c_str());
  unsigned long st = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - st < 12000) {
    delay(200);
  }
  internetConnected = WiFi.status() == WL_CONNECTED;
  if (internetConnected) {
    configTzTime("+05:45", "time.google.com", "asia.pool.ntp.org", "pool.ntp.org");
  }
  return internetConnected;
}

String makeVideoFileName() {
  String t = nowString();
  t.replace(" ", "_");
  t.replace(":", "-");
  if (t == "NoTime") t = String(millis());
  return "/videos/rec_" + t + ".mjpeg";
}

bool ensureVideosDir() {
  if (!sdReady) return false;
  if (!SD_MMC.exists("/videos")) return SD_MMC.mkdir("/videos");
  return true;
}

bool startRecording() {
  if (!sdReady || !ensureVideosDir()) return false;
  if (recordingEnabled) return true;

  if (recMutex && xSemaphoreTake(recMutex, pdMS_TO_TICKS(200)) != pdTRUE) return false;
  activeVideoPath = makeVideoFileName();
  activeVideoFile = SD_MMC.open(activeVideoPath, FILE_WRITE);
  if (!activeVideoFile) {
    if (recMutex) xSemaphoreGive(recMutex);
    return false;
  }

  activeVideoFrames = 0;
  lastSegmentStartMs = millis();
  lastRecordFrameMs = millis();
  recordingEnabled = true;
  if (recMutex) xSemaphoreGive(recMutex);
  logDebug("Recording started: " + activeVideoPath);
  return true;
}

void stopRecording() {
  if (recMutex && xSemaphoreTake(recMutex, pdMS_TO_TICKS(250)) != pdTRUE) return;
  bool was = recordingEnabled;
  recordingEnabled = false;
  String oldPath = activeVideoPath;
  uint32_t oldFrames = activeVideoFrames;
  if (activeVideoFile) {
    activeVideoFile.flush();
    activeVideoFile.close();
  }
  activeVideoPath = "";
  activeVideoFrames = 0;
  if (recMutex) xSemaphoreGive(recMutex);

  if (was) {
    lastRecordTime = nowString();
    File lf = SD_MMC.open("/video_log.csv", FILE_APPEND);
    if (lf) {
      lf.println(lastRecordTime + "," + oldPath + "," + String(oldFrames));
      lf.close();
    }
    logDebug("Recording stopped: " + oldPath);
  }
}

void rotateSegment() {
  if (!recordingEnabled) return;
  if (recMutex && xSemaphoreTake(recMutex, pdMS_TO_TICKS(250)) != pdTRUE) return;

  String oldPath = activeVideoPath;
  uint32_t oldFrames = activeVideoFrames;

  if (activeVideoFile) {
    activeVideoFile.flush();
    activeVideoFile.close();
  }

  activeVideoPath = makeVideoFileName();
  activeVideoFile = SD_MMC.open(activeVideoPath, FILE_WRITE);
  activeVideoFrames = 0;
  lastSegmentStartMs = millis();

  if (recMutex) xSemaphoreGive(recMutex);

  File lf = SD_MMC.open("/video_log.csv", FILE_APPEND);
  if (lf) {
    lf.println(nowString() + "," + oldPath + "," + String(oldFrames));
    lf.close();
  }
}

bool shouldDelete(uint64_t used, uint64_t total, uint16_t count) {
  if (total == 0) return false;
  uint8_t pct = (uint8_t)((used * 100ULL) / total);
  if (pct < cleanupPercent) return false;
  return count > minKeepSegments;
}

void cleanupOldSegments() {
  if (!sdReady || playbackActive) return;
  static unsigned long lastRun = 0;
  if (millis() - lastRun < 15000) return;
  lastRun = millis();

  uint64_t total = SD_MMC.totalBytes();
  for (int k = 0; k < 3; k++) {
    File dir = SD_MMC.open("/videos");
    if (!dir || !dir.isDirectory()) return;

    String oldest = "";
    uint16_t count = 0;
    while (true) {
      File e = dir.openNextFile();
      if (!e) break;
      if (!e.isDirectory()) {
        String p = String(e.path());
        if (p.endsWith(".mjpeg")) {
          count++;
          if (p != activeVideoPath && (oldest.length() == 0 || p < oldest)) oldest = p;
        }
      }
      e.close();
    }
    dir.close();

    uint64_t used = SD_MMC.usedBytes();
    if (!shouldDelete(used, total, count)) return;
    if (oldest.length() == 0) return;
    SD_MMC.remove(oldest);
  }
}

bool uploadFrameToApi(camera_fb_t *fb, const String &name) {
  if (!internetConnected || apiEndpoint.length() == 0 || fb == nullptr) return false;

  HTTPClient http;
  WiFiClient plain;
  WiFiClientSecure secure;

  bool tls = apiEndpoint.startsWith("https://");
  if (tls) {
    secure.setInsecure();
    if (!http.begin(secure, apiEndpoint)) return false;
  } else {
    if (!http.begin(plain, apiEndpoint)) return false;
  }

  String boundary = "----ESP32FixBoundary";
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
  if (apiToken.length() > 0) http.addHeader("Authorization", "Bearer " + apiToken);

  String head = "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"file\"; filename=\"" + name + "\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";

  int totalLen = head.length() + fb->len + tail.length();
  uint8_t *body = (uint8_t *)malloc(totalLen);
  if (!body) {
    http.end();
    return false;
  }

  memcpy(body, head.c_str(), head.length());
  memcpy(body + head.length(), fb->buf, fb->len);
  memcpy(body + head.length() + fb->len, tail.c_str(), tail.length());

  int code = http.POST(body, totalLen);
  free(body);
  http.end();

  if (code >= 200 && code < 300) {
    lastUploadStatus = "OK(" + String(code) + ")";
    return true;
  }
  lastUploadStatus = "FAIL(" + String(code) + ")";
  return false;
}

void captureAndUpload() {
  camera_fb_t *fb = grabFrame(250);
  if (!fb) {
    lastUploadStatus = "Camera busy";
    return;
  }

  uint8_t *copy = (uint8_t *)malloc(fb->len);
  if (!copy) {
    returnFrame(fb);
    lastUploadStatus = "No heap";
    return;
  }
  memcpy(copy, fb->buf, fb->len);
  size_t len = fb->len;
  returnFrame(fb);

  camera_fb_t fbc = {};
  fbc.buf = copy;
  fbc.len = len;

  String name = "api_" + String(millis()) + ".jpg";
  if (internetConnected) uploadFrameToApi(&fbc, name);
  else lastUploadStatus = "No internet";

  free(copy);
}

void uploadTask(void *parameter) {
  while (true) {
    if (uploadRequested) {
      uploadRequested = false;
      captureAndUpload();
    }
    vTaskDelay(10);
  }
}

void recordTask(void *parameter) {
  while (true) {
    if (recordingEnabled) {
      unsigned long nowMs = millis();
      if (nowMs - lastSegmentStartMs >= segmentMs) {
        rotateSegment();
      }

      if (nowMs - lastRecordFrameMs >= recordFrameMs) {
        lastRecordFrameMs = nowMs;
        camera_fb_t *fb = grabFrame(120);
        if (fb) {
          if (!recMutex || xSemaphoreTake(recMutex, pdMS_TO_TICKS(40)) == pdTRUE) {
            if (recordingEnabled && activeVideoFile) {
              uint32_t l = (uint32_t)fb->len;
              activeVideoFile.write((uint8_t *)&l, sizeof(l));
              activeVideoFile.write(fb->buf, fb->len);
              activeVideoFrames++;
              if ((activeVideoFrames % 20) == 0) activeVideoFile.flush();
            }
            if (recMutex) xSemaphoreGive(recMutex);
          }
          returnFrame(fb);
        }
      }
    }
    vTaskDelay(8);
  }
}

void handleStreamClient() {
  if (!liveModeEnabled || playbackActive) {
    delay(4);
    return;
  }

  WiFiClient client = streamServer.available();
  if (!client) return;

  client.setNoDelay(true);
  streamActive = true;

  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: multipart/x-mixed-replace; boundary=frame\r\n");
  client.print("Cache-Control: no-store\r\n");
  client.print("Connection: close\r\n\r\n");

  while (client.connected()) {
    if (!liveModeEnabled || playbackActive) break;

    camera_fb_t *fb = grabFrame(60);
    if (!fb) {
      delay(1);
      continue;
    }

    client.print("--frame\r\n");
    client.print("Content-Type: image/jpeg\r\n");
    client.print("Content-Length: " + String(fb->len) + "\r\n\r\n");
    client.write(fb->buf, fb->len);
    client.print("\r\n");

    returnFrame(fb);
    delay(1);
  }

  client.stop();
  streamActive = false;
}

void streamTask(void *parameter) {
  while (true) {
    handleStreamClient();
    vTaskDelay(1);
  }
}

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <title>ESP32 CAM FIX</title>
  <style>
    body { margin: 0; font-family: Segoe UI, sans-serif; background: #0d1a27; color: #eef6ff; }
    .w { max-width: 1100px; margin: 0 auto; padding: 12px; }
    .card { background: #14283c; border: 1px solid #2b4763; border-radius: 12px; padding: 10px; margin-bottom: 10px; }
    .row { display: grid; grid-template-columns: 1fr 1fr; gap: 8px; }
    .feed { width: 100%; border-radius: 10px; min-height: 220px; background: #000; }
    input, button, select { width: 100%; padding: 8px; margin-top: 6px; border-radius: 8px; border: 1px solid #3f607f; background: #0e1f31; color: #fff; }
    button { background: #46a0ff; color: #03213f; font-weight: 700; }
    .muted { color: #a8c4df; font-size: 12px; }
    textarea { width: 100%; height: 150px; background: #09111a; color: #7cff9d; border: 1px solid #2b4763; border-radius: 8px; }
    @media (max-width: 900px) { .row { grid-template-columns: 1fr; } }
  </style>
</head>
<body>
<div class="w">
  <div class="card">
    <h3>Live (default)</h3>
    <img id="liveFeed" class="feed" alt="live"/>
    <div class="row">
      <button onclick="switchLive()">Live Mode</button>
      <button onclick="switchRecorded()">Recorded Mode</button>
    </div>
    <div class="row">
      <div>Mode: <span id="mode">live</span></div>
      <div>Upload: <span id="up">-</span></div>
      <div>SD Total: <span id="tot">0</span> MB</div>
      <div>SD Used: <span id="usd">0</span> MB</div>
    </div>
    <label>Light <span id="lv">0</span></label>
    <input id="light" type="range" min="0" max="255" value="0"/>
  </div>

  <div class="card">
    <h3>Recorded Playback</h3>
    <select id="records"></select>
    <div class="row">
      <button onclick="playSelected()">Play Selected</button>
      <button onclick="backToLive()">Back To Live</button>
    </div>
    <img id="recFeed" class="feed" alt="recorded"/>
  </div>

  <div class="card">
    <h3>Wi-Fi + API</h3>
    <button onclick="scanWifi()">Scan Networks</button>
    <select id="ssidList"></select>
    <input id="ssid" placeholder="SSID"/>
    <input id="pass" placeholder="Password" type="password"/>
    <input id="api" placeholder="API endpoint"/>
    <button onclick="saveCfg()">Save + Connect</button>
    <button onclick="sendNow()">Send API Photo Now</button>
  </div>

  <div class="card">
    <h3>Debug</h3>
    <textarea id="dbg" readonly></textarea>
  </div>
</div>

<script>
let cursor = 0;
let inPlayback = false;
let mode = 'live';
let lightTimer = null;
let lightPending = 0;
const live = document.getElementById('liveFeed');

const startLive = () => {
  if (inPlayback || mode !== 'live') return;
  live.src = 'http://' + location.hostname + ':81/stream?t=' + Date.now();
};

const stopLive = () => {
  live.src = '';
};

live.onerror = () => {
  if (!inPlayback && mode === 'live') setTimeout(startLive, 1200);
};

const switchLive = async () => {
  mode = 'live';
  inPlayback = false;
  await fetch('/api/mode/set?value=live');
  document.getElementById('recFeed').src = '';
  startLive();
};

const switchRecorded = async () => {
  mode = 'recorded';
  inPlayback = true;
  await fetch('/api/mode/set?value=recorded');
  stopLive();
};

const backToLive = () => switchLive();

const getStatus = async () => {
  const r = await fetch('/api/status');
  const j = await r.json();
  mode = j.mode || mode;
  document.getElementById('mode').textContent = mode;
  document.getElementById('up').textContent = j.lastUpload;
  document.getElementById('tot').textContent = j.sdTotalMB;
  document.getElementById('usd').textContent = j.sdUsedMB;
};

const pollDebug = async () => {
  const r = await fetch('/api/debug?from=' + cursor);
  const j = await r.json();
  if (j.lines && j.lines.length) {
    const box = document.getElementById('dbg');
    for (const ln of j.lines) box.value += ln + '\n';
    box.scrollTop = box.scrollHeight;
    cursor = j.next;
  }
};

const loadRecords = async () => {
  const r = await fetch('/api/records');
  const j = await r.json();
  const sel = document.getElementById('records');
  sel.innerHTML = '';
  for (const it of (j.items || [])) {
    const o = document.createElement('option');
    o.value = it.path;
    o.textContent = it.name;
    sel.appendChild(o);
  }
};

const playSelected = async () => {
  const sel = document.getElementById('records');
  if (!sel.value) return;
  await switchRecorded();
  document.getElementById('recFeed').src = '/media?f=' + encodeURIComponent(sel.value) + '&t=' + Date.now();
};

const scanWifi = async () => {
  const r = await fetch('/api/wifi-scan');
  const j = await r.json();
  const sel = document.getElementById('ssidList');
  sel.innerHTML = '';
  for (const it of (j.items || [])) {
    const o = document.createElement('option');
    o.value = it.ssid;
    o.textContent = it.ssid + ' (' + it.rssi + ' dBm)';
    sel.appendChild(o);
  }
  if ((j.items || []).length) document.getElementById('ssid').value = j.items[0].ssid;
};

document.getElementById('ssidList').addEventListener('change', e => {
  document.getElementById('ssid').value = e.target.value;
});

const saveCfg = async () => {
  const p = {
    ssid: document.getElementById('ssid').value,
    pass: document.getElementById('pass').value,
    api: document.getElementById('api').value,
    token: ''
  };
  const r = await fetch('/api/config', {
    method: 'POST',
    headers: {'Content-Type':'application/json'},
    body: JSON.stringify(p)
  });
  alert(await r.text());
};

const sendNow = async () => {
  const r = await fetch('/api/capture');
  alert(await r.text());
};

const setLight = async (v) => {
  document.getElementById('lv').textContent = v;
  await fetch('/api/light?value=' + encodeURIComponent(v));
};

document.getElementById('light').addEventListener('input', e => {
  lightPending = e.target.value;
  document.getElementById('lv').textContent = lightPending;
  if (lightTimer) return;
  lightTimer = setTimeout(async () => {
    lightTimer = null;
    await setLight(lightPending);
  }, 120);
});
document.getElementById('light').addEventListener('change', e => setLight(e.target.value));

setInterval(() => { getStatus().catch(()=>{}); pollDebug().catch(()=>{}); }, 3000);
setInterval(() => { loadRecords().catch(()=>{}); }, 30000);

switchLive().catch(()=>{});
getStatus();
pollDebug();
loadRecords();
</script>
</body>
</html>
)rawliteral";

String redirectUrl() { return String("http://") + portalIP.toString() + "/"; }

void handleRedirect() {
  server.sendHeader("Location", redirectUrl(), true);
  server.send(302, "text/plain", "Redirecting");
}

void handleStatus() {
  updateSdStatsCache();
  String json = "{";
  json += "\"mode\":\"" + String(liveModeEnabled ? "live" : "recorded") + "\",";
  json += "\"recording\":" + String(recordingEnabled ? "true" : "false") + ",";
  json += "\"lastUpload\":\"" + escapeJson(lastUploadStatus) + "\",";
  json += "\"sdTotalMB\":" + String((unsigned long)cachedSdTotalMB) + ",";
  json += "\"sdUsedMB\":" + String((unsigned long)cachedSdUsedMB);
  json += "}";
  server.send(200, "application/json", json);
}

void handleDebug() {
  uint32_t from = 0;
  if (server.hasArg("from")) from = (uint32_t)server.arg("from").toInt();
  String json = "{\"lines\":[";
  bool first = true;
  int sent = 0;
  for (size_t i = 0; i < DEBUG_RING_SIZE; i++) {
    if (debugSeq[i] == 0 || debugSeq[i] <= from) continue;
    if (!first) json += ",";
    first = false;
    json += "\"" + escapeJson(debugRing[i]) + "\"";
    sent++;
    if (sent >= 25) break;
  }
  json += "],\"next\":" + String(debugCounter) + "}";
  server.send(200, "application/json", json);
}

void handleLight() {
  int v = 0;
  if (server.hasArg("value")) v = server.arg("value").toInt();
  setFlash(v);
  server.send(200, "text/plain", "OK");
}

void handleModeGet() {
  String j = String("{\"mode\":\"") + (liveModeEnabled ? "live" : "recorded") + "\"}";
  server.send(200, "application/json", j);
}

void handleModeSet() {
  String v = server.arg("value");
  v.toLowerCase();
  if (v == "live") {
    liveModeEnabled = true;
    playbackActive = false;
    server.send(200, "text/plain", "Mode live");
    return;
  }
  if (v == "recorded") {
    liveModeEnabled = false;
    server.send(200, "text/plain", "Mode recorded");
    return;
  }
  server.send(400, "text/plain", "Use value=live or recorded");
}

void handleRecords() {
  if (!sdReady) {
    server.send(200, "application/json", "{\"items\":[],\"count\":0}");
    return;
  }
  File dir = SD_MMC.open("/videos");
  if (!dir || !dir.isDirectory()) {
    server.send(200, "application/json", "{\"items\":[],\"count\":0}");
    return;
  }

  String arr[60];
  int n = 0;
  while (true) {
    File e = dir.openNextFile();
    if (!e) break;
    if (!e.isDirectory()) {
      String p = String(e.path());
      if (p.endsWith(".mjpeg") && n < 60) arr[n++] = p;
    }
    e.close();
  }
  dir.close();

  for (int i = 0; i < n - 1; i++) {
    for (int j = i + 1; j < n; j++) {
      if (arr[i] < arr[j]) {
        String t = arr[i];
        arr[i] = arr[j];
        arr[j] = t;
      }
    }
  }

  String json = "{\"items\":[";
  for (int i = 0; i < n; i++) {
    if (i) json += ",";
    String nm = arr[i];
    int slash = nm.lastIndexOf('/');
    if (slash >= 0) nm = nm.substring(slash + 1);
    json += "{\"name\":\"" + escapeJson(nm) + "\",\"path\":\"" + escapeJson(arr[i]) + "\"}";
  }
  json += "],\"count\":" + String(n) + "}";
  server.send(200, "application/json", json);
}

void handleMedia() {
  if (!sdReady || !server.hasArg("f")) {
    server.send(404, "text/plain", "No media");
    return;
  }

  String p = server.arg("f");
  if (!p.startsWith("/videos/") || p.indexOf("..") >= 0 || !p.endsWith(".mjpeg")) {
    server.send(403, "text/plain", "Bad path");
    return;
  }

  WiFiClient c = server.client();
  File f = SD_MMC.open(p, FILE_READ);
  if (!f || f.isDirectory()) {
    if (f) f.close();
    server.send(404, "text/plain", "Not found");
    return;
  }

  playbackActive = true;
  liveModeEnabled = false;
  streamActive = false;

  c.print("HTTP/1.1 200 OK\r\n");
  c.print("Content-Type: multipart/x-mixed-replace; boundary=frame\r\n");
  c.print("Cache-Control: no-store\r\n");
  c.print("Connection: close\r\n\r\n");

  while (c.connected()) {
    uint32_t fl = 0;
    int rl = f.read((uint8_t *)&fl, sizeof(fl));
    if (rl != (int)sizeof(fl)) break;
    if (fl == 0 || fl > 350000) break;

    c.print("--frame\r\n");
    c.print("Content-Type: image/jpeg\r\n");
    c.print("Content-Length: " + String(fl) + "\r\n\r\n");

    uint8_t chunk[1460];
    uint32_t rem = fl;
    while (rem > 0) {
      size_t want = rem > sizeof(chunk) ? sizeof(chunk) : rem;
      int got = f.read(chunk, want);
      if (got <= 0) {
        rem = 0;
        break;
      }
      c.write(chunk, got);
      rem -= (uint32_t)got;
    }

    c.print("\r\n");
    delay(45);
  }

  f.close();
  playbackActive = false;
}

void handleRecordCtrl() {
  String a = server.arg("action");
  if (a == "start") {
    bool ok = startRecording();
    server.send(200, "text/plain", ok ? "Recording started" : "Recording failed");
    return;
  }
  if (a == "stop") {
    stopRecording();
    server.send(200, "text/plain", "Recording stopped");
    return;
  }
  server.send(400, "text/plain", "Use action=start|stop");
}

void handleWifiScan() {
  int n = WiFi.scanNetworks(false, true, false, 300);
  if (n <= 0) {
    server.send(200, "application/json", "{\"items\":[],\"count\":0}");
    return;
  }

  String json = "{\"items\":[";
  int em = 0;
  for (int i = 0; i < n && em < 25; i++) {
    String s = WiFi.SSID(i);
    if (s.length() == 0) continue;
    if (em) json += ",";
    json += "{\"ssid\":\"" + escapeJson(s) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    em++;
  }
  json += "],\"count\":" + String(em) + "}";
  WiFi.scanDelete();
  server.send(200, "application/json", json);
}

String jsonValue(const String &j, const String &k) {
  String t = "\"" + k + "\"";
  int kp = j.indexOf(t);
  if (kp < 0) return "";
  int c = j.indexOf(':', kp + t.length());
  if (c < 0) return "";
  int q1 = j.indexOf('"', c + 1);
  if (q1 < 0) return "";
  int q2 = j.indexOf('"', q1 + 1);
  if (q2 < 0) return "";
  return j.substring(q1 + 1, q2);
}

void handleConfig() {
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing JSON");
    return;
  }

  String body = server.arg("plain");
  String s = jsonValue(body, "ssid");
  String p = jsonValue(body, "pass");
  String api = jsonValue(body, "api");
  String tok = jsonValue(body, "token");

  if (s.length()) internetSsid = s;
  if (p.length() || body.indexOf("\"pass\":\"\"") >= 0) internetPassword = p;
  if (api.length()) apiEndpoint = api;
  if (tok.length() || body.indexOf("\"token\":\"\"") >= 0) apiToken = tok;

  saveSettings();
  bool ok = connectInternet();
  server.send(200, "text/plain", ok ? "Saved + connected" : "Saved (internet not connected)");
}

void handleCaptureNow() {
  uploadRequested = true;
  server.send(200, "text/plain", "Upload queued");
}

void setupRoutes() {
  server.on("/", HTTP_GET, []() { server.send_P(200, "text/html", INDEX_HTML); });
  server.on("/index.html", HTTP_GET, []() { server.send_P(200, "text/html", INDEX_HTML); });

  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/debug", HTTP_GET, handleDebug);
  server.on("/api/light", HTTP_GET, handleLight);
  server.on("/api/mode", HTTP_GET, handleModeGet);
  server.on("/api/mode/set", HTTP_GET, handleModeSet);
  server.on("/api/records", HTTP_GET, handleRecords);
  server.on("/api/record", HTTP_GET, handleRecordCtrl);
  server.on("/media", HTTP_GET, handleMedia);
  server.on("/api/wifi-scan", HTTP_GET, handleWifiScan);
  server.on("/api/config", HTTP_POST, handleConfig);
  server.on("/api/capture", HTTP_GET, handleCaptureNow);

  server.on("/generate_204", HTTP_GET, handleRedirect);
  server.on("/hotspot-detect.html", HTTP_GET, handleRedirect);
  server.on("/connecttest.txt", HTTP_GET, handleRedirect);
  server.on("/ncsi.txt", HTTP_GET, handleRedirect);
  server.on("/fwlink", HTTP_GET, handleRedirect);
  server.onNotFound(handleRedirect);
}

void setup() {
  Serial.begin(115200);
  delay(200);

  setCpuFrequencyMhz(240);

  camMutex = xSemaphoreCreateMutex();
  recMutex = xSemaphoreCreateMutex();

  ledcAttach(FLASH_LED_PIN, 5000, 8);
  setFlash(0);

  if (!initCamera()) {
    Serial.println("Camera init failed");
    delay(2000);
    ESP.restart();
  }

  sdReady = initSD();
  if (sdReady) logDebug("SD ready");

  loadSettings();

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);

  WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);
  bool apOk = (strlen(AP_PASSWORD) == 0) ? WiFi.softAP(AP_SSID) : WiFi.softAP(AP_SSID, AP_PASSWORD);
  if (!apOk) Serial.println("AP start failed");

  portalIP = WiFi.softAPIP();
  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", portalIP);

  setupRoutes();
  server.begin();
  streamServer.begin();

  xTaskCreatePinnedToCore(streamTask, "streamTask", 8192, nullptr, 1, &streamTaskHandle, 0);
  xTaskCreatePinnedToCore(recordTask, "recordTask", 8192, nullptr, 1, &recordTaskHandle, 1);
  xTaskCreatePinnedToCore(uploadTask, "uploadTask", 8192, nullptr, 1, &uploadTaskHandle, 1);

  connectInternet();
  if (sdReady) startRecording();

  logDebug("Portal ready on http://" + portalIP.toString() + "/");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  updateSdStatsCache();
  cleanupOldSegments();

  // Super dim, super fast flicker when no client connected.
  if (WiFi.softAPgetStationNum() == 0 && !streamActive && !playbackActive) {
    unsigned long nowMs = millis();
    if (nowMs - lastFlickerMs >= (unsigned long)flickerIntervalMs) {
      lastFlickerMs = nowMs;
      flickerState = !flickerState;
      ledcWrite(FLASH_LED_PIN, flickerState ? flickerDuty : 0);
    }
  } else {
    ledcWrite(FLASH_LED_PIN, flashDuty);
  }

  if (WiFi.status() != WL_CONNECTED) {
    internetConnected = false;
    if (millis() - lastScanRetryMs > 30000) {
      lastScanRetryMs = millis();
      connectInternet();
    }
  } else {
    internetConnected = true;
  }

  if (millis() - lastUploadMs >= 60000) {
    lastUploadMs = millis();
    uploadRequested = true;
  }
}
