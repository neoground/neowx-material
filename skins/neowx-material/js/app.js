// Tooltip support
$(function () {
    $('[data-toggle="tooltip"]').tooltip()
});

// Number rounding based on weewx values
// Example: Number: 34.5678 Format: %.2f Result: 34.57
function formatNumber(no, format) {
    // Extract number of decimal places from format
    format = format.replace(/[^0-9]/g, '');
    return no.toFixed(format);
}

// --- HELPER FUNCTION: Debug logging (respects debug flag) ---
function debugLog(message) {
    if (window.MQTT_CONFIG && window.MQTT_CONFIG.debug) {
        console.log('[MQTT DEBUG] ' + message);
    }
}

document.addEventListener('DOMContentLoaded', function () {
    // Get all IDs of elements with class "card-value" and store in a variable
    const valueCards = document.querySelectorAll('.card-value');
    const valueCardIds = Array.from(valueCards).map(card => card.id);
    debugLog('Value Card IDs: ' + valueCardIds);

    // Get all IDs of elements with class "card-chart" and store in a variable
    const cardCharts = document.querySelectorAll('.card-chart');
    const cardChartIds = Array.from(cardCharts).map(card => card.id);
    debugLog('Card Chart IDs: ' + cardChartIds);

    // --- CLICK HANDLER 1: Value Cards -> Jump to Chart Cards ---
    valueCards.forEach(function(valueCard) {
        valueCard.addEventListener('click', function(e) {
            // Get the sensor name from data-name attribute
            const sensorName = valueCard.getAttribute('data-name');
            if (!sensorName) {
                debugLog('⚠️ No data-name found on value card');
                return;
            }

            // Special handling for wind icon - jump to wind vector image
            if (sensorName === 'windSpeed' && e.target.closest('i.wi')) {
                const vectorChart = document.getElementById('embedded_imageWindVect');
                if (vectorChart) {
                    scrollToElement(vectorChart);
                    e.preventDefault();
                    e.stopPropagation();
                    return;
                }
            }

            // Don't navigate if clicking on icons (except wind)
            if (e.target.closest('i.wi')) {
                e.preventDefault();
                e.stopPropagation();
                return;
            }

            // Map sensor to chart name
            let chartSensorName = sensorName;

            // Special mappings: some sensors share the same chart
            if (sensorName === 'appTemp' || sensorName === 'dewpoint') {
                chartSensorName = 'outTemp';
            } else if (sensorName === 'heatindex') {
                chartSensorName = 'windchill';
            }

            // Find the corresponding chart card by data-name
            const chartCard = document.querySelector('.card-chart[data-name="' + chartSensorName + '"]');

            if (chartCard) {
                scrollToElement(chartCard, true);
                debugLog('✓ Jumped from value card "' + sensorName + '" to chart "' + chartSensorName + '"');
            } else {
                debugLog('⚠️ No chart card found for sensor: ' + chartSensorName);
            }
        });
    });

    // --- CLICK HANDLER 2: Chart Cards -> Jump to Value Cards ---
    cardCharts.forEach(function(chartCard) {
        chartCard.addEventListener('click', function(e) {
            // Only respond to clicks on the title (H5 element)
            if (e.target.tagName !== 'H5') {
                return;
            }

            // Get the sensor name from data-name attribute
            const sensorName = chartCard.getAttribute('data-name');
            if (!sensorName) {
                debugLog('⚠️ No data-name found on chart card');
                return;
            }

            // Map chart name to value card name
            let valueSensorName = sensorName;

            // Special mappings
            if (sensorName === 'windvec') {
                valueSensorName = 'windSpeed';
            }

            // Find the corresponding value card by data-name
            const valueCard = document.querySelector('.card-value[data-name="' + valueSensorName + '"]');

            if (valueCard) {
                scrollToElement(valueCard, true);
                debugLog('✓ Jumped from chart "' + sensorName + '" to value card "' + valueSensorName + '"');
                e.preventDefault();
            } else {
                debugLog('⚠️ No value card found for sensor: ' + valueSensorName);
            }
        });
    });

    // --- HELPER FUNCTION: Smooth scroll to element with visual feedback ---
    function scrollToElement(element, highlightBackground) {
        if (!element) return;

        const topOffset = element.getBoundingClientRect().top + window.pageYOffset - 100;
        window.scrollTo({
            top: topOffset,
            behavior: 'smooth'
        });

        // Optional green background flash
        if (highlightBackground) {
            element.style.transition = "background-color 0.5s";
            element.style.backgroundColor = "rgba(0, 255, 0, 0.1)";
            setTimeout(function() {
                element.style.backgroundColor = "";
            }, 1000);
        }
    }
});