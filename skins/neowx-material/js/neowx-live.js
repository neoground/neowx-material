// neowx-live.js
// -----------------------------------------------------------------------------
// Keeps the header/footer "chrome" current on NON-current pages (yesterday,
// week, month, year, archive, and the frozen month-YYYY-MM / year-YYYY summary
// pages). Those pages are static HTML that weewx may never rebuild, so their
// server-rendered date / almanac / versions would otherwise stay frozen at
// build time.
//
// The footer uptime is also synced on the CURRENT page, so every page shows the
// same live value: the current page's server-baked uptime is captured at page
// generation and would otherwise read slightly older than the non-current pages,
// which fetch live.json at view time.
//
// On load we fetch live.json (regenerated every report cycle) and overwrite the
// id'd nodes. If the fetch fails, the server-rendered values simply remain as a
// graceful fallback.
//
// Gate: #header-date only exists on non-current pages (the current page uses
// #current-date), so its presence is our "this is a non-current page" signal.
(function () {
    'use strict';

    var isNonCurrent = !!document.getElementById('header-date');
    var hasUptime = !!document.getElementById('footer-station-uptime') ||
                    !!document.getElementById('footer-server-uptime');

    if (!isNonCurrent && !hasUptime) {
        return; // current page with uptime disabled — nothing to sync
    }

    var MOON_ICONS = [
        'wi-moon-new',                 // 0
        'wi-moon-waxing-crescent-4',   // 1
        'wi-moon-first-quarter',       // 2
        'wi-moon-waxing-gibbous-4',    // 3
        'wi-moon-full',                // 4
        'wi-moon-waning-gibbous-4',    // 5
        'wi-moon-third-quarter',       // 6
        'wi-moon-waning-crescent-4'    // 7
    ];
    var MOON_ICON_DEFAULT = 'wi-moon-waning-crescent-3';

    function setText(id, value) {
        if (value === undefined || value === null) { return; }
        var el = document.getElementById(id);
        if (el) { el.textContent = value; }
    }

    function revealGeneratedIcon() {
        // Show the calendar icon when this page's content is "old" enough. The
        // threshold (minutes) is baked from old_page_threshold_minutes; 0 = always.
        // data-generated-ts is an absolute epoch, so the comparison against the
        // browser clock is timezone-independent. Needs no live.json — runs as soon
        // as the script does.
        var status = document.getElementById('generated-status');
        if (!status) { return; }

        var threshold = parseInt(status.getAttribute('data-old-threshold'), 10);
        if (isNaN(threshold)) { threshold = 1440; }

        if (threshold === 0) {
            status.style.display = '';
            return;
        }

        var generatedTs = Number(status.getAttribute('data-generated-ts')); // epoch seconds
        if (!generatedTs) { return; }
        var ageMinutes = (Date.now() / 1000 - generatedTs) / 60;
        if (ageMinutes >= threshold) {
            status.style.display = '';
        }
    }

    function applyAlmanac(alm) {
        if (!alm) { return; }

        // Sun: pyephem (hasExtras) layout uses #alm-sun-rise/#alm-sun-set;
        // the basic layout uses #alm-sunrise/#alm-sunset. Fill whichever exists.
        if (document.getElementById('alm-sun-rise') && alm.sun) {
            setText('alm-sun-rise', alm.sun.rise);
            setText('alm-sun-set', alm.sun.set);
        } else {
            setText('alm-sunrise', alm.sunrise);
            setText('alm-sunset', alm.sunset);
        }

        // Moon (pyephem only)
        if (alm.moon) {
            setText('alm-moon-rise', alm.moon.rise);
            setText('alm-moon-set', alm.moon.set);
        }
        setText('alm-moon-fullness', alm.moon_fullness);

        if (alm.moon_index !== undefined && alm.moon_index !== null) {
            var iconHost = document.getElementById('alm-moon-icon');
            if (iconHost) {
                var cls = MOON_ICONS[alm.moon_index] || MOON_ICON_DEFAULT;
                // Build via DOM methods (cls is from a fixed allowlist) — no innerHTML.
                var icon = document.createElement('i');
                icon.className = 'wi ' + cls + ' mr-1';
                icon.style.opacity = '.75';
                iconHost.textContent = '';
                iconHost.appendChild(icon);
            }
        }

        if (alm.moon_phase) {
            var phaseHost = document.getElementById('alm-moon-phase');
            if (phaseHost) {
                // Bootstrap moves the original title into data-original-title
                // once the tooltip is initialised, so update both.
                phaseHost.setAttribute('title', alm.moon_phase);
                phaseHost.setAttribute('data-original-title', alm.moon_phase);
            }
        }
    }

    function applyUptime(data) {
        // Runs on every page (incl. current) so all pages agree.
        if (!data.uptime) { return; }
        setText('footer-station-uptime', data.uptime.station);
        setText('footer-server-uptime', data.uptime.server);
    }

    function apply(data) {
        applyUptime(data);
        if (!isNonCurrent) { return; } // current page: uptime is all we sync

        setText('header-date', data.date);
        setText('footer-year', data.year);
        if (data.versions) {
            setText('footer-weewx-version', data.versions.weewx);
            setText('footer-skin-version', data.versions.skin);
        }
        if (data.today) {
            window.NX_STOP_DATE = data.today;
            if (window.nxUpdateStopDate) {
                window.nxUpdateStopDate(data.today);
            }
        }
        if (data.archive_start_date) {
            window.NX_START_DATE = data.archive_start_date;
            if (window.nxUpdateStartDate) {
                window.nxUpdateStartDate(data.archive_start_date);
            }
        }
        applyAlmanac(data.almanac);
    }

    function revealLive() {
        // Undo the anti-flash guard from header.inc: reveals the live fields once
        // their values have been applied (or, on failure, the server fallback).
        document.documentElement.classList.remove('nx-live-pending');
    }

    function run() {
        // The calendar icon depends only on baked data + the browser clock, so
        // reveal it right away — no need to wait on live.json.
        revealGeneratedIcon();

        // Reuse the early fetch kicked off in head.inc when available, so the
        // request has been in flight (and usually resolved) while the heavy JS
        // libs downloaded. Fall back to fetching here if it isn't present.
        var pending = window.NX_LIVE_DATA ||
            fetch((window.NX_BASE_PATH || '') + 'live.json?_=' + new Date().getTime(), { cache: 'no-store' })
                .then(function (resp) {
                    if (!resp.ok) { throw new Error('live.json HTTP ' + resp.status); }
                    return resp.json();
                });

        pending
            .then(function (data) {
                apply(data);
                revealLive(); // reveal with fresh values applied — no stale flash
            })
            .catch(function () {
                // Keep server-rendered fallbacks, then reveal them.
                revealLive();
            });
    }

    // js.inc (and therefore this script) is included after header.inc + footer.inc,
    // so every node we touch already exists. Run now instead of waiting for
    // DOMContentLoaded — that wait would delay the update until the charts and the
    // MQTT CDN script finish loading.
    run();
})();
