/*
 * WebPages.cpp
 * 
 * HTML page generation for web interface.
 */

#include "WebPages.h"

// ==================================================
// HTML PAGE GENERATION
// ==================================================
String getIndexHTML() {
  return R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Hydroponics Controller</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
        }

        .container {
            max-width: 1200px;
            margin: 0 auto;
        }

        .header {
            text-align: center;
            color: white;
            margin-bottom: 30px;
        }

        .header h1 {
            font-size: 2.5em;
            margin-bottom: 10px;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
        }

        .status-badge {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 0.9em;
            margin: 5px;
        }

        .badge-online { background: #10b981; color: white; }
        .badge-offline { background: #ef4444; color: white; }

        .card {
            background: white;
            border-radius: 15px;
            padding: 25px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
        }

        .card h2 {
            color: #667eea;
            margin-bottom: 20px;
            font-size: 1.5em;
            border-bottom: 2px solid #667eea;
            padding-bottom: 10px;
        }

        .data-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(250px, 1fr));
            gap: 20px;
            margin-top: 20px;
        }

        .data-item {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
            border-left: 4px solid #667eea;
        }

        .data-label {
            color: #6c757d;
            font-size: 0.9em;
            margin-bottom: 5px;
        }

        .data-value {
            font-size: 1.8em;
            color: #212529;
            font-weight: bold;
        }

        .data-unit {
            font-size: 0.8em;
            color: #6c757d;
            margin-left: 5px;
        }

        .form-group {
            margin-bottom: 20px;
        }

        .form-group label {
            display: block;
            margin-bottom: 8px;
            color: #495057;
            font-weight: 500;
        }

        .form-group input,
        .form-group select {
            width: 100%;
            padding: 12px;
            border: 2px solid #e9ecef;
            border-radius: 8px;
            font-size: 1em;
            transition: border-color 0.3s;
        }

        .form-group input:focus,
        .form-group select:focus {
            outline: none;
            border-color: #667eea;
        }

        .form-group input[type="checkbox"] {
            width: auto;
            margin-right: 10px;
        }

        .btn {
            padding: 12px 30px;
            border: none;
            border-radius: 8px;
            font-size: 1em;
            cursor: pointer;
            transition: all 0.3s;
            font-weight: 500;
        }

        .btn-primary {
            background: #667eea;
            color: white;
        }

        .btn-primary:hover {
            background: #5568d3;
            transform: translateY(-2px);
            box-shadow: 0 5px 15px rgba(102, 126, 234, 0.4);
        }

        .btn-secondary {
            background: #6c757d;
            color: white;
            margin-left: 10px;
        }

        .btn-secondary:hover {
            background: #5a6268;
        }

        .btn-danger {
            background: #ef4444;
            color: white;
        }

        .btn-danger:hover {
            background: #dc2626;
        }

        .tabs {
            display: flex;
            margin-bottom: 20px;
            border-bottom: 2px solid #e9ecef;
        }

        .tab {
            padding: 15px 30px;
            cursor: pointer;
            border: none;
            background: none;
            font-size: 1em;
            color: #6c757d;
            transition: all 0.3s;
        }

        .tab.active {
            color: #667eea;
            border-bottom: 3px solid #667eea;
            font-weight: 600;
        }

        .tab-content {
            display: none;
        }

        .tab-content.active {
            display: block;
        }

        .alert {
            padding: 15px;
            border-radius: 8px;
            margin-bottom: 20px;
        }

        .alert-success {
            background: #d1fae5;
            color: #065f46;
            border: 1px solid #10b981;
        }

        .alert-error {
            background: #fee2e2;
            color: #991b1b;
            border: 1px solid #ef4444;
        }

        @media (max-width: 768px) {
            .header h1 {
                font-size: 1.8em;
            }

            .data-grid {
                grid-template-columns: 1fr;
            }

            .btn {
                width: 100%;
                margin: 5px 0;
            }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌱 Hydroponics Controller</h1>
            <div id="statusBadges"></div>
        </div>

        <div class="card">
            <h2>📊 Live Monitoring</h2>
            <div class="data-grid" id="dataGrid"></div>
        </div>

        <div class="card">
            <div class="tabs">
                <button class="tab active" onclick="switchTab('config')">⚙️ Configuration</button>
                <button class="tab" onclick="switchTab('logs')">📝 Logs</button>
                <button class="tab" onclick="switchTab('system')">🔧 System</button>
            </div>

            <div id="configTab" class="tab-content active">
                <div id="configAlert"></div>
                
                <!-- WiFi Configuration Section -->
                <h3 style="color: #667eea; margin-bottom: 15px;">📡 WiFi Configuration</h3>
                <form id="wifiForm" style="margin-bottom: 30px; padding-bottom: 30px; border-bottom: 2px solid #e9ecef;">
                    <div class="form-group">
                        <label>WiFi SSID</label>
                        <input type="text" id="wifiSSID" placeholder="Enter your WiFi network name" required>
                    </div>

                    <div class="form-group">
                        <label>WiFi Password</label>
                        <input type="password" id="wifiPassword" placeholder="Enter WiFi password (8-63 characters)">
                    </div>

                    <div id="wifiStatus" style="margin-bottom: 15px;"></div>

                    <button type="submit" class="btn btn-primary">💾 Save WiFi & Reboot</button>
                </form>

                <!-- MQTT Configuration Section -->
                <h3 style="color: #667eea; margin-bottom: 15px;">📬 MQTT Configuration</h3>
                <form id="configForm">
                    <div class="form-group">
                        <label>AP Password</label>
                        <input type="text" id="apPassword" required>
                    </div>

                    <div class="form-group">
                        <label>MQTT Broker</label>
                        <input type="text" id="mqttBroker" required>
                    </div>

                    <div class="form-group">
                        <label>MQTT Port</label>
                        <input type="number" id="mqttPort" required>
                    </div>

                    <div class="form-group">
                        <label>MQTT Username (optional)</label>
                        <input type="text" id="mqttUser">
                    </div>

                    <div class="form-group">
                        <label>MQTT Password (optional)</label>
                        <input type="password" id="mqttPass">
                    </div>

                    <div class="form-group">
                        <label>MQTT Publish Topic</label>
                        <input type="text" id="mqttTopic" required>
                    </div>

                    <div class="form-group">
                        <label>MQTT Subscribe Topic 1</label>
                        <input type="text" id="mqttSubTopic1" placeholder="Optional subscribe topic">
                    </div>

                    <div class="form-group">
                        <label>MQTT Subscribe Topic 2</label>
                        <input type="text" id="mqttSubTopic2" placeholder="Optional subscribe topic">
                    </div>

                    <div class="form-group">
                        <label>MQTT Subscribe Topic 3</label>
                        <input type="text" id="mqttSubTopic3" placeholder="Optional subscribe topic">
                    </div>

                    <div class="form-group">
                        <label>Publish Interval (ms)</label>
                        <input type="number" id="publishInterval" required>
                    </div>

                    <div class="form-group">
                        <label>
                            <input type="checkbox" id="enableLogging">
                            Enable Data Logging
                        </label>
                    </div>

                    <button type="submit" class="btn btn-primary">💾 Save Configuration</button>
                    <button type="button" class="btn btn-secondary" onclick="loadConfig()">🔄 Reload</button>
                </form>
            </div>

            <div id="logsTab" class="tab-content">
                <p>Download sensor data logs in CSV format.</p>
                <button class="btn btn-primary" onclick="downloadLogs()">⬇️ Download Logs</button>
                <button class="btn btn-danger" onclick="clearLogs()">🗑️ Clear Logs</button>
            </div>

            <div id="systemTab" class="tab-content">
                <p>System maintenance and diagnostics.</p>
                <button class="btn btn-primary" onclick="syncNTP()">🕒 Sync NTP Time</button>
                <button class="btn btn-danger" onclick="rebootDevice()">🔄 Reboot Device</button>
            </div>
        </div>
    </div>

    <script>
        let ws;
        let wsReconnectInterval;

        function initWebSocket() {
            ws = new WebSocket('ws://' + window.location.hostname + '/ws');

            ws.onopen = function() {
                console.log('WebSocket connected');
                clearInterval(wsReconnectInterval);
            };

            ws.onmessage = function(event) {
                const data = JSON.parse(event.data);
                updateDashboard(data);
            };

            ws.onclose = function() {
                console.log('WebSocket disconnected');
                wsReconnectInterval = setInterval(initWebSocket, 5000);
            };

            ws.onerror = function(error) {
                console.error('WebSocket error:', error);
            };
        }

        function updateDashboard(data) {
            const statusHTML = `
                <span class="status-badge ${data.wifi ? 'badge-online' : 'badge-offline'}">
                    WiFi: ${data.wifi ? 'Connected' : 'Disconnected'}
                </span>
                <span class="status-badge ${data.mqtt ? 'badge-online' : 'badge-offline'}">
                    MQTT: ${data.mqtt ? 'Connected' : 'Disconnected'}
                </span>
                <span class="status-badge ${data.ntpSynced ? 'badge-online' : 'badge-offline'}">
                    NTP: ${data.ntpSynced ? 'Synced' : 'Not Synced'}
                </span>
            `;
            document.getElementById('statusBadges').innerHTML = statusHTML;

            const dataHTML = `
                <div class="data-item">
                    <div class="data-label">Temperature</div>
                    <div class="data-value">${data.temperature.toFixed(1)}<span class="data-unit">°C</span></div>
                </div>
                <div class="data-item">
                    <div class="data-label">Time</div>
                    <div class="data-value" style="font-size: 1.2em;">${data.timestamp.split(' ')[1]}</div>
                </div>
                <div class="data-item">
                    <div class="data-label">Date</div>
                    <div class="data-value" style="font-size: 1.2em;">${data.timestamp.split(' ')[0]}</div>
                </div>
                <div class="data-item">
                    <div class="data-label">LittleFS Storage</div>
                    <div class="data-value">${data.spiffs.used}<span class="data-unit">/ ${data.spiffs.total} KB</span></div>
                </div>
                <div class="data-item">
                    <div class="data-label">IP Address</div>
                    <div class="data-value" style="font-size: 1.2em;">${data.ip}</div>
                </div>
                <div class="data-item">
                    <div class="data-label">Uptime</div>
                    <div class="data-value">${formatUptime(data.uptime)}</div>
                </div>
            `;
            document.getElementById('dataGrid').innerHTML = dataHTML;
        }

        function formatUptime(seconds) {
            const days = Math.floor(seconds / 86400);
            const hours = Math.floor((seconds % 86400) / 3600);
            const minutes = Math.floor((seconds % 3600) / 60);

            if (days > 0) return `${days}d ${hours}h`;
            if (hours > 0) return `${hours}h ${minutes}m`;
            return `${minutes}m`;
        }

        function switchTab(tabName) {
            document.querySelectorAll('.tab').forEach(tab => tab.classList.remove('active'));
            document.querySelectorAll('.tab-content').forEach(content => content.classList.remove('active'));

            event.target.classList.add('active');
            document.getElementById(tabName + 'Tab').classList.add('active');
        }

        function loadConfig() {
            // Load WiFi status
            fetch('/api/wifi/status')
                .then(response => response.json())
                .then(data => {
                    const statusHTML = `
                        <div class="alert ${data.apMode ? 'alert-error' : 'alert-success'}">
                            <strong>Current Status:</strong> ${data.status}<br>
                            <strong>IP Address:</strong> ${data.ip}<br>
                            ${data.apMode ? '<em>⚠️ Device is in AP mode. Configure WiFi credentials above to connect to your network.</em>' : ''}
                        </div>
                    `;
                    document.getElementById('wifiStatus').innerHTML = statusHTML;
                })
                .catch(error => console.error('Error loading WiFi status:', error));

            // Load MQTT config
            fetch('/api/config')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('apPassword').value = data.apPassword;
                    document.getElementById('mqttBroker').value = data.mqttBroker;
                    document.getElementById('mqttPort').value = data.mqttPort;
                    document.getElementById('mqttUser').value = data.mqttUser;
                    document.getElementById('mqttPass').value = data.mqttPass;
                    document.getElementById('mqttTopic').value = data.mqttTopic;
                    document.getElementById('mqttSubTopic1').value = data.mqttSubTopic1 || '';
                    document.getElementById('mqttSubTopic2').value = data.mqttSubTopic2 || '';
                    document.getElementById('mqttSubTopic3').value = data.mqttSubTopic3 || '';
                    document.getElementById('publishInterval').value = data.publishInterval;
                    document.getElementById('enableLogging').checked = data.enableLogging;
                })
                .catch(error => {
                    showAlert('configAlert', 'Error loading configuration', 'error');
                });
        }

        document.getElementById('wifiForm').addEventListener('submit', function(e) {
            e.preventDefault();

            const ssid = document.getElementById('wifiSSID').value;
            const password = document.getElementById('wifiPassword').value;

            if (password && (password.length < 8 || password.length > 63)) {
                alert('WiFi password must be 8-63 characters or empty for open network');
                return;
            }

            const formData = new FormData();
            formData.append('ssid', ssid);
            formData.append('password', password);

            fetch('/api/wifi', {
                method: 'POST',
                body: formData
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    alert('WiFi credentials saved! Device will reboot and connect to "' + ssid + '". Please reconnect to your network and access the device at its new IP address.');
                } else {
                    alert('Error: ' + data.message);
                }
            })
            .catch(error => {
                alert('Error saving WiFi credentials: ' + error);
            });
        });

        document.getElementById('configForm').addEventListener('submit', function(e) {
            e.preventDefault();

            const config = {
                apPassword: document.getElementById('apPassword').value,
                mqttBroker: document.getElementById('mqttBroker').value,
                mqttPort: parseInt(document.getElementById('mqttPort').value),
                mqttUser: document.getElementById('mqttUser').value,
                mqttPass: document.getElementById('mqttPass').value,
                mqttTopic: document.getElementById('mqttTopic').value,
                mqttSubTopic1: document.getElementById('mqttSubTopic1').value,
                mqttSubTopic2: document.getElementById('mqttSubTopic2').value,
                mqttSubTopic3: document.getElementById('mqttSubTopic3').value,
                publishInterval: parseInt(document.getElementById('publishInterval').value),
                enableLogging: document.getElementById('enableLogging').checked
            };

            fetch('/api/config', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(config)
            })
            .then(response => response.json())
            .then(data => {
                if (data.success) {
                    showAlert('configAlert', 'Configuration saved successfully!', 'success');
                } else {
                    showAlert('configAlert', 'Error: ' + data.message, 'error');
                }
            })
            .catch(error => {
                showAlert('configAlert', 'Error saving configuration', 'error');
            });
        });

        function downloadLogs() {
            window.location.href = '/api/logs';
        }

        function clearLogs() {
            if (confirm('Are you sure you want to clear all logs?')) {
                fetch('/api/logs/clear', { method: 'POST' })
                    .then(response => response.json())
                    .then(data => {
                        alert(data.message);
                    });
            }
        }

        function syncNTP() {
            fetch('/api/sync-ntp', { method: 'POST' })
                .then(response => response.json())
                .then(data => {
                    alert(data.message);
                });
        }

        function rebootDevice() {
            if (confirm('Are you sure you want to reboot the device?')) {
                fetch('/api/reboot', { method: 'POST' })
                    .then(response => response.json())
                    .then(data => {
                        alert('Device is rebooting... Please wait 30 seconds and refresh the page.');
                    });
            }
        }

        function showAlert(elementId, message, type) {
            const alertDiv = document.getElementById(elementId);
            alertDiv.innerHTML = `<div class="alert alert-${type}">${message}</div>`;
            setTimeout(() => {
                alertDiv.innerHTML = '';
            }, 5000);
        }

        initWebSocket();
        loadConfig();

        setInterval(() => {
            if (!ws || ws.readyState !== WebSocket.OPEN) {
                fetch('/api/data')
                    .then(response => response.json())
                    .then(data => updateDashboard(data));
            }
        }, 2000);
    </script>
</body>
</html>
)rawliteral";
}