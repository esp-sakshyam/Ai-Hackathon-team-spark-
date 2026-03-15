# ESP32-CAM OV3660 Portal (bus#1)

This project creates a full captive portal camera system for ESP32-CAM (OV3660):

- AP name: `bus#1` (open / unlocked)
- Gateway + portal IP: `1.2.3.4`
- Auto captive redirect to `http://1.2.3.4/`
- Live camera feed in web UI
- CPU forced to 240 MHz for maximum performance
- Inbuilt flash slider control
- SD card storage stats and recording log
- Auto-start video recording on boot (MJPEG)
- Video segmentation every 10 seconds for safer storage
- Recorded video playback in web UI
- Live feed pauses automatically during recorded video playback and resumes on return
- Debug console in UI (serial-monitor style)
- Auto internet AP scan/connect (STA)
- Wi-Fi scan list in web UI for quick SSID selection
- Time sync for Kathmandu (NTP + Google time API placeholder)
- Auto upload: 1 image every minute to API endpoint placeholder
- Super-fast, super-dim flash flicker when no client is connected
- Auto storage cleanup when SD usage is high (oldest video segments removed)

## Required Libraries

Install these in Arduino IDE:

- ESP32 board package (Espressif)
- `esp_camera` (included with ESP32 board package examples/core)
- `WebServer` (ESP32 core)
- `DNSServer` (ESP32 core)
- `HTTPClient` (ESP32 core)
- `SD_MMC` (ESP32 core)
- `Preferences` (ESP32 core)

## Board Settings (recommended)

- Board: `AI Thinker ESP32-CAM`
- PSRAM: `Enabled`
- Partition scheme: `Huge APP` (or one that keeps enough app space)
- Upload speed: `115200` (safe)

## File

- Main sketch: `esp32_cam_portal.ino`

## API Placeholder Setup

In web portal settings, set:

- API Endpoint: your backend upload URL
- API Bearer Token: optional
- Google Time API URL: endpoint that returns JSON with `unixtime`, example:

```json
{"unixtime": 1760000000}
```

If the Google time API URL remains placeholder, the firmware uses NTP for Kathmandu timezone.

## SD Card Notes

- Recorded video segments are saved under `/videos` as `.mjpeg`
- Segment metadata appends to `/video_log.csv`
- Debug logs append to `/debug.log`

## Performance + Retention Defaults

- CPU frequency: `240 MHz`
- Recording frame cadence: `~5 FPS` (`200 ms` interval)
- Segment duration: `10 seconds` per file
- SD cleanup threshold: starts at `90%` usage
- Minimum kept segments: `12`

## Runtime Tuning (Web UI)

You can tune recorder behavior directly in the portal without reflashing:

- Segment Duration (ms): range `4000..60000`
- SD Cleanup Threshold (%): range `60..98`
- Minimum Segments To Keep: range `2..500`

These values are saved in flash (`Preferences`) and restored on boot.

## Tuning API Endpoints

- `GET /api/tuning` returns current values
- `GET /api/tuning/set?segmentMs=10000&cleanupPercent=90&minKeep=12` updates values

## Important

- AP is intentionally open because you asked for unlocked access.
- For production, secure the AP and API endpoint.
