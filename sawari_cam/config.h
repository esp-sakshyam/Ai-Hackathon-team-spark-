/*
 * ============================================================================
 *  SAWARI ESP32-CAM — Passenger Occupancy Detection
 *  Configuration File
 * ============================================================================
 *  Board  : AI-Thinker ESP32-CAM
 *  Purpose: Capture images every 30 s, POST to Sawari vision API for
 *           AI-based passenger counting.
 * ============================================================================
 */

#ifndef CONFIG_H
#define CONFIG_H

// ─── Vehicle Identity (default — can be changed via WiFi portal) ────────────
#define VEHICLE_ID            1          // Compile-time fallback only

// ─── Server Configuration (defaults — overridden by portal values in SPIFFS) ─
#define DEFAULT_SERVER_URL    "https://zenithkandel.com.np/sawari/app/hardware-api/passenger.php"
#define DEFAULT_VEHICLE_ID    "1"
#define HTTP_TIMEOUT_MS       15000      // 15 s (image uploads can be slow)
#define MAX_URL_LEN           256        // Max length for API URL field
#define MAX_VID_LEN           10         // Max length for vehicle ID field
#define CONFIG_FILE           "/config.json"   // SPIFFS path for saved settings

// ─── Timing ─────────────────────────────────────────────────────────────────
#define CAPTURE_INTERVAL_MS   30000      // 30 seconds between captures
#define BOOT_HOLD_MS          3000       // Hold BOOT button 3 s to launch WiFi portal

// ─── Pin Definitions (AI-Thinker ESP32-CAM) ─────────────────────────────────
#define PIN_FLASH_LED         4          // On-board white flash LED
#define PIN_STATUS_LED        33         // Small red LED (active LOW)
#define PIN_BOOT_BUTTON       0          // BOOT / GPIO0 button

// ─── WiFi Manager ───────────────────────────────────────────────────────────
#define WIFI_AP_NAME          "SAWARI_CAM_SETUP"
#define WIFI_AP_PASS          ""         // Open AP — leave empty
#define WIFI_PORTAL_TIMEOUT   180        // Seconds before portal auto-closes
#define WIFI_RECONNECT_MS     10000      // Retry interval when disconnected

// ─── Camera Settings ────────────────────────────────────────────────────────
#define CAM_FRAME_SIZE        FRAMESIZE_VGA   // 640x480 — good quality vs size balance
#define CAM_JPEG_QUALITY      12              // 10 = best, 63 = smallest
#define CAM_FB_COUNT          1               // 1 frame buffer is enough for snapshots

// ─── AI-Thinker ESP32-CAM Pin Map ───────────────────────────────────────────
#define PWDN_GPIO_NUM         32
#define RESET_GPIO_NUM        -1
#define XCLK_GPIO_NUM         0
#define SIOD_GPIO_NUM         26
#define SIOC_GPIO_NUM         27
#define Y9_GPIO_NUM           35
#define Y8_GPIO_NUM           34
#define Y7_GPIO_NUM           39
#define Y6_GPIO_NUM           36
#define Y5_GPIO_NUM           21
#define Y4_GPIO_NUM           19
#define Y3_GPIO_NUM           18
#define Y2_GPIO_NUM           5
#define VSYNC_GPIO_NUM        25
#define HREF_GPIO_NUM         23
#define PCLK_GPIO_NUM         22

#endif // CONFIG_H
