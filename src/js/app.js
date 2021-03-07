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
