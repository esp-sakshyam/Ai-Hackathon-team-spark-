# ESP32 CAM Fix Build

This folder contains a clean rewrite focused on stability and responsiveness.

## File

- `esp32_cam_fix.ino`

## What this rewrite keeps

- AP `bus#1` (open), captive redirect, portal on AP IP
- Live feed default mode
- Explicit mode switch: `live` <-> `recorded`
- Recorded playback pauses live feed, back to live resumes stream
- Auto-start recording to SD (`/videos/*.mjpeg`)
- 10-second segment rotation (configurable in code)
- Auto upload one photo every minute to API
- Wi-Fi scan list and config save
- Debug view and SD log

## Why this rewrite is more stable

- Simplified UI and request flow
- No heavy startup API chaining in browser
- Upload moved to background task
- SD stats cached
- Playback streams in chunks (lower RAM pressure)
- Smaller debug ring buffer

## Install

Only ESP32 board package by Espressif Systems is needed.
