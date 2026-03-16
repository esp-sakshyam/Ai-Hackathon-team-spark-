<?php
// ============================================================
// Sawari - Server Diagnostics & Remote Endpoint Test
// Runs all pre-flight checks and optionally tests the actual
// production endpoint that the ESP32-CAM hardware hits.
//
// Usage: Open in browser — no POST needed.
//        Append ?test_upload=1 to also POST a real test image
//        to the production URL.
//
// Author: Zenith Kandel — https://zenithkandel.com.np
// License: MIT
// ============================================================

header('Content-Type: text/html; charset=utf-8');

$rootDir = dirname(__DIR__);

// The same URL the ESP32-CAM firmware uses
$productionUrl = 'https://zenithkandel.com.np/sawari/app/hardware-api/passenger.php';

// ─── Helper to print a check result ─────────────────────────────────────
function check(string $label, bool $pass, string $detail = ''): void
{
    $icon = $pass ? '✅' : '❌';
    echo "<tr><td>$icon</td><td><strong>$label</strong></td><td>" . htmlspecialchars($detail) . "</td></tr>\n";
}

function warn(string $label, string $detail = ''): void
{
    echo "<tr><td>⚠️</td><td><strong>$label</strong></td><td>" . htmlspecialchars($detail) . "</td></tr>\n";
}

function info(string $label, string $detail = ''): void
{
    echo "<tr><td>ℹ️</td><td><strong>$label</strong></td><td>" . htmlspecialchars($detail) . "</td></tr>\n";
}

?>
<!DOCTYPE html>
<html lang="en">

<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Sawari — Server Diagnostics</title>
    <style>
        body {
            font-family: 'Segoe UI', system-ui, sans-serif;
            background: #0d1117;
            color: #c9d1d9;
            padding: 20px;
            max-width: 900px;
            margin: 0 auto;
        }

        h1 {
            color: #58a6ff;
            margin-bottom: 4px;
        }

        h2 {
            color: #d29922;
            margin-top: 28px;
            border-bottom: 1px solid #30363d;
            padding-bottom: 6px;
        }

        .subtitle {
            color: #7d8590;
            margin-bottom: 20px;
        }

        table {
            width: 100%;
            border-collapse: collapse;
            margin-bottom: 16px;
        }

        td {
            padding: 8px 12px;
            border-bottom: 1px solid #21262d;
            vertical-align: top;
        }

        td:first-child {
            width: 30px;
            text-align: center;
        }

        td:nth-child(2) {
            width: 260px;
        }

        pre {
            background: #161b22;
            border: 1px solid #30363d;
            border-radius: 6px;
            padding: 14px;
            overflow-x: auto;
            font-size: 13px;
            white-space: pre-wrap;
            word-break: break-all;
        }

        .btn {
            display: inline-block;
            padding: 8px 18px;
            background: #238636;
            color: #fff;
            text-decoration: none;
            border-radius: 6px;
            font-weight: 600;
            margin-top: 12px;
        }

        .btn:hover {
            background: #2ea043;
        }

        .section {
            background: #161b22;
            border: 1px solid #30363d;
            border-radius: 8px;
            padding: 16px;
            margin-bottom: 20px;
        }
    </style>
</head>

<body>

    <h1>Sawari Server Diagnostics</h1>
    <p class="subtitle">Checking everything that could cause a 502 or failure when the ESP32-CAM uploads an image.</p>

    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <h2>1. PHP Environment</h2>
    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <div class="section">
        <table>
            <?php
            check('PHP Version', version_compare(PHP_VERSION, '8.0.0', '>='), PHP_VERSION . (version_compare(PHP_VERSION, '8.0.0', '<') ? ' — NEED 8.0+ for match/str_starts_with' : ''));
            check('curl extension', extension_loaded('curl'), extension_loaded('curl') ? curl_version()['version'] . ' / SSL: ' . curl_version()['ssl_version'] : 'NOT LOADED — API calls will fail');
            check('fileinfo extension', extension_loaded('fileinfo'), extension_loaded('fileinfo') ? 'Loaded' : 'NOT LOADED — MIME detection will fail');
            check('json extension', extension_loaded('json'), extension_loaded('json') ? 'Loaded' : 'NOT LOADED — everything will break');
            check('openssl extension', extension_loaded('openssl'), extension_loaded('openssl') ? OPENSSL_VERSION_TEXT : 'NOT LOADED — HTTPS cURL calls may fail');

            $memLimit = ini_get('memory_limit');
            $memBytes = (int) $memLimit;
            if (stripos($memLimit, 'M') !== false)
                $memBytes *= 1024 * 1024;
            elseif (stripos($memLimit, 'G') !== false)
                $memBytes *= 1024 * 1024 * 1024;
            check('memory_limit', $memBytes >= 64 * 1024 * 1024, "$memLimit" . ($memBytes < 64 * 1024 * 1024 ? ' — may OOM on large images (need ≥64M)' : ''));

            $maxExec = ini_get('max_execution_time');
            check('max_execution_time', (int) $maxExec === 0 || (int) $maxExec >= 120, "{$maxExec}s" . ((int) $maxExec > 0 && (int) $maxExec < 120 ? ' — too low for 3-model fallback chain (need ≥120s)' : ''));

            $uploadMax = ini_get('upload_max_filesize');
            $postMax = ini_get('post_max_size');
            check('upload_max_filesize', true, $uploadMax);
            check('post_max_size', true, $postMax);

            $inputTime = ini_get('max_input_time');
            check('max_input_time', (int) $inputTime === -1 || (int) $inputTime >= 60, "{$inputTime}s" . ((int) $inputTime > 0 && (int) $inputTime < 60 ? ' — may kill request before image finishes uploading' : ''));
            ?>
        </table>
    </div>

    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <h2>2. File System</h2>
    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <div class="section">
        <table>
            <?php
            $envFile = $rootDir . '/.env';
            check('.env file exists', file_exists($envFile), $envFile);
            if (file_exists($envFile)) {
                check('.env is readable', is_readable($envFile), 'Can read .env');
                $envLines = file($envFile, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
                $env = [];
                foreach ($envLines as $line) {
                    if (str_starts_with(trim($line), '#') || strpos($line, '=') === false)
                        continue;
                    [$k, $v] = explode('=', $line, 2);
                    $env[trim($k)] = trim($v);
                }
                $hasKey = !empty($env['OPENROUTER_API_KEY'] ?? '');
                check('OPENROUTER_API_KEY set', $hasKey, $hasKey ? 'Key is ' . strlen($env['OPENROUTER_API_KEY']) . ' chars (starts with ' . substr($env['OPENROUTER_API_KEY'], 0, 8) . '...)' : 'MISSING — all AI calls will fail');
            } else {
                warn('.env not found', "Expected at $envFile");
            }

            $vehiclesFile = $rootDir . '/app/data/vehicles.json';
            check('vehicles.json exists', file_exists($vehiclesFile), $vehiclesFile);
            if (file_exists($vehiclesFile)) {
                check('vehicles.json readable', is_readable($vehiclesFile), '');
                check('vehicles.json writable', is_writable($vehiclesFile), is_writable($vehiclesFile) ? '' : 'NOT WRITABLE — passenger updates will fail');
                $vData = json_decode(file_get_contents($vehiclesFile), true);
                check('vehicles.json valid JSON', is_array($vData), is_array($vData) ? count($vData) . ' vehicles found' : 'CORRUPT — json_decode failed: ' . json_last_error_msg());
                if (is_array($vData)) {
                    $hasVid1 = false;
                    foreach ($vData as $v) {
                        if (($v['id'] ?? null) === 1) {
                            $hasVid1 = true;
                            break;
                        }
                    }
                    check('Vehicle ID 1 exists', $hasVid1, $hasVid1 ? 'Default hardware vehicle_id=1 found' : 'vehicle_id=1 not found — default ESP32 config will get 404');
                }
            }

            $logDir = $rootDir . '/logs';
            if (is_dir($logDir)) {
                check('logs/ directory exists', true, $logDir);
                check('logs/ is writable', is_writable($logDir), is_writable($logDir) ? '' : 'NOT WRITABLE — logging will silently fail');
            } else {
                info('logs/ directory', "Doesn't exist yet — will be auto-created on first request");
            }

            $tmpDir = ini_get('upload_tmp_dir') ?: sys_get_temp_dir();
            check('PHP temp directory exists', is_dir($tmpDir), $tmpDir);
            check('PHP temp directory writable', is_writable($tmpDir), is_writable($tmpDir) ? '' : 'NOT WRITABLE — file uploads will fail');
            ?>
        </table>
    </div>

    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <h2>3. Outbound Connectivity (OpenRouter API)</h2>
    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <div class="section">
        <table>
            <?php
            // Test DNS resolution
            $dns = @dns_get_record('openrouter.ai', DNS_A);
            check('DNS resolves openrouter.ai', !empty($dns), !empty($dns) ? 'Resolved to ' . ($dns[0]['ip'] ?? '?') : 'DNS FAILED — server cannot reach external APIs');

            // Test HTTPS connection to OpenRouter (no API call, just a GET to check connectivity)
            if (extension_loaded('curl')) {
                $ch = curl_init('https://openrouter.ai/api/v1/models');
                curl_setopt_array($ch, [
                    CURLOPT_RETURNTRANSFER => true,
                    CURLOPT_TIMEOUT => 10,
                    CURLOPT_CONNECTTIMEOUT => 5,
                    CURLOPT_NOBODY => false,
                    CURLOPT_SSL_VERIFYPEER => true,
                ]);
                $resp = curl_exec($ch);
                $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
                $err = curl_error($ch);
                $errno = curl_errno($ch);
                $sslVerify = curl_getinfo($ch, CURLINFO_SSL_VERIFYRESULT);
                curl_close($ch);

                if ($err) {
                    $errMap = [
                        28 => 'Connection timed out — firewall may block outbound HTTPS',
                        6 => 'Could not resolve host — DNS issue',
                        7 => 'Connection refused — firewall/networking issue',
                        35 => 'SSL handshake failed',
                        60 => 'SSL certificate verification failed — missing CA bundle',
                    ];
                    check('HTTPS to openrouter.ai', false, $errMap[$errno] ?? "cURL error $errno: $err");
                } else {
                    check('HTTPS to openrouter.ai', $code === 200, "HTTP $code" . ($code === 200 ? ' — connection OK' : ''));
                    check('SSL certificate valid', $sslVerify === 0, $sslVerify === 0 ? 'Verified OK' : "SSL verify result: $sslVerify — may need CA bundle update");
                }

                // Test with API key if we have one
                if ($hasKey ?? false) {
                    $ch = curl_init('https://openrouter.ai/api/v1/auth/key');
                    curl_setopt_array($ch, [
                        CURLOPT_RETURNTRANSFER => true,
                        CURLOPT_TIMEOUT => 10,
                        CURLOPT_HTTPHEADER => [
                            'Authorization: Bearer ' . $env['OPENROUTER_API_KEY'],
                        ],
                    ]);
                    $resp = curl_exec($ch);
                    $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
                    curl_close($ch);
                    $keyData = json_decode($resp, true);
                    $keyValid = $code === 200 && isset($keyData['data']);
                    check('API key valid', $keyValid, $keyValid ? 'Key accepted by OpenRouter' : "HTTP $code — key may be expired or invalid");
                    if ($keyValid && isset($keyData['data']['limit'])) {
                        $limit = $keyData['data']['limit'];
                        $usage = $keyData['data']['usage'] ?? 0;
                        info('API credit usage', "Used: \$$usage / Limit: " . ($limit ? "\$$limit" : 'unlimited'));
                    }
                }
            } else {
                check('cURL test', false, 'curl extension not loaded — cannot test');
            }
            ?>
        </table>
    </div>

    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <h2>4. Server / Proxy Configuration</h2>
    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <div class="section">
        <table>
            <?php
            $sapi = php_sapi_name();
            info('PHP SAPI', $sapi);
            info('Server software', $_SERVER['SERVER_SOFTWARE'] ?? 'unknown');

            // Check if behind a proxy (common cause of 502)
            $behindProxy = !empty($_SERVER['HTTP_X_FORWARDED_FOR']) || !empty($_SERVER['HTTP_CF_RAY']);
            if ($behindProxy) {
                warn('Behind reverse proxy', 'Detected proxy headers — 502 may come from proxy timeout, not PHP');
                if (!empty($_SERVER['HTTP_CF_RAY'])) {
                    info('Cloudflare detected', 'CF-RAY: ' . $_SERVER['HTTP_CF_RAY'] . ' — Cloudflare has a 100s timeout on free plans');
                }
            }

            // Check passenger.php exists at expected path
            $passengerFile = __DIR__ . '/passenger.php';
            check('passenger.php exists', file_exists($passengerFile), $passengerFile);

            // Check if allow_url_fopen is on (needed for some operations)
            info('allow_url_fopen', ini_get('allow_url_fopen') ? 'On' : 'Off');

            // Check PHP error reporting
            info('display_errors', ini_get('display_errors') ? 'On' : 'Off — errors hidden from output');
            info('error_log', ini_get('error_log') ?: '(default)');

            // Check if output_buffering could cause issues
            info('output_buffering', ini_get('output_buffering') ?: 'Off');

            // Disk space
            $freeSpace = @disk_free_space($rootDir);
            if ($freeSpace !== false) {
                $freeSpaceMB = round($freeSpace / 1024 / 1024);
                check('Disk space', $freeSpaceMB > 50, "{$freeSpaceMB} MB free" . ($freeSpaceMB < 50 ? ' — critically low!' : ''));
            }
            ?>
        </table>
    </div>

    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <h2>5. Production Endpoint Test</h2>
    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <div class="section">
        <table>
            <?php
            info('Production URL', $productionUrl);

            // First, just check if the endpoint is reachable
            if (extension_loaded('curl')) {
                $ch = curl_init($productionUrl);
                curl_setopt_array($ch, [
                    CURLOPT_RETURNTRANSFER => true,
                    CURLOPT_TIMEOUT => 10,
                    CURLOPT_CONNECTTIMEOUT => 5,
                    // Send GET to see if we get 405 (which means PHP is running) vs 502/404
                    CURLOPT_HTTPHEADER => ['Content-Type: application/json'],
                ]);
                $resp = curl_exec($ch);
                $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
                $err = curl_error($ch);
                $totalTime = round(curl_getinfo($ch, CURLINFO_TOTAL_TIME) * 1000);
                curl_close($ch);

                if ($err) {
                    check('Endpoint reachable', false, "cURL error: $err");
                } else {
                    // 405 = Method not allowed (GET) = PHP is running correctly!
                    $reachable = in_array($code, [200, 405, 400]);
                    check('Endpoint reachable', $reachable, "HTTP $code in {$totalTime}ms" . ($code === 405 ? ' — PHP is responding correctly (405 for GET is expected)' : ($code === 502 ? ' — 502 Bad Gateway! Server/proxy issue' : '')));

                    if ($code === 502) {
                        warn('502 Bad Gateway', 'The production server returned 502 even on a simple GET. This means the issue is with the server/proxy configuration, NOT with the PHP code or API calls.');
                    }

                    $respData = json_decode($resp, true);
                    if ($respData) {
                        info('Response body', json_encode($respData, JSON_PRETTY_PRINT));
                    } else {
                        info('Response body', substr($resp, 0, 300));
                    }
                }
            }
            ?>
        </table>

        <?php if (isset($_GET['test_upload'])): ?>
            <!-- Full upload test with a tiny test image -->
            <h3 style="color: #d29922; margin-top: 20px;">Upload Test (POST with test image)</h3>
            <table>
                <?php
                // Create a minimal 1x1 red JPEG in memory
                $img = imagecreatetruecolor(10, 10);
                $red = imagecolorallocate($img, 255, 0, 0);
                imagefill($img, 0, 0, $red);
                ob_start();
                imagejpeg($img, null, 90);
                $jpegData = ob_get_clean();
                imagedestroy($img);

                $tmpFile = tempnam(sys_get_temp_dir(), 'sawari_test_');
                file_put_contents($tmpFile, $jpegData);

                info('Test image', strlen($jpegData) . ' bytes (10x10 red JPEG)');

                $ch = curl_init($productionUrl);
                $cfile = new CURLFile($tmpFile, 'image/jpeg', 'test.jpg');
                curl_setopt_array($ch, [
                    CURLOPT_RETURNTRANSFER => true,
                    CURLOPT_POST => true,
                    CURLOPT_TIMEOUT => 120,
                    CURLOPT_CONNECTTIMEOUT => 10,
                    CURLOPT_POSTFIELDS => [
                        'vehicle_id' => '1',
                        'image' => $cfile,
                    ],
                ]);

                $start = microtime(true);
                $resp = curl_exec($ch);
                $elapsed = round((microtime(true) - $start) * 1000);
                $code = curl_getinfo($ch, CURLINFO_HTTP_CODE);
                $err = curl_error($ch);
                $errno = curl_errno($ch);
                curl_close($ch);
                @unlink($tmpFile);

                if ($err) {
                    $errMap = [
                        28 => "Request timed out after 120s — server took too long (proxy timeout?)",
                        7 => 'Connection refused',
                        6 => 'DNS resolution failed',
                        35 => 'SSL handshake failed',
                    ];
                    check('Upload POST', false, ($errMap[$errno] ?? "cURL error $errno: $err") . " ({$elapsed}ms)");
                } else {
                    check('Upload POST', $code === 200, "HTTP $code in {$elapsed}ms");
                    $respData = json_decode($resp, true);
                    if ($respData) {
                        if ($code === 200 && ($respData['success'] ?? false)) {
                            check('AI counting worked', true, "Passengers: {$respData['passengers']}, Confidence: {$respData['confidence']}, Model: {$respData['model']}");
                        } else {
                            $errMsg = $respData['error'] ?? $respData['last_error'] ?? 'Unknown error';
                            check('Server response', false, $errMsg);
                            if (isset($respData['detail'])) {
                                warn('Error detail', $respData['detail']);
                            }
                            if (isset($respData['attempts'])) {
                                foreach ($respData['attempts'] as $a) {
                                    info("Model: {$a['model']}", "{$a['status']} — " . ($a['error'] ?? 'ok') . " ({$a['time_ms']}ms)");
                                }
                            }
                        }
                    } else {
                        warn('Non-JSON response', substr($resp, 0, 500));
                    }
                }
                ?>
            </table>
        <?php else: ?>
            <p style="margin-top: 16px;">
                <a href="?test_upload=1" class="btn">Run Full Upload Test (POST test image to production)</a>
                <br><small style="color: #7d8590; margin-top: 6px; display: inline-block;">This sends a tiny 10x10 test JPEG
                    to the production URL with vehicle_id=1 and measures the full round-trip.</small>
            </p>
        <?php endif; ?>

    </div>

    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <h2>6. Recent Log Entries</h2>
    <!-- ═══════════════════════════════════════════════════════════════════ -->
    <div class="section">
        <?php
        $logPath = $rootDir . '/logs/passenger-api.log';
        if (file_exists($logPath)) {
            $lines = file($logPath);
            $recent = array_slice($lines, -20);
            echo '<pre>' . htmlspecialchars(implode('', $recent)) . '</pre>';
        } else {
            echo '<p style="color: #7d8590;">No log file yet. Logs will appear after the first request to passenger.php.</p>';
        }
        ?>
    </div>

    <p style="color: #484f58; text-align: center; margin-top: 30px; font-size: 12px;">
        Sawari Diagnostics — Generated <?= date('Y-m-d H:i:s T') ?> — PHP <?= PHP_VERSION ?> on <?= php_uname('s') ?>
    </p>

</body>

</html>