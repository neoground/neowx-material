// 1. INIT PAHO CLIENTxxx
// Configuration is injected from index.html.tmpl via window.MQTT_CONFIG
// Only initialize if MQTT is enabled

var client = null;
var options = null;

// Wrap initialization to ensure DOM and dependencies are ready
(function initMQTT() {
    // Check if MQTT is enabled before initialization
    if (window.MQTT_CONFIG && window.MQTT_CONFIG.enabled) {
        debugLog('MQTT enabled - initializing connection');

        client = new Paho.Client(
            window.MQTT_CONFIG.host,
            window.MQTT_CONFIG.port,
            window.MQTT_CONFIG.websocket_path,
            "myclientid_" + parseInt(Math.random() * 100, 10));

        options = {
            useSSL: window.MQTT_CONFIG.use_ssl,
            userName: window.MQTT_CONFIG.username,
            password: window.MQTT_CONFIG.password,
            reconnect: true,
            onSuccess: function () {
                console.log("Connected to MQTT Broker");
                updateMQTTStatusIndicator(true);
                client.subscribe(window.MQTT_CONFIG.topic);
            },
            onFailure: function (e) {
                console.log("MQTT Connection Failed", e);
                updateMQTTStatusIndicator(false);
            }
        };

        client.connect(options);
    } else {
        debugLog('MQTT disabled - skipping connection');
        updateMQTTStatusIndicator(null);  // null means disabled
    }
})();

// --- HELPER FUNCTION: Update MQTT status indicator ---
function updateMQTTStatusIndicator(connected) {
    console.log('[MQTT INDICATOR] Function called with:', connected);
    var indicator = document.getElementById('mqtt-indicator');

    if (!indicator) {
        console.log('[MQTT INDICATOR] Element NOT found!');
        return;
    }

    console.log('[MQTT INDICATOR] Element found, applying styles...');

    if (connected === true) {
        // Connected - Green (using !important to override CSS inheritance)
        indicator.style.cssText = 'color: #00ff00 !important; vertical-align: middle;';
        console.log('[MQTT INDICATOR] Set color to GREEN');
        if (window.MQTT_CONFIG && window.MQTT_CONFIG.text_connected) {
            indicator.setAttribute('title', window.MQTT_CONFIG.text_connected);
            indicator.setAttribute('data-original-title', window.MQTT_CONFIG.text_connected);
            // Initialize/update Bootstrap tooltip
            if (typeof $ !== 'undefined' && $.fn.tooltip) {
                $(indicator).tooltip('dispose').tooltip();
            }
            console.log('[MQTT INDICATOR] Set tooltip:', window.MQTT_CONFIG.text_connected);
        }
    } else if (connected === false) {
        // Failed - Red (using !important to override CSS inheritance)
        indicator.style.cssText = 'color: #ff4444 !important; vertical-align: middle;';
        console.log('[MQTT INDICATOR] Set color to RED');
        if (window.MQTT_CONFIG && window.MQTT_CONFIG.text_failed) {
            indicator.setAttribute('title', window.MQTT_CONFIG.text_failed);
            indicator.setAttribute('data-original-title', window.MQTT_CONFIG.text_failed);
            // Initialize/update Bootstrap tooltip
            if (typeof $ !== 'undefined' && $.fn.tooltip) {
                $(indicator).tooltip('dispose').tooltip();
            }
            console.log('[MQTT INDICATOR] Set tooltip:', window.MQTT_CONFIG.text_failed);
        }
    } else if (connected === null) {
        // Disabled - hide status
        console.log('[MQTT INDICATOR] Hiding indicator (MQTT disabled)');
        var statusContainer = document.getElementById('mqtt-status');
        if (statusContainer) {
            statusContainer.style.display = 'none';
        }
    }
}

// 2. Global Helper Functions
function getCompass(deg) {
    if (deg === undefined || deg === null || isNaN(deg)) return "";
    var val = Math.floor((deg / 22.5) + 0.5);
    var arr = ["N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"];
    return arr[(val % 16)];
}

// 3. MQTT MESSAGE HANDLER
// Only set up handler if client was created (i.e., MQTT is enabled)
if (client) {
    client.onMessageArrived = function (message) {
        try {
            var payload = JSON.parse(message.payloadString);
            debugLog('Message received: ' + JSON.stringify(payload).substring(0, 100) + '...');

            // Check if timestamp is newer, if yes - update page
            if (isTimestampNewer(payload)) {
                debugLog('Timestamp is newer, updating values');
                updateDateTime(payload);
                updatePayloadValues(payload);
                updateTelemetry(payload);
            }

        } catch (e) {
            console.log("❌ Sync Error:", e);
        }
    };
}

// --- HELPER FUNCTION 1: Check if payload timestamp is newer ---
function isTimestampNewer(payload) {
    if (payload.dateTime === undefined) {
        debugLog('No Timestamp in payload, skipping timestamp check');
        return true;
    }

    var timeSpan = document.getElementById('current-datetime');
    if (!timeSpan) return false;

    var currentUnixTime = parseInt(timeSpan.getAttribute('data-timestamp'), 10);
    var payloadUnixTime = parseFloat(payload.dateTime);

    debugLog('Timestamp comparison - Server: ' + currentUnixTime + ', Payload: ' + payloadUnixTime);

    if (payloadUnixTime > currentUnixTime) {
        debugLog('✓ Payload timestamp is newer');
        return true;
    } else {
        debugLog('⊗ Payload timestamp is not newer, skipping update');
        return false;
    }
}

// --- HELPER FUNCTION 2: Update datetime on page ---
function updateDateTime(payload) {
    var timeSpan = document.getElementById('current-datetime');
    if (!timeSpan) return;

    var payloadUnixTime = parseFloat(payload.dateTime);
    var payloadDate = new Date(payloadUnixTime * 1000);

    // Initialize DATETIME_CONFIG if not present (safety check)
    if (typeof window.DATETIME_CONFIG === 'undefined') {
        window.DATETIME_CONFIG = {
            initial_value: timeSpan.textContent.trim(),
            initial_format: null
        };
    }

    // Detect format from initial value if not already detected
    if (!window.DATETIME_CONFIG.initial_format) {
        var initialValue = window.DATETIME_CONFIG.initial_value || timeSpan.textContent.trim();
        window.DATETIME_CONFIG.initial_format = detectDateTimeFormat(initialValue);
        debugLog('Detected datetime format: ' + window.DATETIME_CONFIG.initial_format);
    }

    // Format the new datetime using the detected format
    var newDateTime = formatDateTime(payloadDate, window.DATETIME_CONFIG.initial_format);

    timeSpan.innerHTML = newDateTime;
    timeSpan.setAttribute('data-timestamp', payloadUnixTime);

    debugLog('✓ Updated datetime to: ' + newDateTime);

    // Visual feedback - check if flash is enabled
    if (window.MQTT_CONFIG && window.MQTT_CONFIG.flash_on_update !== false) {
        // Get flash color from configuration (default to green if not set)
        var flashColor = (window.MQTT_CONFIG && window.MQTT_CONFIG.flash_color)
            ? window.MQTT_CONFIG.flash_color
            : "#00ff00";

        timeSpan.style.transition = "color 0.5s, text-shadow 0.5s";
        timeSpan.style.color = flashColor;
        timeSpan.style.textShadow = "0 0 8px " + flashColor;
        setTimeout(function () {
            timeSpan.style.color = "";
            timeSpan.style.textShadow = "none";
        }, 1500);
    }
}

// --- HELPER FUNCTION: Detect datetime format from a sample value ---
function detectDateTimeFormat(sampleValue) {
    if (!sampleValue) return 'YYYY-MM-DD HH:mm:ss';

    // Common datetime patterns to detect
    var patterns = [
        // 12-hour formats with AM/PM (check these first)
        {regex: /^\d{2}\/\d{2}\/\d{4}\s+\d{1,2}:\d{2}:\d{2}\s+(?:AM|PM)$/i, format: 'MM/DD/YYYY hh:mm:ss A'},
        {regex: /^\d{2}\/\d{2}\/\d{4}\s+\d{1,2}:\d{2}\s+(?:AM|PM)$/i, format: 'MM/DD/YYYY hh:mm A'},
        {regex: /^\d{4}-\d{2}-\d{2}\s+\d{1,2}:\d{2}:\d{2}\s+(?:AM|PM)$/i, format: 'YYYY-MM-DD hh:mm:ss A'},
        {regex: /^\d{4}-\d{2}-\d{2}\s+\d{1,2}:\d{2}\s+(?:AM|PM)$/i, format: 'YYYY-MM-DD hh:mm A'},
        {regex: /^\d{2}\.\d{2}\.\d{4}\s+\d{1,2}:\d{2}:\d{2}\s+(?:AM|PM)$/i, format: 'DD.MM.YYYY hh:mm:ss A'},
        {regex: /^\d{2}\.\d{2}\.\d{4}\s+\d{1,2}:\d{2}\s+(?:AM|PM)$/i, format: 'DD.MM.YYYY hh:mm A'},
        // 24-hour formats
        {regex: /^\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}:\d{2}$/, format: 'YYYY-MM-DD HH:mm:ss'},
        {regex: /^\d{2}\.\d{2}\.\d{4}\s+\d{2}:\d{2}:\d{2}$/, format: 'DD.MM.YYYY HH:mm:ss'},
        {regex: /^\d{2}\/\d{2}\/\d{4}\s+\d{2}:\d{2}:\d{2}$/, format: 'MM/DD/YYYY HH:mm:ss'},
        {regex: /^\d{4}-\d{2}-\d{2}\s+\d{2}:\d{2}$/, format: 'YYYY-MM-DD HH:mm'},
        {regex: /^\d{2}\.\d{2}\.\d{4}\s+\d{2}:\d{2}$/, format: 'DD.MM.YYYY HH:mm'},
        {regex: /^\d{2}\/\d{2}\/\d{4}\s+\d{2}:\d{2}$/, format: 'MM/DD/YYYY HH:mm'},
        {regex: /^\d{2}:\d{2}:\d{2}$/, format: 'HH:mm:ss'},
        {regex: /^\d{2}:\d{2}$/, format: 'HH:mm'},
        {regex: /^\w{3}\s+\d{1,2}\s+\d{2}:\d{2}:\d{2}$/, format: 'ddd DD HH:mm:ss'},
        {regex: /^\w{3}\s+\d{1,2}\s+\d{2}:\d{2}$/, format: 'ddd DD HH:mm'}
    ];

    for (var i = 0; i < patterns.length; i++) {
        if (patterns[i].regex.test(sampleValue.trim())) {
            return patterns[i].format;
        }
    }

    // Default fallback
    return 'YYYY-MM-DD HH:mm:ss';
}

// --- HELPER FUNCTION: Format datetime according to detected format ---
function formatDateTime(date, format) {
    var year = date.getFullYear();
    var month = String(date.getMonth() + 1).padStart(2, '0');
    var day = String(date.getDate()).padStart(2, '0');
    var hours24 = date.getHours();
    var hours = String(hours24).padStart(2, '0');
    var minutes = String(date.getMinutes()).padStart(2, '0');
    var seconds = String(date.getSeconds()).padStart(2, '0');

    var dayNames = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
    var dayName = dayNames[date.getDay()];

    // Calculate 12-hour format
    var hours12 = hours24 % 12;
    if (hours12 === 0) hours12 = 12; // 0 becomes 12
    var hours12Str = String(hours12).padStart(2, '0');
    var ampm = hours24 >= 12 ? 'PM' : 'AM';

    // Replace format tokens with actual values
    // Handle 12-hour formats first (hh before HH to avoid conflicts)
    var formatted = format
        .replace('YYYY', year)
        .replace('MM', month)
        .replace('DD', day)
        .replace('hh', hours12Str)  // 12-hour format (must be before HH)
        .replace('HH', hours)       // 24-hour format
        .replace('mm', minutes)
        .replace('ss', seconds)
        .replace('A', ampm)         // AM/PM
        .replace('ddd', dayName);

    return formatted;
}

// --- HELPER FUNCTION 3: Get mapping of value-card names to payload attributes ---
// This mapping is loaded from skin.conf via window.MQTT_SENSOR_MAPPING
// Configuration: skin.conf [[MQTT]] -> [[[SensorMapping]]]
function getPayloadMapping() {
    // Return the global mapping if available, otherwise return empty object
    return window.MQTT_SENSOR_MAPPING || {};
}

// --- HELPER FUNCTION 4: Update payload values in value-cards ---
function updatePayloadValues(payload) {
    var mapping = getPayloadMapping();
    var valueCards = document.querySelectorAll('.card-value');

    valueCards.forEach(function (card) {
        var cardName = card.getAttribute('data-name');
        var mapEntry = mapping[cardName];

        if (!mapEntry) {
            debugLog('⚠️ No mapping found for card-name: ' + cardName);
            return;
        }

        var payloadValue = payload[mapEntry.payloadAttr];
        if (payloadValue === undefined || payloadValue === null) {
            debugLog('⚠️ Payload missing attribute: ' + mapEntry.payloadAttr);
            return;
        }

        var numValue = parseFloat(payloadValue);
        if (Number.isNaN(numValue)) {
            debugLog('⚠️ Invalid value for ' + mapEntry.payloadAttr + ' : ' + payloadValue);
            return;
        }

        var formattedValue = numValue.toFixed(mapEntry.decimals) + mapEntry.unit;
        var h4Element = card.querySelector('h4.h2-responsive');

        if (h4Element) {
            // Check if value changed
            var currentText = h4Element.textContent.trim();
            if (currentText !== formattedValue) {
                h4Element.innerHTML = formattedValue;
                debugLog('✓ Updated' + cardName + ' to: ' + formattedValue);

                // Visual feedback
                applyGreenFlash(h4Element);
            }
        }
    });
}

// --- HELPER FUNCTION 5: Apply visual feedback (green flash) ---
function applyGreenFlash(element) {
    // Check if flash on update is enabled
    if (window.MQTT_CONFIG && window.MQTT_CONFIG.flash_on_update === false) {
        return; // Skip flash effect if disabled
    }

    // Get flash color from configuration (default to green if not set)
    var flashColor = (window.MQTT_CONFIG && window.MQTT_CONFIG.flash_color)
        ? window.MQTT_CONFIG.flash_color
        : "#00ff00";

    element.style.transition = "color 0.5s, text-shadow 0.5s";
    element.style.color = flashColor;
    element.style.textShadow = "0 0 8px " + flashColor;
    setTimeout(function () {
        if (element) {
            element.style.color = "";
            element.style.textShadow = "none";
        }
    }, 1500);
}

// --- HELPER FUNCTION 6: Update telemetry (battery, signal, etc) ---
function updateTelemetry(payload) {
    // Signal strength
    if (payload.rxCheckPercent !== undefined) {
        var el = document.getElementById('mqtt-5in1-sig');
        if (el) el.innerHTML = parseFloat(payload.rxCheckPercent).toFixed(0) + "%";
    }

    // 5in1 Battery status
    if (payload.outTempBatteryStatus !== undefined) {
        var el = document.getElementById('mqtt-5in1-batt');
        if (el) {
            var isLow = (payload.outTempBatteryStatus == 1);
            el.innerHTML = isLow ? "LOW" : "OK";
            el.style.color = isLow ? "#ff4444" : "#00ff00";
            el.style.fontWeight = "bold";
        }
    }

    // Garage battery status
    if (payload.batteryStatus1 !== undefined) {
        var el = document.getElementById('mqtt-garage-batt');
        if (el) {
            var isLow = (payload.batteryStatus1 == 1);
            el.innerHTML = isLow ? "LOW" : "OK";
            el.style.color = isLow ? "#ff4444" : "#00ff00";
            el.style.fontWeight = "bold";
        }
    }
}

// 4. CONNECTION LOST HANDLER
if (client) {
    client.onConnectionLost = function (responseObject) {
        if (responseObject.errorCode !== 0) {
            console.log("Connection Lost: " + responseObject.errorMessage);
            updateMQTTStatusIndicator(false);
            // We don't need a setTimeout here anymore!
            // Paho v1.1.0 handles the 'reconnect: true' automatically.
        }
    };
}

// 5. PHONE WAKE-UP RECONNECT
if (client) {
    document.addEventListener("visibilitychange", function () {
        if (!document.hidden) {
            debugLog("Tab wake-up detected. Checking connection...");
            // ONLY connect if the status is actually disconnected
            if (!client.isConnected()) {
                debugLog("MQTT not connected. Initiating wake-up connection...");
                client.connect(options);
            } else {
                debugLog("MQTT already reconnected by internal library logic.");
            }
        }
    });
}
