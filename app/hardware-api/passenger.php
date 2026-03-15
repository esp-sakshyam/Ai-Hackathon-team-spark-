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

// Log errors to a file for debugging hardware issues
$logDir = dirname(__DIR__) . '/logs';
if (!is_dir($logDir)) {
    @mkdir($logDir, 0755, true);
}
$logFile = $logDir . '/passenger-api.log';

function logMsg(string $msg): void
{
    global $logFile;
    $ts = date('Y-m-d H:i:s');
    @file_put_contents($logFile, "[$ts] $msg\n", FILE_APPEND | LOCK_EX);
}

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
        if (str_starts_with(trim($line), '#'))
            continue;
        if (strpos($line, '=') === false)
            continue;
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
$vehicleId = (int) $vehicleId;

logMsg("Request received: vehicle_id=$vehicleId, image_size=" . ($_FILES['image']['size'] ?? 'N/A') . " bytes");
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

// Call OpenRouter vision API to count people — with model fallback chain
$prompt = 'Count the number of people visible in this image.

Respond with ONLY a raw JSON object — no markdown, no explanation, nothing else before or after the JSON:
{
  "count": <integer>,
  "confidence": "<high|medium|low>"
}

Rules:
- count: integer only. Count partial bodies (a visible head or torso) as 1 person.
- confidence: high = certain, medium = some occlusion, low = very crowded or blurry.';

// Fallback model chain: try each model in order until one succeeds
$models = [
    ['id' => 'google/gemini-2.0-flash-001', 'timeout' => 30],
    ['id' => 'google/gemini-flash-1.5', 'timeout' => 30],
    ['id' => 'meta-llama/llama-4-scout:free', 'timeout' => 45],
];

$lastError = null;
$attempts = [];
$raw = null;
$parsed = null;

foreach ($models as $modelInfo) {
    $modelId = $modelInfo['id'];
    $timeout = $modelInfo['timeout'];
    $attemptStart = microtime(true);

    $ch = curl_init('https://openrouter.ai/api/v1/chat/completions');
    curl_setopt_array($ch, [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_POST => true,
        CURLOPT_TIMEOUT => $timeout,
        CURLOPT_CONNECTTIMEOUT => 10,
        CURLOPT_HTTPHEADER => [
            'Content-Type: application/json',
            'Authorization: Bearer ' . $openrouterKey,
            'HTTP-Referer: ' . ($_SERVER['HTTP_HOST'] ?? 'localhost'),
            'X-Title: Sawari Passenger Counter',
        ],
        CURLOPT_POSTFIELDS => json_encode([
            'model' => $modelId,
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
    $curlErrno = curl_errno($ch);
    $totalTime = round((microtime(true) - $attemptStart) * 1000);
    curl_close($ch);

    $attempt = [
        'model' => $modelId,
        'time_ms' => $totalTime,
    ];

    // Curl-level failure (timeout, DNS, connection refused, etc.)
    if ($curlError) {
        $readableError = match ($curlErrno) {
            CURLE_OPERATION_TIMEDOUT, 28
            => "Timed out after {$timeout}s",
            CURLE_COULDNT_CONNECT
            => 'Connection refused',
            CURLE_COULDNT_RESOLVE_HOST
            => 'DNS resolution failed',
            CURLE_SSL_CONNECT_ERROR
            => 'SSL handshake failed',
            default
            => $curlError,
        };
        $attempt['status'] = 'curl_error';
        $attempt['error'] = $readableError;
        $attempts[] = $attempt;
        $lastError = "[$modelId] $readableError";
        continue; // try next model
    }

    // HTTP-level error (rate limit, server error, etc.)
    if ($httpCode !== 200) {
        $errData = json_decode($response, true);
        $errMsg = $errData['error']['message'] ?? "HTTP $httpCode";
        $attempt['status'] = "http_$httpCode";
        $attempt['error'] = $errMsg;
        $attempts[] = $attempt;
        $lastError = "[$modelId] $errMsg";
        continue; // try next model
    }

    // Parse AI response
    $apiResult = json_decode($response, true);
    $raw = trim($apiResult['choices'][0]['message']['content'] ?? '');

    $cleaned = preg_replace('/^```[a-z]*\n?/i', '', $raw);
    $cleaned = preg_replace('/```$/', '', $cleaned);
    $cleaned = trim($cleaned);

    $parsed = json_decode($cleaned, true);
    if (json_last_error() !== JSON_ERROR_NONE) {
        // Fallback: try to extract JSON from prose
        if (preg_match('/\{[\s\S]*?\}/', $cleaned, $m)) {
            $parsed = json_decode($m[0], true);
        }
    }

    if (!$parsed || !isset($parsed['count']) || !is_numeric($parsed['count'])) {
        $attempt['status'] = 'parse_error';
        $attempt['error'] = 'Could not extract count from response';
        $attempt['raw'] = $raw;
        $attempts[] = $attempt;
        $lastError = "[$modelId] Unparseable response";
        continue; // try next model
    }

    // Success — this model worked
    $attempt['status'] = 'ok';
    $attempts[] = $attempt;
    break;
}

// All models failed
if (!$parsed || !isset($parsed['count']) || !is_numeric($parsed['count'])) {
    logMsg("FAIL: All models failed for vehicle_id=$vehicleId. Last error: $lastError");
    http_response_code(502);
    echo json_encode([
        'error' => 'All vision models failed to count passengers',
        'last_error' => $lastError,
        'attempts' => $attempts,
    ], JSON_PRETTY_PRINT);
    exit;
}

$passengerCount = (int) $parsed['count'];
$confidence = $parsed['confidence'] ?? 'unknown';
$usedModel = $attempts[count($attempts) - 1]['model'];

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
        $v['passenger_updated_at'] = date('c'); // ISO 8601 timestamp
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

logMsg("OK: vehicle_id=$vehicleId, passengers=$passengerCount, confidence=$confidence, model=$usedModel");

echo json_encode([
    'success' => true,
    'vehicle_id' => $vehicleId,
    'passengers' => $passengerCount,
    'confidence' => $confidence,
    'model' => $usedModel,
    'attempts' => $attempts,
]);
