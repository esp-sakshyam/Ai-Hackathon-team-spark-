<?php
// ============================================================
// Sawari - Hardware Simulator
// Test page for GPS and Passenger Counter hardware APIs
// Author: Zenith Kandel — https://zenithkandel.com.np
// License: MIT
// ============================================================

// Load vehicles for the dropdown
$vehiclesFile = dirname(__DIR__) . '/data/vehicles.json';
$vehicles = json_decode(file_get_contents($vehiclesFile), true) ?: [];
?>
<!DOCTYPE html>
<html lang="en">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Sawari Hardware Simulator</title>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.1/css/all.min.css">
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }

    body {
      font-family: 'Segoe UI', system-ui, -apple-system, sans-serif;
      background: #0d1117;
      color: #c9d1d9;
      min-height: 100vh;
    }

    header {
      background: #161b22;
      border-bottom: 1px solid #30363d;
      padding: 16px 24px;
      display: flex;
      align-items: center;
      gap: 12px;
    }

    header i {
      color: #58a6ff;
      font-size: 20px;
    }

    header h1 {
      font-size: 18px;
      font-weight: 600;
      color: #e6edf3;
    }

    header span {
      font-size: 12px;
      color: #7d8590;
      margin-left: 8px;
    }

    .container {
      max-width: 1100px;
      margin: 24px auto;
      padding: 0 24px;
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 20px;
    }

    .card {
      background: #161b22;
      border: 1px solid #30363d;
      border-radius: 8px;
      overflow: hidden;
    }

    .card-header {
      padding: 14px 18px;
      border-bottom: 1px solid #30363d;
      display: flex;
      align-items: center;
      gap: 10px;
      font-weight: 600;
      font-size: 14px;
    }

    .card-header i {
      font-size: 16px;
    }

    .card-header .gps {
      color: #3fb950;
    }

    .card-header .cam {
      color: #d29922;
    }

    .card-body {
      padding: 18px;
    }

    .field {
      margin-bottom: 14px;
    }

    .field label {
      display: block;
      font-size: 12px;
      font-weight: 500;
      color: #7d8590;
      margin-bottom: 5px;
      text-transform: uppercase;
      letter-spacing: 0.5px;
    }

    .field select,
    .field input {
      width: 100%;
      padding: 8px 12px;
      background: #0d1117;
      border: 1px solid #30363d;
      border-radius: 6px;
      color: #c9d1d9;
      font-size: 14px;
      font-family: inherit;
      outline: none;
      transition: border-color 0.15s;
    }

    .field select:focus,
    .field input:focus {
      border-color: #58a6ff;
    }

    .field-row {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 10px;
    }

    .btn {
      padding: 8px 18px;
      border: none;
      border-radius: 6px;
      font-size: 13px;
      font-weight: 600;
      font-family: inherit;
      cursor: pointer;
      display: inline-flex;
      align-items: center;
      gap: 8px;
      transition: all 0.15s;
    }

    .btn:disabled {
      opacity: 0.5;
      cursor: not-allowed;
    }

    .btn-green {
      background: #238636;
      color: #fff;
    }

    .btn-green:hover:not(:disabled) {
      background: #2ea043;
    }

    .btn-amber {
      background: #9e6a03;
      color: #fff;
    }

    .btn-amber:hover:not(:disabled) {
      background: #bb8009;
    }

    .btn-blue {
      background: #1f6feb;
      color: #fff;
    }

    .btn-blue:hover:not(:disabled) {
      background: #388bfd;
    }

    .btn-ghost {
      background: transparent;
      color: #7d8590;
      border: 1px solid #30363d;
    }

    .btn-ghost:hover:not(:disabled) {
      color: #c9d1d9;
      border-color: #484f58;
    }

    .actions {
      display: flex;
      gap: 8px;
      margin-top: 16px;
    }

    .log-panel {
      grid-column: 1 / -1;
    }

    .log-header {
      display: flex;
      align-items: center;
      justify-content: space-between;
    }

    .log-body {
      max-height: 350px;
      overflow-y: auto;
      font-family: 'Cascadia Code', 'Fira Code', 'Consolas', monospace;
      font-size: 12px;
      line-height: 1.7;
    }

    .log-entry {
      padding: 6px 18px;
      border-bottom: 1px solid #21262d;
      display: flex;
      gap: 10px;
    }

    .log-entry:last-child {
      border-bottom: none;
    }

    .log-time {
      color: #484f58;
      flex-shrink: 0;
    }

    .log-tag {
      padding: 1px 6px;
      border-radius: 3px;
      font-size: 10px;
      font-weight: 600;
      text-transform: uppercase;
      flex-shrink: 0;
    }

    .log-tag.gps {
      background: #0f2d1a;
      color: #3fb950;
    }

    .log-tag.cam {
      background: #2d1f00;
      color: #d29922;
    }

    .log-tag.err {
      background: #3d0a0a;
      color: #f85149;
    }

    .log-tag.info {
      background: #0a1929;
      color: #58a6ff;
    }

    .log-msg {
      word-break: break-all;
    }

    .log-msg.ok {
      color: #3fb950;
    }

    .log-msg.fail {
      color: #f85149;
    }

    .log-empty {
      padding: 30px;
      text-align: center;
      color: #484f58;
      font-family: inherit;
    }

    .sim-status {
      display: flex;
      align-items: center;
      gap: 8px;
      margin-top: 10px;
      font-size: 12px;
      color: #7d8590;
    }

    .sim-dot {
      width: 8px;
      height: 8px;
      border-radius: 50%;
      background: #484f58;
    }

    .sim-dot.active {
      background: #3fb950;
      animation: pulse 1.5s ease-in-out infinite;
    }

    @keyframes pulse {

      0%,
      100% {
        opacity: 1;
      }

      50% {
        opacity: 0.3;
      }
    }

    .drop-zone {
      border: 2px dashed #30363d;
      border-radius: 8px;
      padding: 28px;
      text-align: center;
      cursor: pointer;
      transition: all 0.2s;
      color: #7d8590;
      font-size: 13px;
    }

    .drop-zone:hover,
    .drop-zone.drag-over {
      border-color: #d29922;
      background: rgba(210, 153, 34, 0.05);
      color: #d29922;
    }

    .drop-zone i {
      font-size: 28px;
      display: block;
      margin-bottom: 8px;
    }

    .preview-img {
      max-width: 100%;
      max-height: 180px;
      border-radius: 6px;
      margin-top: 10px;
      display: none;
      border: 1px solid #30363d;
    }

    .vehicle-badge {
      display: inline-flex;
      align-items: center;
      gap: 6px;
      padding: 4px 10px;
      background: #0d1117;
      border: 1px solid #30363d;
      border-radius: 4px;
      font-size: 12px;
      color: #c9d1d9;
    }

    @media (max-width: 700px) {
      .container {
        grid-template-columns: 1fr;
      }

      .log-panel {
        grid-column: 1;
      }
    }
  </style>
</head>

<body>
  <header>
    <i class="fa-solid fa-microchip"></i>
    <h1>Hardware Simulator</h1>
    <span>Sawari IoT Test Console</span>
  </header>

  <div class="container">

    <!-- GPS Simulator -->
    <div class="card">
      <div class="card-header">
        <i class="fa-solid fa-satellite gps"></i>
        GPS Module
      </div>
      <div class="card-body">
        <div class="field">
          <label>Vehicle</label>
          <select id="gps-vehicle">
            <?php foreach ($vehicles as $v): ?>
              <option value="<?= $v['id'] ?>" data-lat="<?= $v['lat'] ?>" data-lng="<?= $v['lng'] ?>">
                #<?= $v['id'] ?> — <?= htmlspecialchars($v['name']) ?>
              </option>
            <?php endforeach; ?>
          </select>
        </div>
        <div class="field-row">
          <div class="field">
            <label>Latitude</label>
            <input type="number" id="gps-lat" step="0.000001" value="27.7172">
          </div>
          <div class="field">
            <label>Longitude</label>
            <input type="number" id="gps-lng" step="0.000001" value="85.3240">
          </div>
        </div>
        <div class="field-row">
          <div class="field">
            <label>Speed (km/h)</label>
            <input type="number" id="gps-speed" step="0.1" value="28" min="0">
          </div>
          <div class="field">
            <label>Direction (°)</label>
            <input type="number" id="gps-dir" step="1" value="0" min="0" max="359">
          </div>
        </div>
        <div class="actions">
          <button class="btn btn-green" id="gps-send">
            <i class="fa-solid fa-paper-plane"></i> Send GPS Fix
          </button>
          <button class="btn btn-blue" id="gps-auto">
            <i class="fa-solid fa-play"></i> Auto Simulate
          </button>
          <button class="btn btn-ghost" id="gps-randomize">
            <i class="fa-solid fa-shuffle"></i> Randomize
          </button>
        </div>
        <div class="sim-status" id="gps-sim-status" style="display:none">
          <span class="sim-dot active"></span>
          <span>Sending GPS fix every <strong>3s</strong>… <span id="gps-sim-count">0</span> sent</span>
        </div>
      </div>
    </div>

    <!-- Passenger Counter Simulator -->
    <div class="card">
      <div class="card-header">
        <i class="fa-solid fa-camera cam"></i>
        Passenger Camera
      </div>
      <div class="card-body">
        <div class="field">
          <label>Vehicle</label>
          <select id="cam-vehicle">
            <?php foreach ($vehicles as $v): ?>
              <option value="<?= $v['id'] ?>">
                #<?= $v['id'] ?> — <?= htmlspecialchars($v['name']) ?> (current: <?= $v['passengers'] ?? '?' ?> pax)
              </option>
            <?php endforeach; ?>
          </select>
        </div>
        <div class="field">
          <label>Camera Image</label>
          <div class="drop-zone" id="drop-zone">
            <i class="fa-solid fa-cloud-arrow-up"></i>
            Drop an image here, or click to browse
          </div>
          <input type="file" id="file-input" accept="image/*" style="display:none">
          <img class="preview-img" id="preview-img">
        </div>
        <div class="actions">
          <button class="btn btn-amber" id="cam-send" disabled>
            <i class="fa-solid fa-eye"></i> Count &amp; Update
          </button>
        </div>
      </div>
    </div>

    <!-- Log Panel -->
    <div class="card log-panel">
      <div class="card-header">
        <div class="log-header" style="width:100;display:flex;justify-content:space-between">
          <div style="display:flex;align-items:center;gap:10px">
            <i class="fa-solid fa-terminal" style="color:#58a6ff;"></i>
            Console Log
          </div>
          <button class="btn btn-ghost" id="log-clear" style="padding:4px 10px;font-size:11px;">Clear</button>
        </div>
      </div>
      <div class="card-body" style="padding:0;">
        <div class="log-body" id="log-body">
          <div class="log-empty">No activity yet. Send a GPS fix or camera image to see results.</div>
        </div>
      </div>
    </div>

  </div>

  <script>
    (function () {
      'use strict';

      const logBody = document.getElementById('log-body');
      let autoInterval = null;
      let autoCount = 0;
      let selectedFile = null;

      // --- Logging ---

      function log(tag, msg, ok) {
        const empty = logBody.querySelector('.log-empty');
        if (empty) empty.remove();

        const entry = document.createElement('div');
        entry.className = 'log-entry';

        const now = new Date().toLocaleTimeString('en-GB');
        entry.innerHTML = `
        <span class="log-time">${now}</span>
        <span class="log-tag ${tag}">${tag}</span>
        <span class="log-msg ${ok === true ? 'ok' : ok === false ? 'fail' : ''}">${escapeHtml(msg)}</span>
      `;
        logBody.appendChild(entry);
        logBody.scrollTop = logBody.scrollHeight;
      }

      document.getElementById('log-clear').addEventListener('click', () => {
        logBody.innerHTML = '<div class="log-empty">Console cleared.</div>';
      });

      function escapeHtml(str) {
        const d = document.createElement('div');
        d.textContent = str;
        return d.innerHTML;
      }

      // --- GPS Vehicle Selector: auto-fill lat/lng ---

      const gpsVehicle = document.getElementById('gps-vehicle');
      const gpsLat = document.getElementById('gps-lat');
      const gpsLng = document.getElementById('gps-lng');

      gpsVehicle.addEventListener('change', function () {
        const opt = this.options[this.selectedIndex];
        gpsLat.value = parseFloat(opt.dataset.lat).toFixed(6);
        gpsLng.value = parseFloat(opt.dataset.lng).toFixed(6);
      });
      // Init first vehicle
      gpsVehicle.dispatchEvent(new Event('change'));

      // --- GPS: Send Single Fix ---

      document.getElementById('gps-send').addEventListener('click', () => sendGpsFix());

      async function sendGpsFix() {
        const busId = parseInt(gpsVehicle.value);
        const lat = parseFloat(gpsLat.value);
        const lng = parseFloat(gpsLng.value);
        const speed = parseFloat(document.getElementById('gps-speed').value) || 0;
        const direction = parseInt(document.getElementById('gps-dir').value) || 0;

        const payload = {
          data: {
            bus_id: busId,
            latitude: lat,
            longitude: lng,
            speed: speed,
            direction: direction,
            altitude: 1300 + Math.random() * 50,
            satellites: Math.floor(5 + Math.random() * 8),
            hdop: +(1 + Math.random() * 2).toFixed(1),
            timestamp: new Date().toISOString()
          }
        };

        log('gps', `POST bus_id=${busId} lat=${lat.toFixed(6)} lng=${lng.toFixed(6)} speed=${speed}`);

        try {
          const res = await fetch('gps.php', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
          });
          const data = await res.json();
          if (res.ok) {
            log('gps', `✓ Vehicle #${busId} updated → lat=${data.updated.lat}, lng=${data.updated.lng}, speed=${data.updated.speed}`, true);
          } else {
            log('err', `✗ ${data.error}`, false);
          }
        } catch (err) {
          log('err', `✗ Network error: ${err.message}`, false);
        }
      }

      // --- GPS: Randomize ---

      document.getElementById('gps-randomize').addEventListener('click', () => {
        // Random point within Kathmandu valley
        gpsLat.value = (27.65 + Math.random() * 0.12).toFixed(6);
        gpsLng.value = (85.28 + Math.random() * 0.10).toFixed(6);
        document.getElementById('gps-speed').value = (5 + Math.random() * 45).toFixed(1);
        document.getElementById('gps-dir').value = Math.floor(Math.random() * 360);
        log('info', 'Randomized GPS values for Kathmandu valley');
      });

      // --- GPS: Auto Simulate ---

      const autoBtn = document.getElementById('gps-auto');
      const simStatus = document.getElementById('gps-sim-status');
      const simCount = document.getElementById('gps-sim-count');

      autoBtn.addEventListener('click', () => {
        if (autoInterval) {
          clearInterval(autoInterval);
          autoInterval = null;
          autoBtn.innerHTML = '<i class="fa-solid fa-play"></i> Auto Simulate';
          simStatus.style.display = 'none';
          log('info', `Auto simulation stopped after ${autoCount} fixes`);
          autoCount = 0;
          return;
        }

        autoCount = 0;
        autoBtn.innerHTML = '<i class="fa-solid fa-stop"></i> Stop';
        simStatus.style.display = 'flex';
        log('info', 'Auto simulation started — moving vehicle with random drift every 3s');

        autoInterval = setInterval(() => {
          // Drift the position slightly (simulates movement)
          const latDrift = (Math.random() - 0.5) * 0.002;
          const lngDrift = (Math.random() - 0.5) * 0.002;
          gpsLat.value = (parseFloat(gpsLat.value) + latDrift).toFixed(6);
          gpsLng.value = (parseFloat(gpsLng.value) + lngDrift).toFixed(6);
          document.getElementById('gps-speed').value = (10 + Math.random() * 35).toFixed(1);
          document.getElementById('gps-dir').value = Math.floor(Math.random() * 360);

          autoCount++;
          simCount.textContent = autoCount;
          sendGpsFix();
        }, 3000);
      });

      // --- Camera: Drop Zone ---

      const dropZone = document.getElementById('drop-zone');
      const fileInput = document.getElementById('file-input');
      const previewImg = document.getElementById('preview-img');
      const camSend = document.getElementById('cam-send');

      dropZone.addEventListener('click', () => fileInput.click());
      dropZone.addEventListener('dragover', (e) => { e.preventDefault(); dropZone.classList.add('drag-over'); });
      dropZone.addEventListener('dragleave', () => dropZone.classList.remove('drag-over'));
      dropZone.addEventListener('drop', (e) => {
        e.preventDefault();
        dropZone.classList.remove('drag-over');
        if (e.dataTransfer.files[0]) loadImage(e.dataTransfer.files[0]);
      });
      fileInput.addEventListener('change', () => {
        if (fileInput.files[0]) loadImage(fileInput.files[0]);
      });

      function loadImage(file) {
        if (!file.type.startsWith('image/')) {
          log('err', 'Selected file is not an image', false);
          return;
        }
        if (file.size > 10 * 1024 * 1024) {
          log('err', 'Image must be under 10 MB', false);
          return;
        }
        selectedFile = file;
        const reader = new FileReader();
        reader.onload = (e) => {
          previewImg.src = e.target.result;
          previewImg.style.display = 'block';
          camSend.disabled = false;
          dropZone.innerHTML = '<i class="fa-solid fa-check"></i> Image loaded — ' + file.name;
        };
        reader.readAsDataURL(file);
        log('cam', `Image loaded: ${file.name} (${(file.size / 1024).toFixed(1)} KB)`);
      }

      // --- Camera: Count & Update ---

      camSend.addEventListener('click', async () => {
        if (!selectedFile) return;

        const vehicleId = parseInt(document.getElementById('cam-vehicle').value);
        camSend.disabled = true;
        camSend.innerHTML = '<i class="fa-solid fa-spinner fa-spin"></i> Analysing…';
        log('cam', `Sending image to passenger counter for vehicle #${vehicleId}…`);

        const formData = new FormData();
        formData.append('vehicle_id', vehicleId);
        formData.append('image', selectedFile);

        try {
          const res = await fetch('https://zenithkandel.com.np/sawari/app/hardware-api/passenger.php', {
            method: 'POST',
            body: formData
          });
          const data = await res.json();
          if (res.ok) {
            log('cam', `✓ Vehicle #${vehicleId} → ${data.passengers} passengers (confidence: ${data.confidence}, model: ${data.model})`, true);
            if (data.attempts && data.attempts.length > 1) {
              const failedAttempts = data.attempts.filter(a => a.status !== 'ok');
              failedAttempts.forEach(a => log('info', `↳ ${a.model} failed: ${a.error} (${a.time_ms}ms)`));
              log('info', `↳ Succeeded on attempt ${data.attempts.length}/${data.attempts.length} after ${data.attempts.reduce((s, a) => s + a.time_ms, 0)}ms total`);
            } else if (data.attempts) {
              log('info', `↳ First model succeeded in ${data.attempts[0].time_ms}ms`);
            }
          } else {
            log('err', `✗ ${data.error || data.last_error || 'Unknown error'}`, false);
            if (data.attempts) {
              data.attempts.forEach(a => log('info', `↳ ${a.model}: ${a.status} — ${a.error || 'ok'} (${a.time_ms}ms)`));
            }
            if (data.raw) log('info', `Raw AI response: ${data.raw}`);
          }
        } catch (err) {
          log('err', `✗ Network error: ${err.message}`, false);
        } finally {
          camSend.disabled = false;
          camSend.innerHTML = '<i class="fa-solid fa-eye"></i> Count & Update';
        }
      });

    })();
  </script>
</body>

</html>