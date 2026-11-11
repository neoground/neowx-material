// neowx-date-format.js
// Returns a moment.js formatter for the given format string.
//
// Centralizes the ":MM" -> ":mm" minutes-token fix that the chart configs need:
// users commonly write uppercase MM for minutes, but moment uses lowercase mm
// (uppercase MM is the month). We only rewrite ":MM" so month tokens are safe.
//
// The returned function accepts ApexCharts' two calling conventions:
//   - axis labels:  formatter(value, timestamp)  -> formats `timestamp`
//   - tooltip x:    formatter(value)             -> formats `value`
function neowxDateFormatter(fmt) {
    fmt = String(fmt).replace(/:MM/g, ':mm');
    return function (val, timestamp) {
        // Axis-label formatters are called (value, timestampNumber); tooltip formatters are
        // called (value, optsObject). Use the 2nd arg only when it is the numeric timestamp,
        // otherwise fall back to the first arg.
        var ts = (typeof timestamp === 'number') ? timestamp : val;
        return moment(ts).format(fmt);
    };
}

// Export for Node-based unit tests. Ignored in the browser, where this file is
// loaded as a plain script and `neowxDateFormatter` becomes a global.
if (typeof module !== 'undefined' && module.exports) {
    module.exports = neowxDateFormatter;
}
