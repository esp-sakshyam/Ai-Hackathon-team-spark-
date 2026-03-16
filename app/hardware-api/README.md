# Hardware API

API endpoints for IoT hardware devices (GPS modules, passenger cameras) to push data into Sawari.

---

## GPS API — `gps.php`

Updates a vehicle's position in `data/vehicles.json`.

### Request

```
POST /hardware-api/gps.php
Content-Type: application/json
```

```json
{
  "data": {
    "bus_id": 1,
    "latitude": 27.673159,
    "longitude": 85.343842,
    "speed": 28.5,
    "direction": 180,
    "altitude": 1208.1,
    "satellites": 7,
    "hdop": 2.1,
    "timestamp": "2026-03-15T09:06:53Z"
  }
}
```

| Field      | Type    | Required | Description                      |
| ---------- | ------- | -------- | -------------------------------- |
| bus_id     | integer | Yes      | Vehicle ID in vehicles.json      |
| latitude   | float   | Yes      | Latitude (-90 to 90)             |
| longitude  | float   | Yes      | Longitude (-180 to 180)          |
| speed      | float   | No       | Speed in km/h (defaults to 0)    |
| direction  | integer | No       | Heading in degrees 0-359         |
| altitude   | float   | No       | Altitude in meters (not stored)  |
| satellites | integer | No       | GPS satellite count (not stored) |
| hdop       | float   | No       | Horizontal dilution (not stored) |
| timestamp  | string  | No       | ISO 8601 timestamp (not stored)  |

### Response — Success (200)

```json
{
  "success": true,
  "vehicle_id": 1,
  "updated": {
    "lat": 27.673159,
    "lng": 85.343842,
    "speed": 28.5
  }
}
```

### Response — Errors

**400 Bad Request** — Missing or invalid fields:

```json
{ "error": "bus_id is required and must be a number" }
{ "error": "Valid latitude (-90 to 90) is required" }
{ "error": "Valid longitude (-180 to 180) is required" }
```

**404 Not Found** — Vehicle ID doesn't exist:

```json
{ "error": "Vehicle with id 99 not found" }
```

**405 Method Not Allowed** — Non-POST request:

```json
{ "error": "Method not allowed. Use POST." }
```

**500 Internal Server Error** — File system issue:

```json
{ "error": "Could not open vehicles data file" }
{ "error": "Corrupt vehicles data file" }
```

---

## Passenger Counter API — `passenger.php`

Sends a camera image to OpenRouter vision AI to count visible people, then updates the vehicle's `passengers` field in `data/vehicles.json`. Uses a model fallback chain — if the primary model times out or errors, it automatically tries the next model.

**Model fallback chain** (tried in order):

1. `google/gemini-2.0-flash-001` (30s timeout)
2. `google/gemini-flash-1.5` (30s timeout)
3. `meta-llama/llama-4-scout:free` (45s timeout)

### Request

```
POST /hardware-api/passenger.php
Content-Type: multipart/form-data
```

| Field      | Type    | Required | Description                          |
| ---------- | ------- | -------- | ------------------------------------ |
| vehicle_id | integer | Yes      | Vehicle ID in vehicles.json          |
| image      | file    | Yes      | JPEG, PNG, or WebP image (max 10 MB) |

### Response — Success (200)

```json
{
  "success": true,
  "vehicle_id": 1,
  "passengers": 12,
  "confidence": "high",
  "model": "google/gemini-2.0-flash-001",
  "attempts": [
    { "model": "google/gemini-2.0-flash-001", "time_ms": 2340, "status": "ok" }
  ]
}
```

**Success with fallback** (first model timed out, second succeeded):

```json
{
  "success": true,
  "vehicle_id": 1,
  "passengers": 8,
  "confidence": "medium",
  "model": "google/gemini-flash-1.5",
  "attempts": [
    { "model": "google/gemini-2.0-flash-001", "time_ms": 30012, "status": "curl_error", "error": "Timed out after 30s" },
    { "model": "google/gemini-flash-1.5", "time_ms": 1890, "status": "ok" }
  ]
}
```

| Field      | Type    | Description                                                  |
| ---------- | ------- | ------------------------------------------------------------ |
| vehicle_id | integer | The vehicle that was updated                                 |
| passengers | integer | Number of people detected in the image                       |
| confidence | string  | `high`, `medium`, or `low` — AI certainty level              |
| model      | string  | Which model produced the successful result                   |
| attempts   | array   | Each model attempt with model name, time in ms, and status   |

### Response — Errors

**400 Bad Request** — Missing or invalid input:

```json
{ "error": "vehicle_id is required and must be a number" }
{ "error": "Image file is required" }
{ "error": "Image must be JPEG, PNG, or WebP" }
{ "error": "Image must be under 10 MB" }
```

**404 Not Found** — Vehicle ID doesn't exist:

```json
{ "error": "Vehicle with id 99 not found" }
```

**405 Method Not Allowed** — Non-POST request:

```json
{ "error": "Method not allowed. Use POST." }
```

**500 Internal Server Error** — Config or file system issue:

```json
{ "error": "OPENROUTER_API_KEY not configured in .env" }
{ "error": "Could not open vehicles data file" }
```

**502 Bad Gateway** — All models failed:

```json
{
  "error": "All vision models failed to count passengers",
  "last_error": "[meta-llama/llama-4-scout:free] Timed out after 45s",
  "attempts": [
    { "model": "google/gemini-2.0-flash-001", "time_ms": 30004, "status": "curl_error", "error": "Timed out after 30s" },
    { "model": "google/gemini-flash-1.5", "time_ms": 30001, "status": "http_429", "error": "Rate limit exceeded" },
    { "model": "meta-llama/llama-4-scout:free", "time_ms": 45003, "status": "curl_error", "error": "Timed out after 45s" }
  ]
}
```

---

## Test Console — `test.php`

Open `http://localhost/sawari/hardware-api/test.php` in a browser to access the hardware simulator. It provides a web UI to manually send GPS fixes and camera images to both endpoints above.
