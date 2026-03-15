<?php
// ============================================================
// Sawari - Hardware Passenger Counter API
// Receives a vehicle ID and an image from hardware, uses
// OpenRouter vision API to count people in the image, then
// updates the vehicle's passenger count in vehicles.json.
//
// Expected: multipart/form-data POST with:
//   - vehicle_id: integer (form field)
//   - image: image file (JPEG/PNG, max 10MB)
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

// Load .env for OpenRouter API key
$rootDir = dirname(__DIR__);
$envFile = $rootDir . '/.env';
$env = [];
if (file_exists($envFile)) {
    foreach (file($envFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES) as $line) {
        if (str_starts_with(trim($line), '#')) continue;
        if (strpos($line, '=') === false) continue;
        [$key, $value] = explode('=', $line, 2);
        $env[trim($key)] = trim($value);
    }
}

$openrouterKey = $env['OPENROUTER_API_KEY'] ?? '';
if (empty($openrouterKey)) {
    http_response_code(500);
    echo json_encode(['error' => 'OPENROUTER_API_KEY not configured in .env']);
    exit;
}

// Validate vehicle_id
$vehicleId = $_POST['vehicle_id'] ?? null;
if ($vehicleId === null || !is_numeric($vehicleId)) {
    http_response_code(400);
    echo json_encode(['error' => 'vehicle_id is required and must be a number']);
    exit;
}
$vehicleId = (int)$vehicleId;

// Validate image upload
if (!isset($_FILES['image']) || $_FILES['image']['error'] !== UPLOAD_ERR_OK) {
    http_response_code(400);
    echo json_encode(['error' => 'Image file is required']);
    exit;
}

$file = $_FILES['image'];
$allowedTypes = ['image/jpeg', 'image/png', 'image/webp'];
$finfo = finfo_open(FILEINFO_MIME_TYPE);
$mimeType = finfo_file($finfo, $file['tmp_name']);
finfo_close($finfo);

if (!in_array($mimeType, $allowedTypes)) {
    http_response_code(400);
    echo json_encode(['error' => 'Image must be JPEG, PNG, or WebP']);
    exit;
}

if ($file['size'] > 10 * 1024 * 1024) {
    http_response_code(400);
    echo json_encode(['error' => 'Image must be under 10 MB']);
    exit;
}

// Convert image to base64
$imageData = base64_encode(file_get_contents($file['tmp_name']));
$dataUrl = "data:$mimeType;base64,$imageData";

// Call OpenRouter vision API to count people
$prompt = 'Count the number of people visible in this image.

Respond with ONLY a raw JSON object — no markdown, no explanation, nothing else before or after the JSON:
{
  "count": <integer>,
  "confidence": "<high|medium|low>"
}

Rules:
- count: integer only. Count partial bodies (a visible head or torso) as 1 person.
- confidence: high = certain, medium = some occlusion, low = very crowded or blurry.';

$ch = curl_init('https://openrouter.ai/api/v1/chat/completions');
curl_setopt_array($ch, [
    CURLOPT_RETURNTRANSFER => true,
    CURLOPT_POST => true,
    CURLOPT_TIMEOUT => 30,
    CURLOPT_HTTPHEADER => [
        'Content-Type: application/json',
        'Authorization: Bearer ' . $openrouterKey,
        'HTTP-Referer: ' . ($_SERVER['HTTP_HOST'] ?? 'localhost'),
        'X-Title: Sawari Passenger Counter',
    ],
    CURLOPT_POSTFIELDS => json_encode([
        'model' => 'google/gemini-2.0-flash-001',
        'max_tokens' => 128,
        'temperature' => 0,
        'messages' => [
            [
                'role' => 'user',
                'content' => [
                    [
                        'type' => 'image_url',
                        'image_url' => ['url' => $dataUrl],
                    ],
                    [
                        'type' => 'text',
                        'text' => $prompt,
                    ],
                ],
            ],
        ],
    ]),
]);

$response = curl_exec($ch);
$httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
$curlError = curl_error($ch);
curl_close($ch);

if ($curlError) {
    http_response_code(500);
    echo json_encode(['error' => 'OpenRouter request failed: ' . $curlError]);
    exit;
}

if ($httpCode !== 200) {
    http_response_code(502);
    $errData = json_decode($response, true);
    $errMsg = $errData['error']['message'] ?? "OpenRouter API error (HTTP $httpCode)";
    echo json_encode(['error' => $errMsg]);
    exit;
}

$apiResult = json_decode($response, true);
$raw = trim($apiResult['choices'][0]['message']['content'] ?? '');

// Parse the count from the response
$cleaned = preg_replace('/^```[a-z]*\n?/i', '', $raw);
$cleaned = preg_replace('/```$/', '', $cleaned);
$cleaned = trim($cleaned);

$parsed = json_decode($cleaned, true);
if (json_last_error() !== JSON_ERROR_NONE) {
    // Fallback: try to extract JSON from prose
    if (preg_match('/\{[\s\S]*\}/', $cleaned, $m)) {
        $parsed = json_decode($m[0], true);
    }
}

if (!$parsed || !isset($parsed['count']) || !is_int($parsed['count'])) {
    http_response_code(502);
    echo json_encode(['error' => 'Could not parse people count from AI response', 'raw' => $raw]);
    exit;
}

$passengerCount = $parsed['count'];
$confidence = $parsed['confidence'] ?? 'unknown';

// Update vehicle passenger count in vehicles.json
$vehiclesFile = $rootDir . '/data/vehicles.json';
$fp = fopen($vehiclesFile, 'c+');
if (!$fp) {
    http_response_code(500);
    echo json_encode(['error' => 'Could not open vehicles data file']);
    exit;
}

flock($fp, LOCK_EX);
$rawData = stream_get_contents($fp);
$vehicles = json_decode($rawData, true);

if (!is_array($vehicles)) {
    flock($fp, LOCK_UN);
    fclose($fp);
    http_response_code(500);
    echo json_encode(['error' => 'Corrupt vehicles data file']);
    exit;
}

$found = false;
foreach ($vehicles as &$v) {
    if (($v['id'] ?? null) === $vehicleId) {
        $v['passengers'] = $passengerCount;
        $found = true;
        break;
    }
}
unset($v);

if (!$found) {
    flock($fp, LOCK_UN);
    fclose($fp);
    http_response_code(404);
    echo json_encode(['error' => "Vehicle with id $vehicleId not found"]);
    exit;
}

fseek($fp, 0);
ftruncate($fp, 0);
fwrite($fp, json_encode($vehicles, JSON_PRETTY_PRINT | JSON_UNESCAPED_UNICODE | JSON_UNESCAPED_SLASHES));
flock($fp, LOCK_UN);
fclose($fp);

echo json_encode([
    'success' => true,
    'vehicle_id' => $vehicleId,
    'passengers' => $passengerCount,
    'confidence' => $confidence
]);
