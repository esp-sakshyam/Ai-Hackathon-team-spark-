<?php
// ============================================================
// Sawari - Hardware GPS API
// Receives GPS data from hardware devices and updates vehicle
// position in vehicles.json.
//
// Expected JSON payload:
// {
//     "data": {
//         "bus_id": 1,
//         "latitude": 27.673159,
//         "longitude": 85.343842,
//         "speed": 1.8,
//         "direction": 0,
//         "altitude": 1208.1,
//         "satellites": 7,
//         "hdop": 2,
//         "timestamp": "2026-02-19T09:06:53Z"
//     }
// }
//
// Author: Zenith Kandel — https://zenithkandel.com.np
// License: MIT
// ============================================================

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(204);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['error' => 'Method not allowed. Use POST.']);
    exit;
}

$vehiclesFile = dirname(__DIR__) . '/data/vehicles.json';

// Parse input
$input = json_decode(file_get_contents('php://input'), true);
if (!$input || !isset($input['data'])) {
    http_response_code(400);
    echo json_encode(['error' => 'Invalid JSON payload. Expected { "data": { ... } }']);
    exit;
}

$data = $input['data'];

// Validate required fields
$busId = $data['bus_id'] ?? null;
$lat = $data['latitude'] ?? null;
$lng = $data['longitude'] ?? null;
$speed = $data['speed'] ?? null;

if ($busId === null || !is_numeric($busId)) {
    http_response_code(400);
    echo json_encode(['error' => 'bus_id is required and must be a number']);
    exit;
}

if ($lat === null || !is_numeric($lat) || $lat < -90 || $lat > 90) {
    http_response_code(400);
    echo json_encode(['error' => 'Valid latitude (-90 to 90) is required']);
    exit;
}

if ($lng === null || !is_numeric($lng) || $lng < -180 || $lng > 180) {
    http_response_code(400);
    echo json_encode(['error' => 'Valid longitude (-180 to 180) is required']);
    exit;
}

$busId = (int)$busId;
$lat = (float)$lat;
$lng = (float)$lng;
$speed = ($speed !== null && is_numeric($speed)) ? (float)$speed : 0;

// Update vehicle in JSON file with file locking
$fp = fopen($vehiclesFile, 'c+');
if (!$fp) {
    http_response_code(500);
    echo json_encode(['error' => 'Could not open vehicles data file']);
    exit;
}

flock($fp, LOCK_EX);
$raw = stream_get_contents($fp);
$vehicles = json_decode($raw, true);

if (!is_array($vehicles)) {
    flock($fp, LOCK_UN);
    fclose($fp);
    http_response_code(500);
    echo json_encode(['error' => 'Corrupt vehicles data file']);
    exit;
}

$found = false;
foreach ($vehicles as &$v) {
    if (($v['id'] ?? null) === $busId) {
        $v['lat'] = $lat;
        $v['lng'] = $lng;
        $v['speed'] = $speed;
        $v['bearing'] = (int)($data['direction'] ?? $v['bearing'] ?? 0);
        $found = true;
        break;
    }
}
unset($v);

if (!$found) {
    flock($fp, LOCK_UN);
    fclose($fp);
    http_response_code(404);
    echo json_encode(['error' => "Vehicle with id $busId not found"]);
    exit;
}

fseek($fp, 0);
ftruncate($fp, 0);
fwrite($fp, json_encode($vehicles, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES));
flock($fp, LOCK_UN);
fclose($fp);

echo json_encode([
    'success' => true,
    'vehicle_id' => $busId,
    'updated' => [
        'lat' => $lat,
        'lng' => $lng,
        'speed' => $speed
    ]
]);
