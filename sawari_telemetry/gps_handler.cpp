/**
 * ============================================================================
 * SAWARI Bus Telemetry Device - GPS Handler Implementation
 * ============================================================================
 * 
 * Interfaces with the NEO-8M GPS module over UART2 (GPIO16 RX, GPIO17 TX).
 * Uses TinyGPSPlus for NMEA sentence parsing to extract:
 *   - Latitude / Longitude
 *   - Speed (km/h)
 *   - Course / Direction (degrees)
 *   - Altitude (meters)
 *   - Satellite count (GPS + GLONASS)
 *   - HDOP (Horizontal Dilution of Precision)
 *   - UTC Timestamp (ISO 8601)
 * 
 * The NEO-8M is configured via UBX protocol at startup to enable multi-GNSS
 * (GPS + GLONASS) and a 5Hz navigation update rate for improved accuracy
 * and faster position updates compared to the NEO-6M.
 * 
 * The gpsUpdate() function must be called every loop iteration to ensure
 * no NMEA sentences are missed from the serial buffer.
 * ============================================================================
 */

#include "gps_handler.h"
#include "config.h"
#include "network_handler.h"
#include <TinyGPSPlus.h>

// --- GPS parser and serial instances ---
static TinyGPSPlus _gps;
static HardwareSerial _gpsSerial(2);    // UART2

// ============================================================================
// UBX Protocol Helpers (NEO-8M Configuration)
// ============================================================================

/**
 * Send a UBX message with auto-computed Fletcher checksum.
 * @param payload bytes starting from class ID through end of payload
 * @param len     number of bytes in payload
 */
static void ubxSend(const uint8_t* payload, size_t len) {
    _gpsSerial.write(0xB5);  // sync char 1
    _gpsSerial.write(0x62);  // sync char 2

    uint8_t ckA = 0, ckB = 0;
    for (size_t i = 0; i < len; i++) {
        _gpsSerial.write(payload[i]);
        ckA += payload[i];
        ckB += ckA;
    }
    _gpsSerial.write(ckA);
    _gpsSerial.write(ckB);
}

/**
 * Configure NEO-8M for optimal multi-GNSS performance via UBX commands.
 *   - Enables GPS + GLONASS constellation tracking
 *   - Sets 5Hz navigation update rate (200ms measurement interval)
 *   - Saves configuration to battery-backed RAM and flash
 */
static void gpsConfigureNeo8M() {
    // UBX-CFG-RATE: Set measurement rate to 200ms (5Hz), navRate=1, timeRef=UTC
    static const uint8_t cfgRate[] = {
        0x06, 0x08, 0x06, 0x00,
        0xC8, 0x00,   // measRate = 200ms (5Hz)
        0x01, 0x00,   // navRate  = 1 cycle
        0x01, 0x00    // timeRef  = UTC
    };
    ubxSend(cfgRate, sizeof(cfgRate));
    delay(100);

    // UBX-CFG-GNSS: Enable GPS + SBAS + GLONASS + QZSS, disable Galileo/BeiDou/IMES
    // 7 config blocks, 32 tracking channels
    static const uint8_t cfgGnss[] = {
        0x06, 0x3E, 0x3C, 0x00,         // class=CFG, id=GNSS, len=60
        0x00, 0x00, 0x20, 0x07,         // msgVer=0, numTrkChHw=0, numTrkChUse=32, numConfigBlocks=7
        // GPS     (gnssId=0): enabled, 8 min / 16 max channels
        0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01,
        // SBAS    (gnssId=1): enabled, 1 min / 3 max channels
        0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01,
        // Galileo (gnssId=2): disabled
        0x02, 0x04, 0x08, 0x00, 0x00, 0x00, 0x01, 0x01,
        // BeiDou  (gnssId=3): disabled
        0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01,
        // IMES    (gnssId=4): disabled
        0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x03,
        // QZSS    (gnssId=5): enabled, 0 min / 3 max channels
        0x05, 0x00, 0x03, 0x00, 0x01, 0x00, 0x01, 0x05,
        // GLONASS (gnssId=6): enabled, 8 min / 14 max channels
        0x06, 0x08, 0x0E, 0x00, 0x01, 0x00, 0x01, 0x01
    };
    ubxSend(cfgGnss, sizeof(cfgGnss));
    delay(100);

    // UBX-CFG-CFG: Save current configuration to BBR + Flash
    static const uint8_t cfgSave[] = {
        0x06, 0x09, 0x0D, 0x00,
        0x00, 0x00, 0x00, 0x00,   // clearMask  = none
        0xFF, 0xFF, 0x00, 0x00,   // saveMask   = all sections
        0x00, 0x00, 0x00, 0x00,   // loadMask   = none
        0x17                      // deviceMask = BBR + Flash + EEPROM
    };
    ubxSend(cfgSave, sizeof(cfgSave));
    delay(100);

    Serial.println(F("[GPS] NEO-8M configured: GPS+GLONASS enabled, 5Hz update rate"));
}

/**
 * Initialize UART2 for GPS communication and configure NEO-8M.
 * Default baud rate is 9600 (same as NEO-6M, compatible out-of-box).
 */
void gpsInit() {
    _gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
    Serial.println(F("[GPS] UART2 initialized at 9600 baud (NEO-8M)"));
    Serial.print(F("[GPS] RX pin: ")); Serial.print(GPS_RX_PIN);
    Serial.print(F(" | TX pin: ")); Serial.println(GPS_TX_PIN);

    // Allow the NEO-8M to boot before sending UBX configuration
    delay(500);
    gpsConfigureNeo8M();
}

/**
 * Feed all available bytes from GPS serial into the TinyGPSPlus parser.
 * This is non-blocking and processes whatever data is in the UART buffer.
 */
void gpsUpdate() {
    while (_gpsSerial.available() > 0) {
        char c = _gpsSerial.read();
        _gps.encode(c);
    }
}

/**
 * Check if GPS has a valid and recent location fix.
 * Uses isValid() for fix validity and age() to ensure data freshness.
 * 
 * NOTE: We intentionally do NOT use isUpdated() here because it auto-clears
 * after each call, which would cause the fix flag to be true for only one
 * loop iteration per NMEA update (~10ms out of every 1000ms). This would
 * cause the 2-second send interval to frequently miss the "fix available"
 * window. Using age() < 5000ms ensures the data is recent without the
 * one-shot clearing behavior.
 */
bool gpsHasFix() {
    return _gps.location.isValid() && _gps.location.age() < 5000;
}

/**
 * Check if GPS has valid time and date data.
 */
bool gpsHasTime() {
    return _gps.date.isValid() && _gps.time.isValid();
}

/**
 * Get satellite count regardless of fix status.
 * Useful for displaying acquisition progress.
 */
int gpsGetSatellites() {
    return _gps.satellites.isValid() ? (int)_gps.satellites.value() : 0;
}

/**
 * Populate a TelemetryData struct with current GPS readings.
 * 
 * IMPORTANT: Only call this when gpsHasFix() returns true,
 * otherwise the data will contain default/stale values.
 */
void gpsGetTelemetry(TelemetryData* data) {
    // Latitude and longitude in decimal degrees
    data->latitude  = _gps.location.lat();
    data->longitude = _gps.location.lng();

    // Speed in km/h (TinyGPSPlus provides this directly)
    data->speed = _gps.speed.isValid() ? _gps.speed.kmph() : 0.0;

    // Course/direction in degrees (0 = North, 90 = East, etc.)
    data->direction = _gps.course.isValid() ? _gps.course.deg() : 0.0;

    // Altitude in meters above mean sea level
    data->altitude = _gps.altitude.isValid() ? _gps.altitude.meters() : 0.0;

    // Number of satellites used in fix
    data->satellites = gpsGetSatellites();

    // HDOP - lower is better (< 1.0 = excellent, 1-2 = good)
    data->hdop = _gps.hdop.isValid() ? (_gps.hdop.hdop()) : 99.9;

    // Format timestamp as ISO 8601 UTC string
    if (gpsHasTime()) {
        snprintf(data->timestamp, sizeof(data->timestamp),
                 "%04d-%02d-%02dT%02d:%02d:%02dZ",
                 _gps.date.year(),
                 _gps.date.month(),
                 _gps.date.day(),
                 _gps.time.hour(),
                 _gps.time.minute(),
                 _gps.time.second());
    } else {
        // Fallback if GPS time not yet acquired
        strncpy(data->timestamp, "1970-01-01T00:00:00Z", sizeof(data->timestamp));
    }
}

/**
 * Build a JSON payload string from telemetry data.
 * Uses snprintf for efficient string formatting without ArduinoJson dependency.
 * 
 * Output format matches the API specification:
 * {
 *   "data": {
 *     "bus_id": 1,
 *     "latitude": 27.712345,
 *     "longitude": 85.312345,
 *     "speed": 34.5,
 *     "direction": 182.4,
 *     "altitude": 1350.2,
 *     "satellites": 9,
 *     "hdop": 0.9,
 *     "timestamp": "2026-02-19T10:15:23Z"
 *   }
 * }
 */
String gpsFormatPayload(const TelemetryData* data) {
    char buffer[400];
    int busId = networkGetBusId();
    snprintf(buffer, sizeof(buffer),
        "{\"data\":{"
        "\"bus_id\":%d,"
        "\"latitude\":%.6f,"
        "\"longitude\":%.6f,"
        "\"speed\":%.1f,"
        "\"direction\":%.1f,"
        "\"altitude\":%.1f,"
        "\"satellites\":%d,"
        "\"hdop\":%.1f,"
        "\"timestamp\":\"%s\""
        "}}",
        busId,
        data->latitude,
        data->longitude,
        data->speed,
        data->direction,
        data->altitude,
        data->satellites,
        data->hdop,
        data->timestamp
    );
    return String(buffer);
}
