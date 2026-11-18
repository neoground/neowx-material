// Weewx MQTT Client Script
//
// Version 1.0.4
//
// 1. INIT PAHO CLIENT
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
    }
})();

// --- HELPER FUNCTION: Update MQTT status indicator ---
function updateMQTTStatusIndicator(connected) {
    if (window.MQTT_CONFIG && window.MQTT_CONFIG.enabled) {
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
}

// 2. Global Helper Functions
function getCompass(deg) {
    if (deg === undefined || deg === null || isNaN(deg)) return "";
    let val = Math.floor((deg / 22.5) + 0.5);

    // Use language-specific directions passed from skin.conf via MQTT_CONFIG,
    // fall back to English if not available or incomplete (need at least 16 entries)
    const fallbackDirections = ["N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE", "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"];
    let directions = fallbackDirections;
    if (window.MQTT_CONFIG && Array.isArray(window.MQTT_CONFIG.directions) && window.MQTT_CONFIG.directions.length >= 16) {
        directions = window.MQTT_CONFIG.directions;
    }
    return directions[(val % 16)];
}

// 3. MQTT MESSAGE HANDLER
// Only set up handler if client was created (i.e., MQTT is enabled)
if (client) {
    client.onMessageArrived = function (message) {
        try {
            var payload;
            var msgFormat = (window.MQTT_CONFIG && window.MQTT_CONFIG.message_format) || 'json';

            if (msgFormat === 'ecowitt') {
                debugLog('Ecowitt format detected – using Ecowitt parser');
                payload = parseEcowittMessage(message.payloadString);
                if (!payload) {
                    debugLog('⚠️ Ecowitt parser returned null, skipping update');
                    return;
                }
            } else {
                payload = JSON.parse(message.payloadString);
            }

            debugLog('Message received: ' + JSON.stringify(payload).substring(0, 100) + '...');

            // Check if timestamp is newer, if yes - update page
            var timestampCheck = evaluateMessageTimestamp(payload);
            if (timestampCheck.isNewer) {
                debugLog('Timestamp check passed (isNewer: ' + timestampCheck.isNewer + ', skipped: ' + timestampCheck.skipped + '), updating values');
                updateDateTime(payload, timestampCheck.skipped);
                updatePayloadValues(payload);
                updateTelemetry(payload);
            } else {
                debugLog('Timestamp check failed, no update performed');
            }

        } catch (e) {
            console.log("❌ Sync Error:", e);
        }
    };
}

// --- HELPER FUNCTION 1: Check if payload timestamp is newer ---
// Returns: { isNewer: bool, skipped: bool }
function evaluateMessageTimestamp(payload) {
    // Check if timestamp check should be skipped
    if (window.MQTT_CONFIG && window.MQTT_CONFIG.skip_timestamp_check === true) {
        debugLog('Timestamp check skipped (skip_timestamp_check is enabled)');
        return { isNewer: true, skipped: true };
    }

    // Get the timestamp field name from config (default: dateTime)
    var timestampField = (window.MQTT_CONFIG && window.MQTT_CONFIG.message_timestamp_field)
        ? window.MQTT_CONFIG.message_timestamp_field
        : 'dateTime';

    if (payload[timestampField] === undefined) {
        debugLog('No Timestamp in payload field "' + timestampField + '", and NO timestamp check -> no update performed');
        return { isNewer: false, skipped: false };
    }

    var timeSpan = document.getElementById('current-datetime');
    if (!timeSpan) return { isNewer: false, skipped: false };

    var currentUnixTime = parseInt(timeSpan.getAttribute('data-timestamp'), 10);
    var payloadUnixTime = parseFloat(payload[timestampField]);

    debugLog('Timestamp comparison - Server: ' + currentUnixTime + ', Payload: ' + payloadUnixTime + ' (field: ' + timestampField + ')');

    if (payloadUnixTime > currentUnixTime) {
        debugLog('✓ Payload timestamp is newer');
        return { isNewer: true, skipped: false };
    } else {
        debugLog('⊗ Payload timestamp is not newer, skipping update');
        return { isNewer: false, skipped: false };
    }
}

// --- HELPER FUNCTION 2: Update datetime on page ---
function updateDateTime(payload, skipped) {
    var timeSpan = document.getElementById('current-datetime');
    if (!timeSpan) return;

    var payloadUnixTime;
    var payloadDate;

    // Get the timestamp field name from config (default: dateTime)
    var timestampField = (window.MQTT_CONFIG && window.MQTT_CONFIG.message_timestamp_field)
        ? window.MQTT_CONFIG.message_timestamp_field
        : 'dateTime';

    // Always try to use timestamp from payload
    if (payload[timestampField] !== undefined) {
        payloadUnixTime = parseFloat(payload[timestampField]);
        payloadDate = new Date(payloadUnixTime * 1000);
        debugLog('Using timestamp from payload field "' + timestampField + '": ' + payloadUnixTime +
            (skipped ? ' (timestamp check was skipped)' : ''));
    } else {
        // Fallback: if timestamp field not in payload, use current time
        payloadDate = new Date();
        payloadUnixTime = payloadDate.getTime() / 1000;
        debugLog('⚠️ Timestamp field "' + timestampField + '" not found in payload, using current time');
    }

    // Initialize DATETIME_CONFIG if not present (safety check)
    if (typeof window.DATETIME_CONFIG === 'undefined') {
        window.DATETIME_CONFIG = {
            weewx_format: '%a %d %H:%M'  // Default fallback
        };
    }

    // Convert WeeWX strftime format to JavaScript-compatible format (only once)
    if (!window.DATETIME_CONFIG.js_format) {
        var strftimeFormat = window.DATETIME_CONFIG.weewx_format || '%a %d %H:%M';
        window.DATETIME_CONFIG.js_format = convertStrftimeToJS(strftimeFormat);
        debugLog('Using WeeWX format: ' + strftimeFormat + ' -> JS format: ' + window.DATETIME_CONFIG.js_format);
    }

    // Format the new datetime using the converted format
    var newDateTime = formatDateTime(payloadDate, window.DATETIME_CONFIG.js_format);

    timeSpan.innerHTML = newDateTime;
    timeSpan.setAttribute('data-timestamp', payloadUnixTime);

    debugLog('✓ Updated datetime to: ' + newDateTime + ' (Unix: ' + payloadUnixTime + ')');

    // --- Update split date / time display elements (new 3-column header layout) ---
    var dateEl = document.getElementById('current-date');
    var timeEl = document.getElementById('current-time');
    if (dateEl) {
        if (window.DATETIME_CONFIG && window.DATETIME_CONFIG.date_format) {
            if (!window.DATETIME_CONFIG.js_date_format) {
                window.DATETIME_CONFIG.js_date_format = convertStrftimeToJS(window.DATETIME_CONFIG.date_format);
            }
            dateEl.textContent = formatDateTime(payloadDate, window.DATETIME_CONFIG.js_date_format);
        } else {
            var _p = function(n) { return String(n).padStart(2, '0'); };
            dateEl.textContent = _p(payloadDate.getDate()) + '.' + _p(payloadDate.getMonth() + 1) + '.' + payloadDate.getFullYear();
        }
    }
    if (timeEl) {
        if (window.DATETIME_CONFIG && window.DATETIME_CONFIG.time_format) {
            if (!window.DATETIME_CONFIG.js_time_format) {
                window.DATETIME_CONFIG.js_time_format = convertStrftimeToJS(window.DATETIME_CONFIG.time_format);
            }
            timeEl.textContent = formatDateTime(payloadDate, window.DATETIME_CONFIG.js_time_format);
        } else {
            var _p = function(n) { return String(n).padStart(2, '0'); };
            timeEl.textContent = _p(payloadDate.getHours()) + ':' + _p(payloadDate.getMinutes()) + ':' + _p(payloadDate.getSeconds());
        }
    }

    // Visual feedback - check if flash is enabled
    if (window.MQTT_CONFIG && window.MQTT_CONFIG.flash_on_update !== false) {
        // Get flash color from configuration (default to green if not set)
        var flashColor = (window.MQTT_CONFIG && window.MQTT_CONFIG.flash_color)
            ? window.MQTT_CONFIG.flash_color
            : "#00ff00";

        // Flash only the time element (date stays static); fall back to timeSpan if not present
        var flashTargets = [timeEl].filter(Boolean);
        if (flashTargets.length === 0) flashTargets = [timeSpan];
        flashTargets.forEach(function(el) {
            el.style.transition = "color 0.5s, text-shadow 0.5s";
            el.style.color = flashColor;
            el.style.textShadow = "0 0 8px " + flashColor;
            setTimeout(function () {
                el.style.color = "#fff"; // restore full white to match station name / nav
                el.style.textShadow = "none";
            }, 1500);
        });
    }
}


// --- HELPER FUNCTION: Convert Python strftime format to JavaScript-compatible format ---
// Takes a Python strftime format string (e.g., "%d.%m.%Y %H:%M") and converts it to a custom format
// that our formatDateTime function can use (e.g., "DD.MM.YYYY HH:mm")
function convertStrftimeToJS(strftimeFormat) {
    if (!strftimeFormat) return 'YYYY-MM-DD HH:mm:ss';

    debugLog('Converting strftime format: ' + strftimeFormat);

    // Map Python strftime codes to our JavaScript format tokens
    var converted = strftimeFormat
        // Date components
        .replace(/%Y/g, 'YYYY')      // 4-digit year
        .replace(/%y/g, 'YY')        // 2-digit year
        .replace(/%m/g, 'MM')        // Month as zero-padded number
        .replace(/%d/g, 'DD')        // Day of month as zero-padded number
        .replace(/%j/g, 'DDD')       // Day of year
        // Time components
        .replace(/%H/g, 'HH')        // Hour (24-hour) zero-padded
        .replace(/%I/g, 'hh')        // Hour (12-hour) zero-padded
        .replace(/%M/g, 'mm')        // Minute zero-padded
        .replace(/%S/g, 'ss')        // Second zero-padded
        .replace(/%p/g, 'A')         // AM/PM
        // Weekday
        .replace(/%A/g, 'dddd')      // Full weekday name
        .replace(/%a/g, 'ddd')       // Abbreviated weekday name
        // Month name
        .replace(/%B/g, 'MMMM')      // Full month name
        .replace(/%b/g, 'MMM')       // Abbreviated month name
        // Special formats
        .replace(/%x/g, 'MM/DD/YYYY') // Locale date representation
        .replace(/%X/g, 'HH:mm:ss')   // Locale time representation
        .replace(/%c/g, 'ddd MMM DD HH:mm:ss YYYY'); // Locale datetime

    debugLog('Converted to JS format: ' + converted);
    return converted;
}

// --- HELPER FUNCTION: Format datetime according to detected format ---
function formatDateTime(date, format) {
    var year = date.getFullYear();
    var yearShort = String(year).substr(2, 2);
    var month = String(date.getMonth() + 1).padStart(2, '0');
    var day = String(date.getDate()).padStart(2, '0');
    var dayOfYear = String(Math.ceil((date - new Date(date.getFullYear(), 0, 0)) / 86400000)).padStart(3, '0');
    var hours24 = date.getHours();
    var hours = String(hours24).padStart(2, '0');
    var minutes = String(date.getMinutes()).padStart(2, '0');
    var seconds = String(date.getSeconds()).padStart(2, '0');

    // Day and month names
    var dayNames = ['Sunday', 'Monday', 'Tuesday', 'Wednesday', 'Thursday', 'Friday', 'Saturday'];
    var dayNamesShort = ['Sun', 'Mon', 'Tue', 'Wed', 'Thu', 'Fri', 'Sat'];
    var monthNames = ['January', 'February', 'March', 'April', 'May', 'June',
        'July', 'August', 'September', 'October', 'November', 'December'];
    var monthNamesShort = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun',
        'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];

    var dayName = dayNames[date.getDay()];
    var dayNameShort = dayNamesShort[date.getDay()];
    var monthName = monthNames[date.getMonth()];
    var monthNameShort = monthNamesShort[date.getMonth()];

    // Calculate 12-hour format
    var hours12 = hours24 % 12;
    if (hours12 === 0) hours12 = 12; // 0 becomes 12
    var hours12Str = String(hours12).padStart(2, '0');
    var ampm = hours24 >= 12 ? 'PM' : 'AM';

    // Replace format tokens with actual values
    // Handle longer tokens first to avoid partial replacements (e.g., MMMM before MMM, dddd before ddd)
    var formatted = format
        .replace(/YYYY/g, year)         // 4-digit year
        .replace(/YY/g, yearShort)      // 2-digit year
        .replace(/MMMM/g, monthName)    // Full month name
        .replace(/MMM/g, monthNameShort) // Abbreviated month name
        .replace(/MM/g, month)          // Month as zero-padded number
        .replace(/DDD/g, dayOfYear)     // Day of year
        .replace(/DD/g, day)            // Day of month as zero-padded number
        .replace(/dddd/g, dayName)      // Full weekday name
        .replace(/ddd/g, dayNameShort)  // Abbreviated weekday name
        .replace(/hh/g, hours12Str)     // 12-hour format (must be before HH)
        .replace(/HH/g, hours)          // 24-hour format
        .replace(/mm/g, minutes)        // Minutes
        .replace(/ss/g, seconds)        // Seconds
        .replace(/A/g, ampm);           // AM/PM

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
        let cardName = card.getAttribute('data-name');
        let mapEntry = mapping[cardName];

        if (!mapEntry) {
            debugLog('⚠️ No mapping found for card-name: ' + cardName);
            return;
        }

        let payloadValue = payload[mapEntry.payloadAttr];
        if (payloadValue === undefined || payloadValue === null) {
            debugLog('⚠️ Payload missing attribute: ' + mapEntry.payloadAttr);
            return;
        }

        var numValue = parseFloat(payloadValue);
        if (Number.isNaN(numValue)) {
            debugLog('⚠️ Invalid value for ' + mapEntry.payloadAttr + ' : ' + payloadValue);
            return;
        }

        var h4Element = card.querySelector('h4.h2-responsive');

        if (h4Element) {
            // Check if value changed
            let currentText = h4Element.textContent.trim();

            // Detect decimal separator from current text (WeeWX format)
            let decimalSeparator = '.'; // default
            let hasComma = /\d,\d/.test(currentText);
            let hasDot = /\d\.\d/.test(currentText);

            if (hasComma && !hasDot) {
                decimalSeparator = ',';
            }
            debugLog('Detected decimal separator for ' + cardName + ': "' + decimalSeparator + '"');

            // Format value with detected separator
            let formattedValue = numValue.toFixed(mapEntry.decimals);
            if (decimalSeparator === ',') {
                formattedValue = formattedValue.replace('.', ',');
            }
            formattedValue += mapEntry.unit;

            // Telemetry gauge cards (inc/telemetry.inc) carry a server-built spec
            // in data-nwm-gauge: they show a mapped state such as "OK" and a
            // drawn gauge, not just a number. Since telemetry items may sit in
            // any card section they share this page with MQTT, and writing
            // formattedValue over them turned "OK" into "0.0 V". They get
            // their own updater instead - the attribute is the contract.
            var gaugeSpec = card.getAttribute('data-nwm-gauge');
            if (gaugeSpec !== null) {
                updateGaugeCard(card, gaugeSpec, numValue, formattedValue, h4Element);
                return;
            }

            if (cardName === 'windSpeed') {
                let windDirMapEntry = mapping['windDir'];
                let windDirValue = payload[windDirMapEntry ? windDirMapEntry.payloadAttr : null];
                if (windDirValue !== undefined) {
                    formattedValue += ' ' + getCompass(windDirValue);

                    // Update wind direction icon (class, not id - a windSpeed
                    // card can render more than once per page under
                    // per-section duplicates, so scope the lookup to this card)
                    let windIcon = card.querySelector('.nwm-wind-icon');
                    if (windIcon && (currentText !== formattedValue)) {
                        let deg = Math.round(parseFloat(windDirValue));
                        // Remove existing direction class (wi-wind from-NNN-deg)
                        let existingClasses = windIcon.className.split(' ');
                        existingClasses = existingClasses.filter(function (c) {
                            return !/^from-\d+-deg$/.test(c);
                        });
                        existingClasses.push('from-' + deg + '-deg');
                        windIcon.className = existingClasses.join(' ');
                        windIcon.setAttribute('title', deg + '°');
                        windIcon.setAttribute('data-original-title', deg + '°');
                        if (typeof $ !== 'undefined' && $.fn.tooltip) {
                            $(windIcon).tooltip('dispose').tooltip();
                        }
                        debugLog('✓ Updated wind-icon to: from-' + deg + '-deg');
                    }
                }
            }

            if (currentText !== formattedValue) {
                h4Element.innerHTML = formattedValue;
                debugLog('✓ Updated ' + cardName + ' to: ' + formattedValue);

                // Visual feedback
                applyGreenFlash(h4Element);
            }
        }
    });
}

// --- HELPER FUNCTION 4b: Update a telemetry gauge card from its spec ---
// The spec is built server-side by gaugeSpec() in inc/telemetry.inc from the same
// helpers that drew the card, so nothing here reads [[Telemetry]]. A status
// field arrives as a complete per-state table (label, bar percentage, fill
// width, colour); a range field as min/max and low_threshold. The arithmetic
// below mirrors rangePercentage(), percentValue() and getGaugeColor() in the
// include - change one and change the other.
var GAUGE_RED = '#f44336';
var GAUGE_GREEN = '#4caf50';

function gaugePercent(spec, value) {
    var pct;
    if (spec.type === 'percent') {
        pct = value;
    } else if (spec.max === null || spec.max === undefined) {
        // No usable range. A voltage gauge is then not drawn at all (the card
        // reserves the space); a signal gauge reads the raw value as a percentage.
        if (spec.type !== 'signal') return null;
        pct = value;
    } else {
        var min = Number(spec.min) || 0;
        pct = ((value - min) / (Number(spec.max) - min)) * 100;
    }
    if (pct < 0) pct = 0;
    if (pct > 100) pct = 100;
    // Same epsilon as the include, so 56.99999 reads as 57 on both sides
    return Math.floor(pct + 0.0000001);
}

// Recompute the two filled wings of the signal cone. Geometry is a port of
// signalCone() in inc/telemetry.inc; the static grey wings and the centre dot are
// not touched.
function drawSignalCone(svg, pct, color) {
    var R = 14.6, hhI = 4.75, hhO = 11.0;
    var f = Math.min(1, Math.max(0, pct / 100));
    var si = R - Math.sqrt(R * R - hhI * hhI);
    var hhf = hhI + f * (hhO - hhI);
    var sf = R - Math.sqrt(R * R - hhf * hhf);
    var xfL = 52 + f * (12 - 52);
    var xfR = 68 + f * (108 - 68);
    var n = function (v) { return v.toFixed(3); };
    var left = svg.querySelector('path[data-nwm-cone="l"]');
    var right = svg.querySelector('path[data-nwm-cone="r"]');
    if (left) {
        left.setAttribute('d', 'M 52 7.25 L ' + n(xfL) + ' ' + n(12 - hhf) +
            ' Q ' + n(xfL - 2 * sf) + ' 12 ' + n(xfL) + ' ' + n(12 + hhf) +
            ' L 52 16.75 Q ' + n(52 - 2 * si) + ' 12 52 7.25 Z');
        left.setAttribute('fill', color);
    }
    if (right) {
        right.setAttribute('d', 'M 68 7.25 L ' + n(xfR) + ' ' + n(12 - hhf) +
            ' Q ' + n(xfR + 2 * sf) + ' 12 ' + n(xfR) + ' ' + n(12 + hhf) +
            ' L 68 16.75 Q ' + n(68 + 2 * si) + ' 12 68 7.25 Z');
        right.setAttribute('fill', color);
    }
}

function updateGaugeCard(card, specJson, numValue, formattedValue, h4Element) {
    var spec;
    try {
        spec = JSON.parse(specJson);
    } catch (e) {
        debugLog('⚠️ Unreadable gauge spec on ' + card.getAttribute('data-name'));
        return;
    }

    // What the server would have rendered for this value
    var heading = formattedValue;   // raw number + unit, as the card shows when unmapped
    var pct = null, fillWidth = null, color = null, valText = null;

    if (spec.type === 'status') {
        var key = String(Math.floor(numValue + 0.000001));
        var state = spec.states ? spec.states[key] : undefined;
        if (state) {
            heading = state.label;
            pct = state.pct;
            fillWidth = state.fill;
            color = state.color;
        }
        valText = numValue.toFixed(0);
    } else if (spec.type === 'voltage' || spec.type === 'signal' || spec.type === 'percent') {
        pct = gaugePercent(spec, numValue);
        if (pct !== null) {
            var threshold = Number(spec.low_threshold);
            if (Number.isNaN(threshold)) threshold = 20;
            color = pct < threshold ? GAUGE_RED : GAUGE_GREEN;
            fillWidth = pct;
            valText = pct + '%';
        }
        if (spec.type === 'percent') {
            heading = valText || (Math.round(numValue) + '%');
        }
    }
    // sensor_type none: the heading is all there is

    if (h4Element && h4Element.textContent.trim() !== heading) {
        h4Element.textContent = heading;
        applyGreenFlash(h4Element);
    }

    if (fillWidth !== null && color !== null) {
        var fill = card.querySelector('.battery-fill');
        if (fill) {
            fill.style.width = fillWidth + '%';
            fill.style.backgroundColor = color;
        }
    }
    if (pct !== null && color !== null) {
        var cone = card.querySelector('svg[data-nwm-gauge-cone]');
        if (cone) drawSignalCone(cone, pct, color);
    }
    if (valText !== null) {
        var text = card.querySelector('[data-nwm-gauge-text]');
        if (text) text.textContent = valText;
    }
    debugLog('✓ Updated gauge ' + card.getAttribute('data-name') + ' to: ' + heading);
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

// =============================================================================
// 6. ECOWITT MESSAGE PARSER
//
// Converts a raw Ecowitt HTTP POST payload to a weewx-compatible JSON object.
// Activated when  [[MQTT]] message_format = ecowitt  in skin.conf.
//
// Ecowitt sends data as an application/x-www-form-urlencoded body, optionally
// preceded by HTTP headers.  The payload starts at "PASSKEY=".
//
// Field mapping  :  Ecowitt name  →  weewx payload attribute (+ unit conversion)
// Derived fields :  dewpoint, windchill, heatindex, humidex, appTemp, cloudbase,
//                   inDewpoint, altimeter (≈ barometer)
// =============================================================================

/**
 * Parse a raw Ecowitt HTTP POST message and return a weewx-compatible object.
 *
 * @param  {string}      rawMessage  Full HTTP request string from the device
 * @returns {Object|null}            weewx payload object or null on failure
 */
function parseEcowittMessage(rawMessage) {
    debugLog('Ecowitt: starting parse...');

    // ── Step 1: locate PASSKEY= and extract the form-encoded body ────────────
    var start = rawMessage.indexOf('PASSKEY=');
    if (start === -1) {
        debugLog('⚠️ Ecowitt: PASSKEY= not found – aborting');
        return null;
    }
    var body = rawMessage.substring(start);

    // ── Step 2: split into key/value pairs and URL-decode ────────────────────
    var params = {};
    body.split('&').forEach(function (pair) {
        var eq = pair.indexOf('=');
        if (eq === -1) return;
        var k = decodeURIComponent(pair.substring(0, eq).trim());
        var v = decodeURIComponent(pair.substring(eq + 1).trim());
        params[k] = v;
    });
    debugLog('Ecowitt: parsed ' + Object.keys(params).length + ' raw params');

    // ── Step 3: unit-conversion helpers ──────────────────────────────────────
    function fToC(v)       { return (parseFloat(v) - 32) * 5 / 9; }
    function inHgToMbar(v) { return parseFloat(v) * 33.8639; }
    function mphToKph(v)   { return parseFloat(v) * 1.60934; }
    function inToCm(v)     { return parseFloat(v) * 2.54; }
    function pf(v)         { return parseFloat(v); }

    // ── Step 4: map Ecowitt fields → weewx payload attribute names ───────────
    var r = {};

    // Indoor
    if (params.tempinf    !== undefined) r.inTemp_C   = fToC(params.tempinf);
    if (params.humidityin !== undefined) r.inHumidity = pf(params.humidityin);

    // Pressure
    if (params.baromrelin !== undefined) r.barometer_mbar = inHgToMbar(params.baromrelin);
    if (params.baromabsin !== undefined) r.pressure_mbar  = inHgToMbar(params.baromabsin);

    // Outdoor temperature / humidity
    if (params.tempf    !== undefined) r.outTemp_C   = fToC(params.tempf);
    if (params.humidity !== undefined) r.outHumidity = pf(params.humidity);

    // Wind  (prefer 10-min avg direction over instantaneous)
    if      (params.winddir_avg10m !== undefined) r.windDir      = pf(params.winddir_avg10m);
    else if (params.winddir        !== undefined) r.windDir      = pf(params.winddir);
    if (params.windspeedmph  !== undefined) r.windSpeed_kph = mphToKph(params.windspeedmph);
    if (params.windgustmph   !== undefined) r.windGust_kph  = mphToKph(params.windgustmph);
    if (params.maxdailygust  !== undefined) r.maxGust_kph   = mphToKph(params.maxdailygust);

    // Solar / UV
    if (params.solarradiation !== undefined) r.radiation_Wpm2 = pf(params.solarradiation);
    if (params.uv             !== undefined) r.UV             = pf(params.uv);

    // Rain  (inches → cm)
    if (params.rainratein    !== undefined) r.rainRate_cm_per_hour = inToCm(params.rainratein);
    if (params.eventrainin   !== undefined) r.eventRain_cm         = inToCm(params.eventrainin);
    if (params.hourlyrainin  !== undefined) r.hourRain_cm          = inToCm(params.hourlyrainin);
    if (params.last24hrainin !== undefined) r.rain24_cm            = inToCm(params.last24hrainin);
    if (params.dailyrainin   !== undefined) r.dayRain_cm           = inToCm(params.dailyrainin);
    if (params.weeklyrainin  !== undefined) r.weekRain_cm          = inToCm(params.weeklyrainin);
    if (params.monthlyrainin !== undefined) r.monthRain_cm         = inToCm(params.monthlyrainin);
    if (params.yearlyrainin  !== undefined) r.yearRain_cm          = inToCm(params.yearlyrainin);

    // Extra sensors 1–8
    for (var i = 1; i <= 16; i++) {
        if (params['temp'     + i + 'f'] !== undefined) r['extraTemp'  + i + '_C'] = fToC(params['temp' + i + 'f']);
        if (params['humidity' + i      ] !== undefined) r['extraHumid' + i       ] = pf(params['humidity' + i]);
        if (params['batt'     + i      ] !== undefined) r['extraBattery' + i     ] = pf(params['batt'    + i]);
    }

    // Battery / telemetry
    if (params.console_batt !== undefined) r.consBatteryVoltage_volt = pf(params.console_batt);
    if (params.wh65batt     !== undefined) r.outTempBatteryStatus    = pf(params.wh65batt);
    if (params.soil_ec_batt1 !== undefined) r.soil_ec_batt1    = pf(params.soil_ec_batt1);

    // Timestamp  – dateutc arrives as "2026-03-29+20:31:20" after URL-decode
    if (params.dateutc !== undefined) {
        try {
            var iso = params.dateutc.replace(/\+/g, ' ').replace(' ', 'T') + 'Z';
            r.dateTime = new Date(iso).getTime() / 1000;
        } catch (e) {
            r.dateTime = Date.now() / 1000;
        }
    } else {
        r.dateTime = Date.now() / 1000;
    }

    r.usUnits = 16; // METRICWX

    // ── Step 5: derived meteorological values ────────────────────────────────
    var T  = r.outTemp_C;
    var RH = r.outHumidity;
    var W  = r.windSpeed_kph;

    if (T !== undefined && RH !== undefined) {
        r.dewpoint_C      = ecowittDewpoint(T, RH);
        r.heatindex_C     = ecowittHeatindex(T, RH);
        r.humidex_C       = ecowittHumidex(T, r.dewpoint_C);
        r.appTemp_C       = ecowittAppTemp(T, RH, W !== undefined ? W / 3.6 : 0);
        r.cloudbase_meter = ecowittCloudbase(T, r.dewpoint_C);
    }
    if (T !== undefined && W !== undefined) {
        r.windchill_C = ecowittWindchill(T, W);
    }
    if (r.inTemp_C !== undefined && r.inHumidity !== undefined) {
        r.inDewpoint_C = ecowittDewpoint(r.inTemp_C, r.inHumidity);
    }
    // Altimeter ≈ sea-level barometric pressure (accurate calc needs station altitude)
    if (r.barometer_mbar !== undefined) {
        r.altimeter_mbar = r.barometer_mbar;
    }

    debugLog('Ecowitt: produced ' + Object.keys(r).length + ' weewx fields');
    return r;
}

// ── Meteorological helpers (prefixed ecowitt* to avoid naming conflicts) ─────

/** Dewpoint via Magnus formula.  T in °C, RH in % → °C */
function ecowittDewpoint(T, RH) {
    var g = (17.271 * T) / (237.3 + T) + Math.log(RH / 100.0);
    return (237.3 * g) / (17.271 - g);
}

/**
 * Windchill – US/Canada formula (valid only when T ≤ 10 °C and wind ≥ 4.8 kph).
 * Falls back to T otherwise.
 */
function ecowittWindchill(T, windKph) {
    if (T > 10 || windKph < 4.8) return T;
    var v = Math.pow(windKph, 0.16);
    return 13.12 + 0.6215 * T - 11.37 * v + 0.3965 * T * v;
}

/**
 * Heat index – Rothfusz equation (valid when T > 26.7 °C).
 * Falls back to T otherwise.
 */
function ecowittHeatindex(T, RH) {
    if (T < 26.7) return T;
    var F  = T * 9 / 5 + 32;
    var HI = -42.379
        + 2.04901523  * F
        + 10.14333127 * RH
        - 0.22475541  * F  * RH
        - 0.00683783  * F  * F
        - 0.05391553  * RH * RH
        + 0.00122874  * F  * F  * RH
        + 0.00085282  * F  * RH * RH
        - 0.00000199  * F  * F  * RH * RH;
    return (HI - 32) * 5 / 9;
}

/** Humidex (Canadian).  T and Td in °C → °C */
function ecowittHumidex(T, Td) {
    var e = 6.112 * Math.exp(17.67 * Td / (Td + 243.5));
    return T + 0.5555 * (e - 10.0);
}

/**
 * Apparent temperature (Australian Bureau of Meteorology).
 * T in °C, RH in %, wsMs = wind speed in m/s → °C
 */
function ecowittAppTemp(T, RH, wsMs) {
    var e = (RH / 100) * 6.105 * Math.exp(17.27 * T / (237.7 + T));
    return T + 0.33 * e - 0.70 * wsMs - 4.00;
}

/** Cloudbase – Lifted Condensation Level approximation.  T and Td in °C → metres */
function ecowittCloudbase(T, Td) {
    return 125 * (T - Td);
}


