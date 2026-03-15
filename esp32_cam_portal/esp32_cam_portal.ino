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
#define FLASH_LED_CHANNEL 7

static const char *AP_SSID = "bus#1";
static const char *AP_PASSWORD = ""; // unlocked AP

static const IPAddress AP_IP(1, 2, 3, 4);
static const IPAddress AP_GATEWAY(1, 2, 3, 4);
static const IPAddress AP_SUBNET(255, 255, 255, 0);
IPAddress portalIP = AP_IP;

static const char *DEFAULT_API_URL = "https://example.com/api/upload";
static const char *DEFAULT_GOOGLE_TIME_API_URL = "https://example.com/google-time-kathmandu";

WebServer server(80);
DNSServer dnsServer;
WiFiServer streamServer(81);
Preferences prefs;
TaskHandle_t streamTaskHandle = nullptr;
TaskHandle_t recorderTaskHandle = nullptr;
TaskHandle_t uploadTaskHandle = nullptr;
volatile bool streamActive = false;
SemaphoreHandle_t cameraMutex = nullptr;
SemaphoreHandle_t recordMutex = nullptr;

String internetSsid;
String internetPassword;
String apiEndpoint;
String googleTimeApiUrl;
String apiBearerToken;

String lastRecordTime = "Never";
String lastUploadStatus = "Idle";
bool sdReady = false;
bool internetConnected = false;

unsigned long lastUploadMs = 0;
volatile bool uploadRequested = false;
unsigned long lastStatusLogMs = 0;
unsigned long lastPortalActivityMs = 0;
unsigned long lastLightLogMs = 0;
unsigned long lastRecordFrameMs = 0;

volatile bool recordingEnabled = false;
volatile bool playbackActive = false;
volatile bool liveModeEnabled = true;
String activeVideoPath;
uint32_t activeVideoFrames = 0;
File activeVideoFile;
const uint16_t VIDEO_RECORD_INTERVAL_MS = 200; // ~5 FPS MJPEG recording
const uint32_t DEFAULT_VIDEO_SEGMENT_MS = 10000;
unsigned long recordSegmentStartMs = 0;
const uint8_t DEFAULT_SD_USAGE_LIMIT_PERCENT = 90;
const uint16_t DEFAULT_MIN_VIDEO_SEGMENTS_TO_KEEP = 12;
const uint32_t STORAGE_CLEANUP_INTERVAL_MS = 15000;
unsigned long lastStorageCleanupMs = 0;
const uint32_t LOG_MAINTENANCE_INTERVAL_MS = 60000;
unsigned long lastLogMaintenanceMs = 0;
unsigned long lastSdStatsMs = 0;
uint64_t cachedSdTotalMB = 0;
uint64_t cachedSdUsedMB = 0;

uint32_t videoSegmentMs = DEFAULT_VIDEO_SEGMENT_MS;
uint8_t sdUsageLimitPercent = DEFAULT_SD_USAGE_LIMIT_PERCENT;
uint16_t minVideoSegmentsToKeep = DEFAULT_MIN_VIDEO_SEGMENTS_TO_KEEP;

int userLightDuty = 0;
int flickerDuty = 6;      // very dim
int flickerIntervalMs = 25; // super fast
unsigned long lastFlickerMs = 0;
bool flickerState = false;

static const size_t DEBUG_RING_SIZE = 120;
String debugRing[DEBUG_RING_SIZE];
uint32_t debugSeq[DEBUG_RING_SIZE];
uint32_t debugWriteIndex = 0;
uint32_t debugCounter = 0;

void updateCachedSdStats() {
  if (!sdReady) {
    cachedSdTotalMB = 0;
    cachedSdUsedMB = 0;
    return;
  }

  unsigned long nowMs = millis();
  if (nowMs - lastSdStatsMs < 5000) {
    return;
  }
  lastSdStatsMs = nowMs;
  cachedSdTotalMB = SD_MMC.totalBytes() / (1024ULL * 1024ULL);
  cachedSdUsedMB = SD_MMC.usedBytes() / (1024ULL * 1024ULL);
}

void requestUploadCycle() {
  uploadRequested = true;
}

String escapeJson(const String &s) {
  String out;
  out.reserve(s.length() + 16);
  for (size_t i = 0; i < s.length(); i++) {
    char c = s[i];
    switch (c) {
    case '"':
      out += "\\\"";
      break;
    case '\\':
      out += "\\\\";
      break;
    case '\n':
      out += "\\n";
      break;
    case '\r':
      out += "\\r";
      break;
    case '\t':
      out += "\\t";
      break;
    default:
      out += c;
      break;
    }
  }
  return out;
}

String nowString();

void touchPortalActivity() { lastPortalActivityMs = millis(); }

camera_fb_t *cameraGrabFrame(uint32_t timeoutMs) {
  if (cameraMutex) {
    if (xSemaphoreTake(cameraMutex, pdMS_TO_TICKS(timeoutMs)) != pdTRUE) {
      return nullptr;
    }
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb && cameraMutex) {
    xSemaphoreGive(cameraMutex);
  }
  return fb;
}

void cameraReturnFrame(camera_fb_t *fb) {
  if (fb) {
    esp_camera_fb_return(fb);
  }
  if (cameraMutex) {
    xSemaphoreGive(cameraMutex);
  }
}

void logDebug(const String &msg) {
  String line = "[" + nowString() + "] " + msg;
  Serial.println(line);

  debugRing[debugWriteIndex] = line;
  debugSeq[debugWriteIndex] = ++debugCounter;
  debugWriteIndex = (debugWriteIndex + 1) % DEBUG_RING_SIZE;

  if (sdReady) {
    File f = SD_MMC.open("/debug.log", FILE_APPEND);
    if (f) {
      f.println(line);
      f.close();
    }
  }
}

void trimLogFileIfLarge(const char *path, size_t maxSize, size_t keepTailSize) {
  (void)keepTailSize;
  if (!sdReady) {
    return;
  }

  File r = SD_MMC.open(path, FILE_READ);
  if (!r || r.isDirectory()) {
    if (r) {
      r.close();
    }
    return;
  }

  size_t sz = r.size();
  if (sz <= maxSize) {
    r.close();
    return;
  }
  r.close();

  // Low-memory rollover: delete and recreate tiny header.
  SD_MMC.remove(path);
  File w = SD_MMC.open(path, FILE_WRITE);
  if (w) {
    w.println("[log rotated to protect RAM/storage]");
    w.close();
  }
}

void maintainSdLogs() {
  if (!sdReady) {
    return;
  }

  unsigned long nowMs = millis();
  if (nowMs - lastLogMaintenanceMs < LOG_MAINTENANCE_INTERVAL_MS) {
    return;
  }
  lastLogMaintenanceMs = nowMs;

  trimLogFileIfLarge("/debug.log", 2 * 1024 * 1024, 64 * 1024);
  trimLogFileIfLarge("/video_log.csv", 256 * 1024, 48 * 1024);
}

String nowString() {
  time_t now = time(nullptr);
  if (now < 100000) {
    return "NoTime";
  }
  struct tm t;
  localtime_r(&now, &t);
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
  return String(buf);
}

bool initCamera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    // Balanced high resolution with smooth FPS for OV3660.
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 3;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 15;
    config.fb_count = 1;
    config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 0);
    s->set_saturation(s, 0);
  }

  return true;
}

bool initSD() {
  if (!SD_MMC.begin("/sdcard", true)) {
    return false;
  }
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    return false;
  }
  return true;
}

void setFlashDuty(int duty) {
  duty = constrain(duty, 0, 255);
  ledcWrite(FLASH_LED_PIN, duty);
}

void saveSettings() {
  prefs.begin("portal", false);
  prefs.putString("ssid", internetSsid);
  prefs.putString("pass", internetPassword);
  prefs.putString("api", apiEndpoint);
  prefs.putString("timeapi", googleTimeApiUrl);
  prefs.putString("token", apiBearerToken);
  prefs.putUInt("segms", videoSegmentMs);
  prefs.putUInt("sduse", sdUsageLimitPercent);
  prefs.putUInt("minkeep", minVideoSegmentsToKeep);
  prefs.end();
}

void loadSettings() {
  prefs.begin("portal", true);
  internetSsid = prefs.getString("ssid", "");
  internetPassword = prefs.getString("pass", "");
  apiEndpoint = prefs.getString("api", DEFAULT_API_URL);
  googleTimeApiUrl = prefs.getString("timeapi", DEFAULT_GOOGLE_TIME_API_URL);
  apiBearerToken = prefs.getString("token", "");
  videoSegmentMs = prefs.getUInt("segms", DEFAULT_VIDEO_SEGMENT_MS);
  sdUsageLimitPercent = (uint8_t)prefs.getUInt("sduse", DEFAULT_SD_USAGE_LIMIT_PERCENT);
  minVideoSegmentsToKeep = (uint16_t)prefs.getUInt("minkeep", DEFAULT_MIN_VIDEO_SEGMENTS_TO_KEEP);
  prefs.end();

  videoSegmentMs = constrain(videoSegmentMs, 4000, 60000);
  sdUsageLimitPercent = (uint8_t)constrain(sdUsageLimitPercent, 60, 98);
  minVideoSegmentsToKeep = (uint16_t)constrain(minVideoSegmentsToKeep, 2, 500);
}

bool syncTimeFromGoogleApi() {
  if (googleTimeApiUrl.length() == 0 || googleTimeApiUrl.indexOf("example.com") >= 0) {
    logDebug("Google time API placeholder still configured; skipping direct HTTP sync.");
    return false;
  }

  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;

  if (!http.begin(client, googleTimeApiUrl)) {
    logDebug("Failed to begin Google time API request.");
    return false;
  }

  int code = http.GET();
  if (code != 200) {
    logDebug("Google time API HTTP code: " + String(code));
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  // Expected payload should include: {"unixtime": 1710000000}
  int keyIndex = payload.indexOf("unixtime");
  if (keyIndex < 0) {
    logDebug("Google time API payload missing unixtime.");
    return false;
  }

  int colon = payload.indexOf(':', keyIndex);
  if (colon < 0) {
    return false;
  }

  int endPos = colon + 1;
  while (endPos < (int)payload.length() && isspace((unsigned char)payload[endPos])) {
    endPos++;
  }

  String num;
  while (endPos < (int)payload.length() && isdigit((unsigned char)payload[endPos])) {
    num += payload[endPos++];
  }

  if (num.length() == 0) {
    return false;
  }

  time_t ts = (time_t)num.toInt();
  struct timeval tv = {.tv_sec = ts, .tv_usec = 0};
  settimeofday(&tv, nullptr);
  logDebug("Time synced from Google time API.");
  return true;
}

void syncTimeNepal() {
  // Public NTP servers suitable for Nepal local time display.
  configTzTime("+05:45", "time.google.com", "asia.pool.ntp.org", "pool.ntp.org");
  delay(1200);

  if (internetConnected) {
    bool ok = syncTimeFromGoogleApi();
    if (!ok) {
      logDebug("Using NTP time source for Kathmandu timezone.");
    }
  }
}

bool connectToInternet() {
  if (internetSsid.length() == 0) {
    logDebug("No internet SSID configured yet.");
    internetConnected = false;
    return false;
  }

  logDebug("Scanning for internet APs...");
  int count = WiFi.scanNetworks();
  if (count <= 0) {
    logDebug("No APs found in scan.");
    internetConnected = false;
    return false;
  }

  bool found = false;
  for (int i = 0; i < count; i++) {
    if (WiFi.SSID(i) == internetSsid) {
      found = true;
      break;
    }
  }

  if (!found) {
    logDebug("Configured internet AP not in range: " + internetSsid);
    internetConnected = false;
    return false;
  }

  logDebug("Trying STA connection to " + internetSsid + "...");
  WiFi.begin(internetSsid.c_str(), internetPassword.c_str());
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 18000) {
    delay(250);
  }

  internetConnected = WiFi.status() == WL_CONNECTED;
  if (internetConnected) {
    logDebug("STA connected. IP: " + WiFi.localIP().toString());
    syncTimeNepal();
  } else {
    logDebug("STA connection failed.");
  }
  return internetConnected;
}

String makeApiPhotoName() {
  String t = nowString();
  t.replace(" ", "_");
  t.replace(":", "-");
  if (t == "NoTime") {
    t = String(millis());
  }
  return "api_" + t + ".jpg";
}

bool ensureVideoDir() {
  if (!sdReady) {
    return false;
  }
  if (!SD_MMC.exists("/videos")) {
    return SD_MMC.mkdir("/videos");
  }
  return true;
}

String makeVideoFilename() {
  String t = nowString();
  t.replace(" ", "_");
  t.replace(":", "-");
  if (t == "NoTime") {
    t = String(millis());
  }
  return "/videos/rec_" + t + ".mjpeg";
}

bool shouldDeleteForStorage(uint64_t usedBytes, uint64_t totalBytes, uint16_t segmentCount) {
  if (totalBytes == 0) {
    return false;
  }
  uint8_t usagePercent = (uint8_t)((usedBytes * 100ULL) / totalBytes);
  if (usagePercent < sdUsageLimitPercent) {
    return false;
  }
  return segmentCount > minVideoSegmentsToKeep;
}

bool findOldestDeletableSegment(String &oldestPath, uint16_t &segmentCount) {
  oldestPath = "";
  segmentCount = 0;

  if (!sdReady) {
    return false;
  }

  File dir = SD_MMC.open("/videos");
  if (!dir || !dir.isDirectory()) {
    return false;
  }

  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }

    if (!entry.isDirectory()) {
      String p = String(entry.path());
      if (p.endsWith(".mjpeg")) {
        segmentCount++;
        if (p != activeVideoPath) {
          if (oldestPath.length() == 0 || p < oldestPath) {
            oldestPath = p;
          }
        }
      }
    }
    entry.close();
  }
  dir.close();
  return oldestPath.length() > 0;
}

void cleanupOldVideoSegmentsIfNeeded() {
  if (!sdReady || playbackActive) {
    return;
  }

  unsigned long nowMs = millis();
  if (nowMs - lastStorageCleanupMs < STORAGE_CLEANUP_INTERVAL_MS) {
    return;
  }
  lastStorageCleanupMs = nowMs;

  uint64_t total = SD_MMC.totalBytes();
  if (total == 0) {
    return;
  }

  for (int i = 0; i < 3; i++) {
    uint16_t segmentCount = 0;
    String oldestPath;
    bool hasCandidate = findOldestDeletableSegment(oldestPath, segmentCount);
    uint64_t used = SD_MMC.usedBytes();

    if (!shouldDeleteForStorage(used, total, segmentCount)) {
      return;
    }
    if (!hasCandidate) {
      return;
    }

    if (SD_MMC.remove(oldestPath)) {
      logDebug("Storage cleanup removed old segment: " + oldestPath);
    } else {
      logDebug("Storage cleanup failed to remove: " + oldestPath);
      return;
    }
  }
}

bool startVideoRecording() {
  if (!sdReady) {
    logDebug("Cannot start recording: SD not ready.");
    return false;
  }
  if (recordingEnabled) {
    return true;
  }
  if (!ensureVideoDir()) {
    logDebug("Cannot start recording: failed to create /videos.");
    return false;
  }

  if (recordMutex && xSemaphoreTake(recordMutex, pdMS_TO_TICKS(200)) != pdTRUE) {
    return false;
  }

  activeVideoPath = makeVideoFilename();
  activeVideoFile = SD_MMC.open(activeVideoPath, FILE_WRITE);
  if (!activeVideoFile) {
    if (recordMutex) {
      xSemaphoreGive(recordMutex);
    }
    logDebug("Cannot start recording: failed to open " + activeVideoPath);
    return false;
  }

  activeVideoFrames = 0;
  lastRecordFrameMs = millis();
  recordSegmentStartMs = millis();
  recordingEnabled = true;
  if (recordMutex) {
    xSemaphoreGive(recordMutex);
  }
  logDebug("Video recording started: " + activeVideoPath);
  return true;
}

bool rotateVideoRecordingSegment() {
  if (!recordingEnabled || !sdReady) {
    return false;
  }
  if (!recordMutex || xSemaphoreTake(recordMutex, pdMS_TO_TICKS(250)) != pdTRUE) {
    return false;
  }

  String oldPath = activeVideoPath;
  uint32_t oldFrames = activeVideoFrames;

  if (activeVideoFile) {
    activeVideoFile.flush();
    activeVideoFile.close();
  }

  String newPath = makeVideoFilename();
  File newFile = SD_MMC.open(newPath, FILE_WRITE);
  if (!newFile) {
    if (oldPath.length() > 0) {
      activeVideoPath = oldPath;
      activeVideoFile = SD_MMC.open(activeVideoPath, FILE_APPEND);
    }
    if (recordMutex) {
      xSemaphoreGive(recordMutex);
    }
    logDebug("Segment rotate failed: could not open new segment file.");
    return false;
  }

  activeVideoFile = newFile;
  activeVideoPath = newPath;
  activeVideoFrames = 0;
  recordSegmentStartMs = millis();

  if (recordMutex) {
    xSemaphoreGive(recordMutex);
  }

  File logFile = SD_MMC.open("/video_log.csv", FILE_APPEND);
  if (logFile && oldPath.length() > 0) {
    logFile.println(nowString() + "," + oldPath + "," + String(oldFrames));
    logFile.close();
  }
  logDebug("Recording rotated to new 10s segment: " + activeVideoPath);
  return true;
}

void stopVideoRecording() {
  if (recordMutex && xSemaphoreTake(recordMutex, pdMS_TO_TICKS(250)) != pdTRUE) {
    return;
  }

  bool wasRecording = recordingEnabled;
  recordingEnabled = false;

  String finalPath = activeVideoPath;
  uint32_t totalFrames = activeVideoFrames;

  if (activeVideoFile) {
    activeVideoFile.flush();
    activeVideoFile.close();
  }
  activeVideoFrames = 0;
  activeVideoPath = "";
  if (recordMutex) {
    xSemaphoreGive(recordMutex);
  }

  if (wasRecording) {
    lastRecordTime = nowString();
    File logFile = SD_MMC.open("/video_log.csv", FILE_APPEND);
    if (logFile) {
      logFile.println(lastRecordTime + "," + finalPath + "," + String(totalFrames));
      logFile.close();
    }
    logDebug("Video recording stopped. Frames=" + String(totalFrames) + " file=" + finalPath);
  }
}

void recorderTask(void *parameter) {
  while (true) {
    if (recordingEnabled) {
      unsigned long nowMs = millis();

      if (nowMs - recordSegmentStartMs >= videoSegmentMs) {
        rotateVideoRecordingSegment();
      }

      if (nowMs - lastRecordFrameMs >= VIDEO_RECORD_INTERVAL_MS) {
        lastRecordFrameMs = nowMs;

        camera_fb_t *fb = cameraGrabFrame(120);
        if (fb) {
          if (!recordMutex || xSemaphoreTake(recordMutex, pdMS_TO_TICKS(40)) == pdTRUE) {
            if (recordingEnabled && activeVideoFile) {
              uint32_t frameLen = (uint32_t)fb->len;
              activeVideoFile.write((uint8_t *)&frameLen, sizeof(frameLen));
              activeVideoFile.write(fb->buf, fb->len);
              activeVideoFrames++;
              if ((activeVideoFrames % 25) == 0) {
                activeVideoFile.flush();
              }
            }
            if (recordMutex) {
              xSemaphoreGive(recordMutex);
            }
          }
          cameraReturnFrame(fb);
        }
      }
    }
    vTaskDelay(8);
  }
}

bool uploadFrameToApi(camera_fb_t *fb, const String &filename) {
  if (!internetConnected || apiEndpoint.length() == 0 || fb == nullptr) {
    return false;
  }

  HTTPClient http;
  WiFiClient plainClient;
  WiFiClientSecure secureClient;

  bool useTls = apiEndpoint.startsWith("https://");
  if (useTls) {
    secureClient.setInsecure();
    if (!http.begin(secureClient, apiEndpoint)) {
      logDebug("API upload begin failed.");
      return false;
    }
  } else {
    if (!http.begin(plainClient, apiEndpoint)) {
      logDebug("API upload begin failed.");
      return false;
    }
  }

  String boundary = "----ESP32CamBoundary7MA4YWxkTrZu0gW";
  http.addHeader("Content-Type", "multipart/form-data; boundary=" + boundary);
  if (apiBearerToken.length() > 0) {
    http.addHeader("Authorization", "Bearer " + apiBearerToken);
  }

  String head = "--" + boundary + "\r\n";
  head += "Content-Disposition: form-data; name=\"file\"; filename=\"" + filename + "\"\r\n";
  head += "Content-Type: image/jpeg\r\n\r\n";
  String tail = "\r\n--" + boundary + "--\r\n";

  int bodyLen = head.length() + fb->len + tail.length();
  uint8_t *body = (uint8_t *)malloc(bodyLen);
  if (!body) {
    http.end();
    logDebug("Not enough heap for multipart body.");
    return false;
  }

  memcpy(body, head.c_str(), head.length());
  memcpy(body + head.length(), fb->buf, fb->len);
  memcpy(body + head.length() + fb->len, tail.c_str(), tail.length());

  int code = http.POST(body, bodyLen);
  free(body);

  String response = http.getString();
  http.end();

  if (code >= 200 && code < 300) {
    lastUploadStatus = "OK (" + String(code) + ")";
    logDebug("API upload success code " + String(code));
    return true;
  }

  lastUploadStatus = "FAIL (" + String(code) + ")";
  logDebug("API upload failed. code=" + String(code) + " body=" + response);
  return false;
}

void captureAndUploadCycle() {
  camera_fb_t *fb = cameraGrabFrame(250);
  if (!fb) {
    lastUploadStatus = "Camera busy";
    logDebug("Camera busy, skipped this capture cycle.");
    return;
  }

  uint8_t *copied = (uint8_t *)malloc(fb->len);
  size_t copiedLen = fb->len;
  if (!copied) {
    cameraReturnFrame(fb);
    lastUploadStatus = "Capture copy failed";
    logDebug("Not enough heap to copy frame for concurrent stream+record.");
    return;
  }
  memcpy(copied, fb->buf, fb->len);
  cameraReturnFrame(fb);

  camera_fb_t frameCopy = {};
  frameCopy.buf = copied;
  frameCopy.len = copiedLen;
  String fileNameForApi = makeApiPhotoName();

  if (internetConnected) {
    uploadFrameToApi(&frameCopy, fileNameForApi);
  } else {
    lastUploadStatus = "Skipped: no internet";
  }

  free(copied);
}

void uploadTask(void *parameter) {
  while (true) {
    if (uploadRequested) {
      uploadRequested = false;
      captureAndUploadCycle();
    }
    vTaskDelay(10);
  }
}

uint64_t getCardTotal() {
  if (!sdReady) {
    return 0;
  }
  return SD_MMC.totalBytes();
}

uint64_t getCardUsed() {
  if (!sdReady) {
    return 0;
  }
  return SD_MMC.usedBytes();
}

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html>
<head>
  <meta charset="utf-8"/>
  <meta name="viewport" content="width=device-width,initial-scale=1"/>
  <title>bus#1 Camera Portal</title>
  <style>
    :root {
      --bgA: #0d1117;
      --bgB: #152232;
      --card: rgba(255,255,255,.08);
      --border: rgba(255,255,255,.18);
      --text: #f7fbff;
      --muted: #b6c7da;
      --accent: #ffb703;
      --accent2: #69d2e7;
      --ok: #52d273;
      --err: #ff6b6b;
      --mono: "Consolas", "SFMono-Regular", monospace;
      --ui: "Segoe UI", "Trebuchet MS", sans-serif;
    }
    * { box-sizing: border-box; }
    body {
      margin: 0;
      font-family: var(--ui);
      color: var(--text);
      background:
        radial-gradient(1200px 500px at -10% -20%, #24557d66, transparent),
        radial-gradient(900px 600px at 110% -10%, #3f1d4d66, transparent),
        linear-gradient(135deg, var(--bgA), var(--bgB));
      min-height: 100vh;
      animation: fadeIn .6s ease;
    }
    @keyframes fadeIn {
      from { opacity: 0; transform: translateY(8px); }
      to { opacity: 1; transform: translateY(0); }
    }
    .wrap {
      max-width: 1200px;
      margin: 0 auto;
      padding: 18px;
      display: grid;
      gap: 14px;
    }
    .top {
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 12px;
      border: 1px solid var(--border);
      background: var(--card);
      border-radius: 18px;
      padding: 14px 16px;
      backdrop-filter: blur(8px);
    }
    .title {
      font-size: 1.35rem;
      letter-spacing: .4px;
      font-weight: 700;
    }
    .pill {
      background: #ffffff14;
      border: 1px solid var(--border);
      border-radius: 999px;
      padding: 6px 10px;
      font-size: .86rem;
      color: var(--muted);
    }
    .grid {
      display: grid;
      grid-template-columns: 1.2fr .8fr;
      gap: 14px;
    }
    .card {
      border: 1px solid var(--border);
      background: var(--card);
      border-radius: 18px;
      padding: 14px;
      backdrop-filter: blur(8px);
    }
    .card h3 {
      margin: 0 0 10px;
      font-size: 1rem;
      color: #fff;
      letter-spacing: .2px;
    }
    .feed {
      width: 100%;
      border-radius: 12px;
      border: 1px solid #ffffff2f;
      background: #05080c;
      min-height: 220px;
      object-fit: cover;
    }
    .row {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
      margin-top: 10px;
    }
    .kv {
      border: 1px solid #ffffff1f;
      border-radius: 12px;
      padding: 8px;
      background: #00000020;
    }
    .k { color: var(--muted); font-size: .8rem; }
    .v { font-weight: 600; margin-top: 2px; font-size: .95rem; }
    label { display: block; margin-top: 8px; font-size: .85rem; color: var(--muted); }
    input, button {
      width: 100%;
      border-radius: 10px;
      border: 1px solid #ffffff2f;
      padding: 9px 10px;
      background: #0b1622;
      color: white;
      margin-top: 6px;
    }
    button {
      background: linear-gradient(135deg, var(--accent), #fb8500);
      color: #1a1a1a;
      border: none;
      font-weight: 700;
      cursor: pointer;
      transition: transform .14s ease;
    }
    button:hover { transform: translateY(-1px); }
    .secondary {
      background: linear-gradient(135deg, var(--accent2), #3bb0f0);
    }
    input[type=range] {
      accent-color: #ffb703;
      padding: 0;
    }
    #debug {
      width: 100%;
      height: 220px;
      border-radius: 12px;
      border: 1px solid #ffffff30;
      background: #04070bcc;
      color: #84f8b5;
      font-family: var(--mono);
      font-size: .78rem;
      padding: 10px;
      resize: vertical;
    }
    .status-ok { color: var(--ok); }
    .status-err { color: var(--err); }
    @media (max-width: 960px) {
      .grid { grid-template-columns: 1fr; }
    }
  </style>
</head>
<body>
  <div class="wrap">
    <div class="top">
      <div class="title">bus#1 ESP32-CAM Portal</div>
      <div class="pill" id="time">Time: --</div>
    </div>

    <div class="grid">
      <div class="card">
        <h3>Live Camera Feed</h3>
        <img id="feed" class="feed" alt="camera feed"/>
        <div class="row">
          <div class="kv"><div class="k">STA Internet</div><div class="v" id="sta">-</div></div>
          <div class="kv"><div class="k">Last Upload</div><div class="v" id="upload">-</div></div>
          <div class="kv"><div class="k">SD Total</div><div class="v" id="total">-</div></div>
          <div class="kv"><div class="k">SD Used</div><div class="v" id="used">-</div></div>
          <div class="kv"><div class="k">Previous Recording Time</div><div class="v" id="prev">-</div></div>
          <div class="kv"><div class="k">Gateway</div><div class="v">1.2.3.4</div></div>
        </div>
        <label>Inbuilt Light Slider: <span id="lightVal">0</span></label>
        <input id="light" type="range" min="0" max="255" value="0"/>
      </div>

      <div class="card">
        <h3>Internet + API Settings</h3>
        <label>Auto Scan Wi-Fi Networks</label>
        <button class="secondary" onclick="refreshNetworks()">Scan Networks</button>
        <select id="ssidList" style="width:100%;margin-top:6px;border-radius:10px;padding:9px 10px;background:#0b1622;color:#fff;border:1px solid #ffffff2f;"></select>
        <label>Internet SSID</label><input id="ssid" placeholder="Your Wi-Fi SSID"/>
        <label>Internet Password</label><input id="pass" type="password" placeholder="Your Wi-Fi Password"/>
        <label>API Endpoint (1 image/min)</label><input id="api" placeholder="https://example.com/api/upload"/>
        <label>API Bearer Token (optional)</label><input id="token" placeholder="token"/>
        <label>Google Time API URL (Kathmandu)</label><input id="timeApi" placeholder="https://..."/>
        <button onclick="saveConfig()">Save Config + Connect</button>
        <button class="secondary" onclick="captureNow()">Send One API Photo Now</button>

        <h3 style="margin-top:14px;">Recorder Tuning</h3>
        <label>Segment Duration (ms)</label><input id="segmentMs" type="number" min="4000" max="60000" step="1000" value="10000"/>
        <label>SD Cleanup Threshold (%)</label><input id="cleanupPercent" type="number" min="60" max="98" step="1" value="90"/>
        <label>Minimum Segments To Keep</label><input id="minKeep" type="number" min="2" max="500" step="1" value="12"/>
        <button class="secondary" onclick="saveTuning()">Save Tuning</button>
      </div>
    </div>

    <div class="card">
      <h3>Debug Console (Serial-Monitor Style)</h3>
      <textarea id="debug" readonly></textarea>
    </div>

    <div class="card">
      <h3>Previous Recordings (SD)</h3>
      <div class="row">
        <div class="kv"><div class="k">Saved Videos</div><div class="v" id="recCount">0</div></div>
        <div class="kv"><div class="k">Recorder</div><div class="v" id="playState">Stopped</div></div>
      </div>
      <label>Choose Recording</label>
      <select id="records" style="width:100%;margin-top:6px;border-radius:10px;padding:9px 10px;background:#0b1622;color:#fff;border:1px solid #ffffff2f;"></select>
      <div class="row">
        <button onclick="showSelected()">Play Selected Video</button>
        <button class="secondary" onclick="toggleRecording()">Start/Stop Recording</button>
      </div>
      <div class="row">
        <button onclick="switchToRecordedMode()">Recorded Mode</button>
        <button class="secondary" onclick="switchToLiveMode()">Live Mode</button>
      </div>
      <button class="secondary" onclick="backToLive()">Back To Live Feed</button>
      <img id="recordPreview" class="feed" alt="record preview" style="margin-top:10px;min-height:180px;"/>
    </div>
  </div>

<script>
let debugCursor = 0;
let lightThrottleTimer = null;
let lightPending = 0;
let records = [];
let recordingNow = false;
let inPlayback = false;
let currentMode = 'live';
const feed = document.getElementById('feed');

const startLiveFeed = () => {
  if (inPlayback) {
    return;
  }
  feed.src = `http://${location.hostname}:81/stream?t=${Date.now()}`;
};

const stopLiveFeed = () => {
  feed.src = '';
};

feed.onerror = () => {
  if (!inPlayback && currentMode === 'live') {
    setTimeout(() => startLiveFeed(), 1200);
  }
};

startLiveFeed();

const getStatus = async () => {
  const r = await fetch('/api/status');
  const j = await r.json();
  document.getElementById('time').textContent = 'Time: ' + j.time;
  document.getElementById('sta').textContent = j.internet ? ('Connected ' + j.staIp) : 'Disconnected';
  document.getElementById('upload').textContent = j.lastUpload;
  document.getElementById('total').textContent = j.sdTotalMB + ' MB';
  document.getElementById('used').textContent = j.sdUsedMB + ' MB';
  document.getElementById('prev').textContent = j.lastRecord;
  recordingNow = !!j.recording;
  currentMode = j.mode || currentMode;
  document.getElementById('playState').textContent = recordingNow ? 'Recording' : 'Stopped';
};

const pollDebug = async () => {
  const r = await fetch('/api/debug?from=' + debugCursor);
  const j = await r.json();
  const box = document.getElementById('debug');
  if (j.lines && j.lines.length) {
    for (const line of j.lines) box.value += line + '\n';
    debugCursor = j.next;
    box.scrollTop = box.scrollHeight;
  }
};

const setLight = async (v) => {
  document.getElementById('lightVal').textContent = v;
  await fetch('/api/light?value=' + encodeURIComponent(v));
};

const queueLight = (v) => {
  lightPending = v;
  document.getElementById('lightVal').textContent = v;
  if (lightThrottleTimer) {
    return;
  }
  lightThrottleTimer = setTimeout(async () => {
    const value = lightPending;
    lightThrottleTimer = null;
    await setLight(value);
  }, 120);
};

document.getElementById('light').addEventListener('input', e => queueLight(e.target.value));
document.getElementById('light').addEventListener('change', e => setLight(e.target.value));

const saveConfig = async () => {
  const payload = {
    ssid: document.getElementById('ssid').value,
    pass: document.getElementById('pass').value,
    api: document.getElementById('api').value,
    token: document.getElementById('token').value,
    timeApi: document.getElementById('timeApi').value
  };
  const r = await fetch('/api/config', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify(payload)
  });
  const t = await r.text();
  alert(t);
};

const refreshNetworks = async () => {
  const r = await fetch('/api/wifi-scan');
  const j = await r.json();
  const sel = document.getElementById('ssidList');
  sel.innerHTML = '';

  for (const item of (j.items || [])) {
    const o = document.createElement('option');
    const lock = item.secure ? 'locked' : 'open';
    o.value = item.ssid;
    o.textContent = `${item.ssid} (${item.rssi} dBm, ${lock})`;
    sel.appendChild(o);
  }

  if ((j.items || []).length) {
    document.getElementById('ssid').value = j.items[0].ssid;
  }
};

document.getElementById('ssidList').addEventListener('change', e => {
  document.getElementById('ssid').value = e.target.value;
});

const captureNow = async () => {
  const r = await fetch('/api/capture');
  const t = await r.text();
  alert(t);
};

const loadTuning = async () => {
  const r = await fetch('/api/tuning');
  const j = await r.json();
  document.getElementById('segmentMs').value = j.segmentMs;
  document.getElementById('cleanupPercent').value = j.cleanupPercent;
  document.getElementById('minKeep').value = j.minKeep;
};

const saveTuning = async () => {
  const segmentMs = document.getElementById('segmentMs').value;
  const cleanupPercent = document.getElementById('cleanupPercent').value;
  const minKeep = document.getElementById('minKeep').value;
  const qs = `segmentMs=${encodeURIComponent(segmentMs)}&cleanupPercent=${encodeURIComponent(cleanupPercent)}&minKeep=${encodeURIComponent(minKeep)}`;
  const r = await fetch('/api/tuning/set?' + qs);
  const j = await r.json();
  document.getElementById('segmentMs').value = j.segmentMs;
  document.getElementById('cleanupPercent').value = j.cleanupPercent;
  document.getElementById('minKeep').value = j.minKeep;
  alert('Tuning saved');
};

const loadRecords = async () => {
  const r = await fetch('/api/records');
  const j = await r.json();
  records = j.items || [];
  document.getElementById('recCount').textContent = String(j.count || records.length || 0);

  const sel = document.getElementById('records');
  sel.innerHTML = '';
  for (const item of records) {
    const o = document.createElement('option');
    o.value = item.path;
    o.textContent = item.name;
    sel.appendChild(o);
  }
  if (!records.length) {
    document.getElementById('recordPreview').src = '';
  }
};

const showSelected = () => {
  const sel = document.getElementById('records');
  if (!sel.value) {
    return;
  }
  inPlayback = true;
  currentMode = 'recorded';
  fetch('/api/mode/set?value=recorded').catch(()=>{});
  stopLiveFeed();
  document.getElementById('recordPreview').src = '/media?f=' + encodeURIComponent(sel.value) + '&t=' + Date.now();
};

const backToLive = () => {
  inPlayback = false;
  currentMode = 'live';
  fetch('/api/mode/set?value=live').catch(()=>{});
  document.getElementById('recordPreview').src = '';
  startLiveFeed();
};

const switchToRecordedMode = async () => {
  inPlayback = true;
  currentMode = 'recorded';
  await fetch('/api/mode/set?value=recorded');
  stopLiveFeed();
};

const switchToLiveMode = async () => {
  inPlayback = false;
  currentMode = 'live';
  await fetch('/api/mode/set?value=live');
  document.getElementById('recordPreview').src = '';
  startLiveFeed();
};

const toggleRecording = async () => {
  const action = recordingNow ? 'stop' : 'start';
  const r = await fetch('/api/record?action=' + action);
  const t = await r.text();
  alert(t);
  await getStatus();
  await loadRecords();
};

setInterval(() => { getStatus().catch(()=>{}); pollDebug().catch(()=>{}); }, 3000);
setInterval(() => { loadRecords().catch(()=>{}); }, 30000);
getStatus();
pollDebug();
loadRecords();
loadTuning();

window.saveConfig = saveConfig;
window.captureNow = captureNow;
window.saveTuning = saveTuning;
window.refreshNetworks = refreshNetworks;
window.showSelected = showSelected;
window.toggleRecording = toggleRecording;
window.backToLive = backToLive;
window.switchToLiveMode = switchToLiveMode;
window.switchToRecordedMode = switchToRecordedMode;
</script>
</body>
</html>
)rawliteral";

String redirectUrl() {
  return String("http://") + portalIP.toString() + "/";
}

void handleCaptiveRedirect() {
  server.sendHeader("Location", redirectUrl(), true);
  server.send(302, "text/plain", "Redirecting to portal...");
}

void handleStatus() {
  touchPortalActivity();
  updateCachedSdStats();

  String json = "{";
  json += "\"time\":\"" + escapeJson(nowString()) + "\",";
  json += "\"internet\":" + String(internetConnected ? "true" : "false") + ",";
  json += "\"recording\":" + String(recordingEnabled ? "true" : "false") + ",";
  json += "\"mode\":\"" + String(liveModeEnabled ? "live" : "recorded") + "\",";
  json += "\"segmentMs\":" + String(videoSegmentMs) + ",";
  json += "\"cleanupPercent\":" + String(sdUsageLimitPercent) + ",";
  json += "\"minKeep\":" + String(minVideoSegmentsToKeep) + ",";
  json += "\"staIp\":\"" + escapeJson(WiFi.localIP().toString()) + "\",";
  json += "\"lastUpload\":\"" + escapeJson(lastUploadStatus) + "\",";
  json += "\"lastRecord\":\"" + escapeJson(lastRecordTime) + "\",";
  json += "\"sdTotalMB\":" + String((unsigned long)cachedSdTotalMB) + ",";
  json += "\"sdUsedMB\":" + String((unsigned long)cachedSdUsedMB);
  json += "}";

  server.send(200, "application/json", json);
}

void handleDebug() {
  touchPortalActivity();
  uint32_t from = 0;
  if (server.hasArg("from")) {
    from = (uint32_t)server.arg("from").toInt();
  }

  String json = "{\"lines\":[";
  bool first = true;
  int sent = 0;
  const int maxLines = 35;

  for (size_t i = 0; i < DEBUG_RING_SIZE; i++) {
    if (debugSeq[i] == 0) {
      continue;
    }
    if (debugSeq[i] <= from) {
      continue;
    }

    if (!first) {
      json += ",";
    }
    first = false;
    json += "\"" + escapeJson(debugRing[i]) + "\"";
    sent++;
    if (sent >= maxLines) {
      break;
    }
  }

  json += "],\"next\":" + String(debugCounter) + "}";
  server.send(200, "application/json", json);
}

void handleLight() {
  touchPortalActivity();
  int value = 0;
  if (server.hasArg("value")) {
    value = server.arg("value").toInt();
  }
  userLightDuty = constrain(value, 0, 255);

  setFlashDuty(userLightDuty);

  unsigned long nowMs = millis();
  if (nowMs - lastLightLogMs > 1200) {
    lastLightLogMs = nowMs;
    logDebug("Flash slider set to " + String(userLightDuty));
  }
  server.send(200, "text/plain", "OK");
}

void handleRecords() {
  touchPortalActivity();
  if (!sdReady) {
    server.send(200, "application/json", "{\"items\":[],\"count\":0}");
    return;
  }

  File dir = SD_MMC.open("/videos");
  if (!dir || !dir.isDirectory()) {
    server.send(200, "application/json", "{\"items\":[],\"count\":0}");
    return;
  }

  String paths[80];
  int n = 0;
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    if (!entry.isDirectory()) {
      String p = String(entry.path());
      if (p.endsWith(".mjpeg")) {
        if (n < 80) {
          paths[n++] = p;
        }
      }
    }
    entry.close();
  }
  dir.close();

  for (int i = 0; i < n - 1; i++) {
    for (int j = i + 1; j < n; j++) {
      if (paths[i] < paths[j]) {
        String tmp = paths[i];
        paths[i] = paths[j];
        paths[j] = tmp;
      }
    }
  }

  String json = "{\"items\":[";
  for (int i = 0; i < n; i++) {
    if (i) {
      json += ",";
    }
    String name = paths[i];
    int slash = name.lastIndexOf('/');
    if (slash >= 0) {
      name = name.substring(slash + 1);
    }
    json += "{\"name\":\"" + escapeJson(name) + "\",\"path\":\"" + escapeJson(paths[i]) + "\"}";
  }
  json += "],\"count\":" + String(n) + "}";
  server.send(200, "application/json", json);
}

void handleMedia() {
  touchPortalActivity();
  if (!sdReady) {
    server.send(404, "text/plain", "SD not ready");
    return;
  }
  if (!server.hasArg("f")) {
    server.send(400, "text/plain", "Missing file path");
    return;
  }

  String p = server.arg("f");
  if (!p.startsWith("/videos/") || p.indexOf("..") >= 0 || !p.endsWith(".mjpeg")) {
    server.send(403, "text/plain", "Forbidden path");
    return;
  }

  WiFiClient client = server.client();
  File f = SD_MMC.open(p, FILE_READ);
  if (!f || f.isDirectory()) {
    server.send(404, "text/plain", "Not found");
    if (f) {
      f.close();
    }
    return;
  }

  playbackActive = true;
  liveModeEnabled = false;
  streamActive = false;

  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: multipart/x-mixed-replace; boundary=frame\r\n");
  client.print("Cache-Control: no-store\r\n");
  client.print("Connection: close\r\n\r\n");

  while (client.connected()) {
    uint32_t frameLen = 0;
    int readLen = f.read((uint8_t *)&frameLen, sizeof(frameLen));
    if (readLen != (int)sizeof(frameLen)) {
      break;
    }
    if (frameLen == 0 || frameLen > 350000) {
      break;
    }

    client.print("--frame\r\n");
    client.print("Content-Type: image/jpeg\r\n");
    client.print("Content-Length: " + String(frameLen) + "\r\n\r\n");

    uint8_t chunk[1460];
    uint32_t remaining = frameLen;
    bool frameOk = true;
    while (remaining > 0) {
      size_t want = remaining > sizeof(chunk) ? sizeof(chunk) : remaining;
      int got = f.read(chunk, want);
      if (got <= 0) {
        frameOk = false;
        break;
      }
      client.write(chunk, got);
      remaining -= (uint32_t)got;
    }

    if (!frameOk) {
      break;
    }

    client.print("\r\n");
    delay(45);
  }

  f.close();
  playbackActive = false;
}

void handleRecordControl() {
  touchPortalActivity();
  String action = server.arg("action");
  if (action == "start") {
    bool ok = startVideoRecording();
    server.send(200, "text/plain", ok ? "Recording started" : "Recording start failed");
    return;
  }
  if (action == "stop") {
    stopVideoRecording();
    server.send(200, "text/plain", "Recording stopped");
    return;
  }
  server.send(400, "text/plain", "Use action=start or action=stop");
}

void handleModeGet() {
  touchPortalActivity();
  String json = "{";
  json += "\"mode\":\"" + String(liveModeEnabled ? "live" : "recorded") + "\"";
  json += "}";
  server.send(200, "application/json", json);
}

void handleModeSet() {
  touchPortalActivity();
  String value = server.arg("value");
  value.toLowerCase();

  if (value == "live") {
    liveModeEnabled = true;
    playbackActive = false;
    server.send(200, "text/plain", "Mode: live");
    return;
  }

  if (value == "recorded") {
    liveModeEnabled = false;
    server.send(200, "text/plain", "Mode: recorded");
    return;
  }

  server.send(400, "text/plain", "Use value=live or value=recorded");
}

void handleWifiScan() {
  touchPortalActivity();
  int count = WiFi.scanNetworks(false, true, false, 300);
  if (count <= 0) {
    server.send(200, "application/json", "{\"items\":[],\"count\":0}");
    return;
  }

  String json = "{\"items\":[";
  int emitted = 0;
  for (int i = 0; i < count && emitted < 25; i++) {
    String ssid = WiFi.SSID(i);
    if (ssid.length() == 0) {
      continue;
    }
    if (emitted) {
      json += ",";
    }
    bool secure = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    json += "{\"ssid\":\"" + escapeJson(ssid) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + ",\"secure\":" + String(secure ? "true" : "false") + "}";
    emitted++;
  }
  json += "],\"count\":" + String(emitted) + "}";
  WiFi.scanDelete();
  server.send(200, "application/json", json);
}

void handleTuningGet() {
  touchPortalActivity();
  String json = "{";
  json += "\"segmentMs\":" + String(videoSegmentMs) + ",";
  json += "\"cleanupPercent\":" + String(sdUsageLimitPercent) + ",";
  json += "\"minKeep\":" + String(minVideoSegmentsToKeep);
  json += "}";
  server.send(200, "application/json", json);
}

void handleTuningSet() {
  touchPortalActivity();
  bool changed = false;

  if (server.hasArg("segmentMs")) {
    uint32_t v = (uint32_t)server.arg("segmentMs").toInt();
    v = constrain(v, 4000, 60000);
    if (v != videoSegmentMs) {
      videoSegmentMs = v;
      changed = true;
    }
  }

  if (server.hasArg("cleanupPercent")) {
    uint8_t v = (uint8_t)server.arg("cleanupPercent").toInt();
    v = (uint8_t)constrain(v, 60, 98);
    if (v != sdUsageLimitPercent) {
      sdUsageLimitPercent = v;
      changed = true;
    }
  }

  if (server.hasArg("minKeep")) {
    uint16_t v = (uint16_t)server.arg("minKeep").toInt();
    v = (uint16_t)constrain(v, 2, 500);
    if (v != minVideoSegmentsToKeep) {
      minVideoSegmentsToKeep = v;
      changed = true;
    }
  }

  if (changed) {
    saveSettings();
    logDebug("Tuning updated: segmentMs=" + String(videoSegmentMs) + ", cleanup=" + String(sdUsageLimitPercent) + "%, minKeep=" + String(minVideoSegmentsToKeep));
  }

  handleTuningGet();
}

String getJsonValue(const String &json, const String &key) {
  String token = "\"" + key + "\"";
  int keyPos = json.indexOf(token);
  if (keyPos < 0) {
    return "";
  }

  int colon = json.indexOf(':', keyPos + token.length());
  if (colon < 0) {
    return "";
  }

  int firstQuote = json.indexOf('"', colon + 1);
  if (firstQuote < 0) {
    return "";
  }

  int secondQuote = json.indexOf('"', firstQuote + 1);
  if (secondQuote < 0) {
    return "";
  }

  return json.substring(firstQuote + 1, secondQuote);
}

void handleConfig() {
  touchPortalActivity();
  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "Missing JSON body");
    return;
  }

  String body = server.arg("plain");
  String ssid = getJsonValue(body, "ssid");
  String pass = getJsonValue(body, "pass");
  String api = getJsonValue(body, "api");
  String token = getJsonValue(body, "token");
  String timeApi = getJsonValue(body, "timeApi");

  if (ssid.length() > 0) {
    internetSsid = ssid;
  }
  if (pass.length() > 0 || body.indexOf("\"pass\":\"\"") >= 0) {
    internetPassword = pass;
  }
  if (api.length() > 0) {
    apiEndpoint = api;
  }
  if (token.length() > 0 || body.indexOf("\"token\":\"\"") >= 0) {
    apiBearerToken = token;
  }
  if (timeApi.length() > 0) {
    googleTimeApiUrl = timeApi;
  }

  saveSettings();
  bool ok = connectToInternet();

  server.send(200, "text/plain", ok ? "Saved. Internet connected." : "Saved. Internet not connected yet.");
}

void handleCaptureNow() {
  touchPortalActivity();
  requestUploadCycle();
  server.send(200, "text/plain", "Capture queued.");
}

void setupRoutes() {
  server.on("/", HTTP_GET, []() {
    touchPortalActivity();
    server.send_P(200, "text/html", INDEX_HTML);
  });
  server.on("/index.html", HTTP_GET, []() {
    touchPortalActivity();
    server.send_P(200, "text/html", INDEX_HTML);
  });

  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/debug", HTTP_GET, handleDebug);
  server.on("/api/light", HTTP_GET, handleLight);
  server.on("/api/config", HTTP_POST, handleConfig);
  server.on("/api/capture", HTTP_GET, handleCaptureNow);
  server.on("/api/record", HTTP_GET, handleRecordControl);
  server.on("/api/mode", HTTP_GET, handleModeGet);
  server.on("/api/mode/set", HTTP_GET, handleModeSet);
  server.on("/api/wifi-scan", HTTP_GET, handleWifiScan);
  server.on("/api/tuning", HTTP_GET, handleTuningGet);
  server.on("/api/tuning/set", HTTP_GET, handleTuningSet);
  server.on("/api/records", HTTP_GET, handleRecords);
  server.on("/media", HTTP_GET, handleMedia);

  // Captive portal compatibility endpoints.
  server.on("/generate_204", HTTP_GET, handleCaptiveRedirect);
  server.on("/hotspot-detect.html", HTTP_GET, handleCaptiveRedirect);
  server.on("/connecttest.txt", HTTP_GET, handleCaptiveRedirect);
  server.on("/ncsi.txt", HTTP_GET, handleCaptiveRedirect);
  server.on("/fwlink", HTTP_GET, handleCaptiveRedirect);
  server.on("/redirect", HTTP_GET, handleCaptiveRedirect);
  server.on("/success.txt", HTTP_GET, handleCaptiveRedirect);

  server.onNotFound(handleCaptiveRedirect);
}

void handleStreamClient() {
  if (!liveModeEnabled || playbackActive) {
    delay(4);
    return;
  }

  WiFiClient client = streamServer.available();
  if (!client) {
    return;
  }

  client.setNoDelay(true);
  streamActive = true;
  logDebug("Live stream client connected.");

  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: multipart/x-mixed-replace; boundary=frame\r\n");
  client.print("Connection: close\r\n\r\n");

  while (client.connected()) {
    if (playbackActive) {
      break;
    }

    camera_fb_t *fb = cameraGrabFrame(40);
    if (!fb) {
      delay(1);
      continue;
    }

    client.print("--frame\r\n");
    client.print("Content-Type: image/jpeg\r\n");
    client.print("Content-Length: " + String(fb->len) + "\r\n\r\n");
    client.write(fb->buf, fb->len);
    client.print("\r\n");

    cameraReturnFrame(fb);
    delay(1);

    if (!client.connected()) {
      break;
    }
  }

  client.stop();
  streamActive = false;
  logDebug("Live stream client disconnected.");
}

void streamTask(void *parameter) {
  while (true) {
    handleStreamClient();
    vTaskDelay(1);
  }
}

void handleNoClientFlicker() {
  uint8_t clients = WiFi.softAPgetStationNum();
  unsigned long now = millis();
  bool recentlyActivePortal = (now - lastPortalActivityMs) < 8000;

  if (clients == 0 && !recentlyActivePortal && !streamActive) {
    if (now - lastFlickerMs >= (unsigned long)flickerIntervalMs) {
      lastFlickerMs = now;
      flickerState = !flickerState;
      setFlashDuty(flickerState ? flickerDuty : 0);
    }
  } else {
    setFlashDuty(userLightDuty);
  }
}

void setup() {
  Serial.begin(115200);
  delay(250);

  setCpuFrequencyMhz(240);

  cameraMutex = xSemaphoreCreateMutex();
  if (!cameraMutex) {
    Serial.println("Failed to create camera mutex.");
  }
  recordMutex = xSemaphoreCreateMutex();
  if (!recordMutex) {
    Serial.println("Failed to create record mutex.");
  }

  ledcAttach(FLASH_LED_PIN, 5000, 8);
  setFlashDuty(0);

  if (!initCamera()) {
    Serial.println("Camera init failed. Rebooting...");
    delay(2000);
    ESP.restart();
  }

  sdReady = initSD();
  if (sdReady) {
    logDebug("SD card initialized.");
  } else {
    Serial.println("SD card not ready.");
  }

  loadSettings();

  WiFi.mode(WIFI_AP_STA);
  WiFi.setSleep(false);
  WiFi.setTxPower(WIFI_POWER_19_5dBm);
  bool apConfigOk = WiFi.softAPConfig(AP_IP, AP_GATEWAY, AP_SUBNET);

  bool apOk = false;
  if (strlen(AP_PASSWORD) == 0) {
    apOk = WiFi.softAP(AP_SSID);
  } else {
    apOk = WiFi.softAP(AP_SSID, AP_PASSWORD);
  }

  if (!apOk) {
    Serial.println("Failed to start AP.");
  }

  portalIP = WiFi.softAPIP();
  if (!apConfigOk) {
    logDebug("softAPConfig failed, using default AP IP " + portalIP.toString());
  }

  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", portalIP);

  setupRoutes();
  server.begin();
  streamServer.begin();
  xTaskCreatePinnedToCore(streamTask, "streamTask", 8192, nullptr, 1, &streamTaskHandle, 0);
  xTaskCreatePinnedToCore(recorderTask, "recorderTask", 8192, nullptr, 1, &recorderTaskHandle, 1);
  xTaskCreatePinnedToCore(uploadTask, "uploadTask", 8192, nullptr, 1, &uploadTaskHandle, 1);

  logDebug("AP started: SSID bus#1 (open)");
  logDebug("Configured gateway target: 1.2.3.4");
  logDebug("Active AP IP: " + portalIP.toString());
  logDebug("Wi-Fi TX power set to maximum supported level.");
  logDebug("CPU frequency set to " + String(getCpuFrequencyMhz()) + " MHz");
  logDebug("Portal ready on http://" + portalIP.toString() + "/");

  connectToInternet();
  syncTimeNepal();

  if (sdReady) {
    startVideoRecording();
  }
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  handleNoClientFlicker();
  cleanupOldVideoSegmentsIfNeeded();
  maintainSdLogs();

  if (WiFi.status() != WL_CONNECTED) {
    internetConnected = false;
    if (millis() - lastStatusLogMs > 30000) {
      lastStatusLogMs = millis();
      connectToInternet();
    }
  } else {
    internetConnected = true;
  }

  if (millis() - lastUploadMs >= 60000) {
    lastUploadMs = millis();
    requestUploadCycle();
  }
}
