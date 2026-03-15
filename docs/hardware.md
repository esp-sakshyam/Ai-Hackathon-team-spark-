# SAWARI Bus Camera Device (ESP32-CAM)

## Hardware Module – Image Capture & Upload Unit

This device is a **small ESP32-CAM based hardware module** designed to capture photos inside a bus and send them to the Sawari backend server.  
The purpose of the module is to periodically take an image and upload it to an API endpoint where the Sawari system can analyze the image (for example: passenger counting or occupancy estimation).

The device is designed to be **low-cost, compact, and easy to install inside buses**.

---

# Overview

The ESP32-CAM module performs three simple tasks:

1. Connect to WiFi
2. Capture an image using the onboard camera
3. Send the image to the Sawari server API

This cycle repeats **every 10 seconds**.

---

# Hardware Components

| Qty | Component | Description |
|----|-----------|-------------|
| 1 | ESP32-CAM (AI Thinker) | Microcontroller with built-in WiFi and camera |
| 1 | OV2640 Camera | Default camera module included with ESP32-CAM |
| 1 | Buck Converter | 12V → 5V step-down power supply |
| 1 | FTDI Programmer | Used to upload firmware |
| 1 | MicroSD Card (optional) | Temporary image storage |
| 1 | Enclosure | Protective housing for installation |

---

# Hardware Specifications

| Parameter | Value |
|-----------|------|
| Microcontroller | ESP32 |
| Camera Sensor | OV2640 |
| Image Format | JPEG |
| Recommended Resolution | 800×600 |
| Connectivity | WiFi |
| Capture Interval | 10 seconds |
| Power Supply | 5V |
| Average Power Consumption | ~150mA |

---

# Power Supply

The device is powered from the vehicle's **12V electrical system**.
