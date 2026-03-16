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

// ─── Catch fatal errors (memory exhaustion, etc.) and return JSON ──────────
ob_start();

header('Content-Type: application/json');
header('Access-Control-Allow-Origin: *');
header('Access-Control-Allow-Methods: POST, OPTIONS');
header('Access-Control-Allow-Headers: Content-Type');

// ─── Logging ──────────────────────────────────────────────────────────────
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

// ─── Global error & shutdown handlers — ensure JSON response on any crash ─
set_error_handler(function ($severity, $message, $file, $line) {
    logMsg("PHP Error [$severity]: $message in $file:$line");
    return false;
});

register_shutdown_function(function () {
    $error = error_get_last();
    if ($error && in_array($error['type'], [E_ERROR, E_CORE_ERROR, E_COMPILE_ERROR, E_PARSE])) {
        ob_end_clean();
        http_response_code(500);
        $msg = $error['message'] . ' in ' . $error['file'] . ':' . $error['line'];
        logMsg("FATAL: $msg");
        echo json_encode([
            'error' => 'Internal server error',
            'detail' => $msg,
        ]);
    }
});

if ($_SERVER['REQUEST_METHOD'] === 'OPTIONS') {
    http_response_code(204);
    exit;
}

if ($_SERVER['REQUEST_METHOD'] !== 'POST') {
    http_response_code(405);
    echo json_encode(['error' => 'Method not allowed. Use POST.']);
    exit;
}

// ─── Runtime limits ───────────────────────────────────────────────────────
set_time_limit(180);
ini_set('memory_limit', '128M');

// ─── Pre-flight: check required PHP extensions ────────────────────────────
$missing = [];
if (!extension_loaded('curl'))
    $missing[] = 'curl';
if (!extension_loaded('fileinfo'))
    $missing[] = 'fileinfo';
if (!extension_loaded('json'))
    $missing[] = 'json';
if (!empty($missing)) {
    http_response_code(500);
    $msg = 'Missing PHP extensions: ' . implode(', ', $missing);
    logMsg($msg);
    echo json_encode(['error' => $msg]);
    exit;
}

// ─── Load .env for OpenRouter API key ─────────────────────────────────────
// Search upward from this script's directory to find .env
$envFile = null;
$searchDir = dirname(__DIR__);
$checkedPaths = [];
for ($i = 0; $i < 5; $i++) {
    $candidate = $searchDir . '/.env';
    $checkedPaths[] = $candidate;
    if (file_exists($candidate)) {
        $envFile = $candidate;
        break;
    }
    $parent = dirname($searchDir);
    if ($parent === $searchDir) break; // reached filesystem root
    $searchDir = $parent;
}

$env = [];
if ($envFile) {
    logMsg("Found .env at $envFile");
    foreach (file($envFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES) as $line) {
        if (str_starts_with(trim($line), '#'))
            continue;
        if (strpos($line, '=') === false)
            continue;
        [$key, $value] = explode('=', $line, 2);
        $env[trim($key)] = trim($value);
    }
} else {
    logMsg("WARN: .env file not found. Checked: " . implode(', ', $checkedPaths));
}

$rootDir = $envFile ? dirname($envFile) : dirname(__DIR__);

$openrouterKey = $env['OPENROUTER_API_KEY'] ?? '';
if (empty($openrouterKey)) {
    http_response_code(500);
    logMsg('ERROR: OPENROUTER_API_KEY not configured');
    echo json_encode([
        'error' => 'OPENROUTER_API_KEY not configured in .env',
        'searched' => $checkedPaths,
    ]);
    exit;
}

// ─── Validate vehicle_id ──────────────────────────────────────────────────
$vehicleId = $_POST['vehicle_id'] ?? null;
if ($vehicleId === null || !is_numeric($vehicleId)) {
    http_response_code(400);
    echo json_encode(['error' => 'vehicle_id is required and must be a number']);
    exit;
}
$vehicleId = (int) $vehicleId;

logMsg("Request received: vehicle_id=$vehicleId, image_size=" . ($_FILES['image']['size'] ?? 'N/A') . " bytes");

// ─── Validate image upload ───────────────────────────────────────────────
if (!isset($_FILES['image'])) {
    http_response_code(400);
    logMsg("ERROR: No image file in request");
    echo json_encode(['error' => 'Image file is required (no "image" field in upload)']);
    exit;
}

$uploadErr = $_FILES['image']['error'];
if ($uploadErr !== UPLOAD_ERR_OK) {
    $uploadErrors = [
        UPLOAD_ERR_INI_SIZE => 'File exceeds server upload_max_filesize (' . ini_get('upload_max_filesize') . ')',
        UPLOAD_ERR_FORM_SIZE => 'File exceeds form MAX_FILE_SIZE',
        UPLOAD_ERR_PARTIAL => 'File was only partially uploaded',
        UPLOAD_ERR_NO_FILE => 'No file was uploaded',
        UPLOAD_ERR_NO_TMP_DIR => 'Server missing temp directory',
        UPLOAD_ERR_CANT_WRITE => 'Server failed to write file to disk',
        UPLOAD_ERR_EXTENSION => 'A PHP extension stopped the upload',
    ];
    $errMsg = $uploadErrors[$uploadErr] ?? "Unknown upload error code: $uploadErr";
    http_response_code(400);
    logMsg("Upload error: $errMsg");
    echo json_encode(['error' => "Image upload failed: $errMsg"]);
    exit;
}

$file = $_FILES['image'];

if (!file_exists($file['tmp_name']) || !is_readable($file['tmp_name'])) {
    http_response_code(500);
    logMsg("ERROR: Temp file missing or unreadable: " . $file['tmp_name']);
    echo json_encode(['error' => 'Uploaded temp file is missing or unreadable']);
    exit;
}

$finfo = finfo_open(FILEINFO_MIME_TYPE);
$mimeType = finfo_file($finfo, $file['tmp_name']);
finfo_close($finfo);

$allowedTypes = ['image/jpeg', 'image/png', 'image/webp'];
if (!in_array($mimeType, $allowedTypes)) {
    http_response_code(400);
    echo json_encode(['error' => "Image must be JPEG, PNG, or WebP (got: $mimeType)"]);
    exit;
}

if ($file['size'] > 10 * 1024 * 1024) {
    http_response_code(400);
    echo json_encode(['error' => 'Image must be under 10 MB']);
    exit;
}

// ─── Convert image to base64 ─────────────────────────────────────────────
$imageRaw = file_get_contents($file['tmp_name']);
if ($imageRaw === false) {
    http_response_code(500);
    logMsg("ERROR: Failed to read uploaded image from " . $file['tmp_name']);
    echo json_encode(['error' => 'Failed to read uploaded image']);
    exit;
}

$imageData = base64_encode($imageRaw);
unset($imageRaw); // Free raw image memory
$dataUrl = "data:$mimeType;base64,$imageData";

// ─── OpenRouter vision API — model fallback chain ─────────────────────────
$prompt = 'Count the number of people visible in this image.

Respond with ONLY a raw JSON object — no markdown, no explanation, nothing else before or after the JSON:
{
  "count": <integer>,
  "confidence": "<high|medium|low>"
}

Rules:
- count: integer only. Count partial bodies (a visible head or torso) as 1 person.
- confidence: high = certain, medium = some occlusion, low = very crowded or blurry.';

$models = [
    ['id' => 'google/gemini-2.0-flash-001', 'timeout' => 30],
    ['id' => 'google/gemini-flash-1.5', 'timeout' => 30],
    ['id' => 'meta-llama/llama-4-scout:free', 'timeout' => 45],
];

$lastError = null;
$attempts = [];
$raw = null;
$parsed = null;

// Build request body once (reuse across models to save memory)
$requestBody = json_encode([
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
]);

// Free base64 data now that it's encoded in request body
unset($imageData, $dataUrl);

if ($requestBody === false) {
    http_response_code(500);
    logMsg("ERROR: json_encode failed: " . json_last_error_msg());
    echo json_encode(['error' => 'Failed to encode API request: ' . json_last_error_msg()]);
    exit;
}

foreach ($models as $modelInfo) {
    $modelId = $modelInfo['id'];
    $timeout = $modelInfo['timeout'];
    $attemptStart = microtime(true);

    // Inject model into the pre-built request body
    $bodyWithModel = json_decode($requestBody, true);
    $bodyWithModel['model'] = $modelId;
    $encodedBody = json_encode($bodyWithModel);

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
        CURLOPT_POSTFIELDS => $encodedBody,
        CURLOPT_SSL_VERIFYPEER => true,
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

    // Curl-level failure
    if ($curlError) {
        $readableError = match ($curlErrno) {
            CURLE_OPERATION_TIMEDOUT, 28
            => "Timed out after {$timeout}s",
            CURLE_COULDNT_CONNECT
            => 'Connection refused',
            CURLE_COULDNT_RESOLVE_HOST
            => 'DNS resolution failed',
            CURLE_SSL_CONNECT_ERROR, 35
            => 'SSL handshake failed',
            60 // CURLE_SSL_CACERT
            => 'SSL certificate verification failed — server may lack CA bundle',
            default
            => "cURL error $curlErrno: $curlError",
        };
        $attempt['status'] = 'curl_error';
        $attempt['error'] = $readableError;
        $attempts[] = $attempt;
        $lastError = "[$modelId] $readableError";
        logMsg("Model $modelId curl error: $readableError");
        continue;
    }

    // HTTP-level error
    if ($httpCode !== 200) {
        $errData = json_decode($response, true);
        $errMsg = $errData['error']['message'] ?? "HTTP $httpCode";
        $attempt['status'] = "http_$httpCode";
        $attempt['error'] = $errMsg;
        $attempts[] = $attempt;
        $lastError = "[$modelId] $errMsg";
        logMsg("Model $modelId HTTP $httpCode: $errMsg");
        continue;
    }

    // Parse AI response
    $apiResult = json_decode($response, true);
    $raw = trim($apiResult['choices'][0]['message']['content'] ?? '');

    $cleaned = preg_replace('/^```[a-z]*\n?/i', '', $raw);
    $cleaned = preg_replace('/```$/', '', $cleaned);
    $cleaned = trim($cleaned);

    $parsed = json_decode($cleaned, true);
    if (json_last_error() !== JSON_ERROR_NONE) {
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
        logMsg("Model $modelId parse error. Raw: $raw");
        continue;
    }

    // Success
    $attempt['status'] = 'ok';
    $attempts[] = $attempt;
    logMsg("Model $modelId succeeded in {$totalTime}ms");
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

// ─── Update vehicle passenger count in vehicles.json ──────────────────────
$vehiclesFile = $rootDir . '/data/vehicles.json';

if (!file_exists($vehiclesFile)) {
    logMsg("ERROR: vehicles.json not found at $vehiclesFile");
    http_response_code(500);
    echo json_encode(['error' => 'Vehicles data file not found', 'path_checked' => $vehiclesFile]);
    exit;
}

if (!is_writable($vehiclesFile)) {
    logMsg("ERROR: vehicles.json not writable at $vehiclesFile");
    http_response_code(500);
    echo json_encode(['error' => 'Vehicles data file is not writable (check permissions)']);
    exit;
}

$fp = fopen($vehiclesFile, 'c+');
if (!$fp) {
    http_response_code(500);
    logMsg("ERROR: fopen failed on $vehiclesFile");
    echo json_encode(['error' => 'Could not open vehicles data file']);
    exit;
}

flock($fp, LOCK_EX);
$rawData = stream_get_contents($fp);
$vehicles = json_decode($rawData, true);

if (!is_array($vehicles)) {
    flock($fp, LOCK_UN);
    fclose($fp);
    logMsg("ERROR: vehicles.json corrupt or empty. First 200 chars: " . substr($rawData, 0, 200));
    http_response_code(500);
    echo json_encode(['error' => 'Corrupt vehicles data file']);
    exit;
}

$found = false;
foreach ($vehicles as &$v) {
    if (($v['id'] ?? null) === $vehicleId) {
        $v['passengers'] = $passengerCount;
        $v['passenger_updated_at'] = date('c');
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
