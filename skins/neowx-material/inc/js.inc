#encoding UTF-8
## +-------------------------------------------------------------------------+
## |    js.inc                   Common javascript code for all templates    |
## +-------------------------------------------------------------------------+

#if not $varExists('base_path')
    #set $base_path = ''
#end if

## jQuery
<script type="text/javascript" src="${base_path}js/jquery.min.js"></script>
## Bootstrap tooltips / JS
<script type="text/javascript" src="${base_path}js/popper.min.js"></script>
<script type="text/javascript" src="${base_path}js/bootstrap.min.js"></script>
## MDB core JavaScript
<script type="text/javascript" src="${base_path}js/mdb.min.js"></script>
## Vendor JS
#if $lang == "en"
    <script src="${base_path}js/vendor/moment.min.js"></script>
#else
    <script src="${base_path}js/vendor/moment-with-locales.min.js"></script>
    <script type="text/javascript">moment.locale('$lang')</script>
#end if
<script src="${base_path}js/vendor/apexcharts/apexcharts.min.js"></script>
## NeoWX Material chart helpers (shared date formatter)
<script type="text/javascript" src="${base_path}js/neowx-date-format.js?v=$Extras.version"></script>
## NeoWX Material App
<script type="text/javascript" src="${base_path}js/app.js?v=$Extras.version"></script>
## Keeps header/footer chrome current on non-current (incl. frozen archive) pages
<script type="text/javascript" src="${base_path}js/neowx-live.js?v=$Extras.version"></script>
## Interactive date, week, month, and year navigation pickers
<script type="text/javascript" src="${base_path}js/neowx-date-nav.js?v=$Extras.version"></script>

<script type="text/javascript">
    // Show/update status label only if configured threshold > 0 in skin.conf
    const showOfflineLabel = "$active_nav" === "current"; // Show offline-state only on current page
    if (showOfflineLabel && $Extras.Header.offline_threshold_minutes > 0) {
        let lastUpdateTimestampString = '$current.dateTime.raw';
        if (lastUpdateTimestampString === "" ) {
            lastUpdateTimestampString = "4891363200"; // Fallback to 01.01.2125 if not available
        }
        const lastUpdateTimestamp =  Number(lastUpdateTimestampString) ;  // in seconds
        const nowTimestamp = new Date() / 1000;             // now in seconds
        const diffMinutes = (nowTimestamp - lastUpdateTimestamp) / 60; // Difference in minutes

        const statusLabel = document.getElementById("status-label");
        if (statusLabel && diffMinutes > $Extras.Header.offline_threshold_minutes) {
            statusLabel.textContent = "$gettext('OFFLINE')";
            statusLabel.style.display = 'inline-block';
        }
    }
</script>

#set $allowMouseWheelZoom = "false"
#if $Extras.Appearance.get('allowMouseWheelZoom') == "true"
    #set $allowMouseWheelZoom = "true"
#end if

#set $defaultChartBehavior = "zoom"
#if $Extras.Appearance.get('defaultChartBehavior') == "pan"
    #set $defaultChartBehavior = "pan"
#end if

## Default-on, so an unrecognised value must not leave the feature switched on
## with no way to turn it off. Normalised the way bin/user/panelorder.py reads
## enablePanels, rather than the exact-match the opt-in settings above use.
#set $enableChartPopup = "true"
#if str($Extras.Appearance.get('enableChartPopup', 'true')).strip().lower() in ('false', 'no', '0')
    #set $enableChartPopup = "false"
#end if

## Where the chart buttons sit - see chart_toolbar in skin.conf. Anything not
## recognised falls back to the ApexCharts placement rather than to a guess.
#set $chartToolbar = str($Extras.Appearance.get('chart_toolbar', 'chart_default')).strip().lower()
#if $chartToolbar not in ('chart_default', 'separate', 'align')
    #set $chartToolbar = "chart_default"
#end if
## "separate" moves the expand button out of the toolbar, so with the pop-up
## switched off there is nothing left for it to do.
#if $chartToolbar == "separate" and $enableChartPopup == "false"
    #set $chartToolbar = "chart_default"
#end if

## Strings for js/app.js, which is a plain .js file with no template access.
## Emitted after app.js is loaded above, so app.js must only read these lazily
## (at click or mount time) - anything read at its top level would still be
## undefined.
<script type="text/javascript">
    window.NEOWX_TEXTS = {
        close: "$gettext('close')",
        // Used when app.js builds the expand button itself, in "separate" mode.
        expand_chart: "$gettext('expand_chart')"
    };

    // Where a chart's buttons go: chart_default / separate / align. Read
    // lazily, at mount time - see neowxChartToolbarMode in js/app.js.
    window.NEOWX_CHART_TOOLBAR = "$chartToolbar";

    // Card headings are <h5 class="h5-responsive $Extras.color-text">. The
    // pop-up builds its title in JS, so it is given the same classes from here
    // rather than hard-coded ones - the colour follows [Extras] color in
    // skin.conf and would otherwise drift on any theme but the default.
    window.NEOWX_TITLE_CLASS = "h5-responsive $Extras.color-text";
</script>

## Global apexcharts config
<script type="text/javascript">
    // Charts drawn in "align" mode (the only ones with xaxis.min+max set) keep
    // their calendar alignment through a zoom: record their original aligned
    // axis at mount, re-snap on zoom, and restore the original on reset zoom.
    // fixed/auto charts are left alone. Granularity follows the zoom width,
    // same as graph_align_axis.inc server-side: hour tier (<=2 days), day tier
    // (<=62 days), else even spacing.
    var NEOWX_ALIGNED = {};
    function neowxMountedHandler(chartContext) {
        try {
            var x = chartContext.w.config.xaxis;
            if (x && x.min != null && x.max != null) {
                NEOWX_ALIGNED[chartContext.w.globals.chartID] =
                    { min: x.min, max: x.max, tickAmount: x.tickAmount };
            }
        } catch (e) {}
        // Puts the chart's buttons where chart_toolbar asks for them. Lives
        // in js/app.js, which is loaded above.
        neowxSetupChartToolbar(chartContext);
    }
    function neowxAlignAxis(minMs, maxMs) {
        var secs = (maxMs - minMs) / 1000;
        if (secs < 3600) { secs = 3600; }
        if (secs <= 172800) {
            var hours = Math.ceil(secs / 3600);
            var hs = Math.max(1, Math.round(hours / 6));
            var ht = Math.ceil(hours / hs);
            var h0 = moment(minMs).startOf('hour');
            return { min: h0.valueOf(), max: h0.clone().add(ht * hs, 'hours').valueOf(), tickAmount: ht };
        }
        var days = Math.ceil(secs / 86400);
        if (days <= 62) {
            var ds = Math.max(1, Math.round(days / 12));
            var dt = Math.ceil(days / ds);
            var d0 = moment(minMs).startOf('day');
            return { min: d0.valueOf(), max: d0.clone().add(dt * ds, 'days').valueOf(), tickAmount: dt };
        }
        return { min: minMs, max: maxMs, tickAmount: 12 };
    }
    var _neowxZooming = false;
    var _neowxResetting = false;
    function neowxZoomHandler(chartContext, opts) {
        if (_neowxZooming || _neowxResetting) { return; }
        try { if (!NEOWX_ALIGNED[chartContext.w.globals.chartID]) { return; } } catch (e) { return; }
        if (!opts || !opts.xaxis || opts.xaxis.min == null || opts.xaxis.max == null) { return; }
        var a = neowxAlignAxis(opts.xaxis.min, opts.xaxis.max);
        _neowxZooming = true;
        try {
            var p = chartContext.updateOptions(
                { xaxis: { min: a.min, max: a.max, tickAmount: a.tickAmount } }, false, false);
            if (p && p.then) { p.then(function(){ _neowxZooming = false; }, function(){ _neowxZooming = false; }); }
            else { _neowxZooming = false; }
        } catch (e) { _neowxZooming = false; }
    }
    function neowxResetDone() { _neowxZooming = false; _neowxResetting = false; }
    function neowxBeforeResetHandler(chartContext) {
        // Restore the ORIGINAL aligned min/max/tickAmount captured at mount. beforeResetZoom only
        // positions min/max, so re-apply the exact tickAmount right after via updateOptions.
        try {
            var o = NEOWX_ALIGNED[chartContext.w.globals.chartID];
            if (!o) { return; }
            _neowxResetting = true;
            setTimeout(function() {
                try {
                    _neowxZooming = true;
                    var p = chartContext.updateOptions(
                        { xaxis: { min: o.min, max: o.max, tickAmount: o.tickAmount } }, false, false);
                    if (p && p.then) { p.then(neowxResetDone, neowxResetDone); }
                    else { neowxResetDone(); }
                } catch (e) { neowxResetDone(); }
            }, 0);
            return { xaxis: { min: o.min, max: o.max } };
        } catch (e) {}
    }

    var config_mode = '${Extras.Appearance.mode}';
    var theme_mode = 'light';
    if (window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches) {
        theme_mode = 'dark';
    }
    // Override mode by config
    if(config_mode != "auto") {
        theme_mode = config_mode;
    }

    window.Apex = {
        theme: {
            mode: theme_mode,
            palette: '$Extras.Charts.palette'
        },
        dataLabels: {
            enabled: false
        },
        stroke: {
            curve: '$Extras.Charts.stroke_curve',
            width: $Extras.Charts.stroke_width
        },
        markers: {
            size: $Extras.Charts.markers_size,
        },
        chart: {
            height: $Extras.Charts.height,
            stacked: false,
            background: theme_mode === 'dark' ? '#444' : '#ffffff',
            zoom: {
                type: 'x',
                enabled: true,
                autoScaleYaxis: true,
                allowMouseWheelZoom: $allowMouseWheelZoom,
            },
            toolbar: {
                autoSelected: '$defaultChartBehavior',
#if $enableChartPopup == "true" and $chartToolbar != "separate"
                // Opens a copy of the chart in a pop-up window. Set as a global
                // default so every chart gets the button; neowxChartPopup and
                // the icon both live in js/app.js, which is loaded above. In
                // "separate" mode app.js puts this button in the card's corner
                // instead, so it is left out of the toolbar here.
                tools: {
                    customIcons: [{
                        icon: NEOWX_EXPAND_ICON,
                        title: "$gettext('expand_chart')",
                        class: 'neowx-chart-popup-icon',
                        click: neowxChartPopup
                    }]
                },
#end if
                export: {
                    csv: {
                        headerCategory: 'DateTime',
                        categoryFormatter(x) {
                            return new Date(x).getTime()
                        }
                    }
                }
            },
            locales: [
                #include raw "js/vendor/apexcharts/locales/" + $lang + ".json"
            ],
            defaultLocale: '$lang',
            animations: {
                enabled: false
            },
            events: {
                mounted: neowxMountedHandler,
                // ApexCharts rebuilds the toolbar on a re-render, so the
                // rebuilt one has to be put back where chart_toolbar asks for
                // it. (A heading that changes height is covered separately, by
                // the resize listener and ResizeObserver in js/app.js.)
                updated: neowxSetupChartToolbar,
                zoomed: neowxZoomHandler,
                beforeResetZoom: neowxBeforeResetHandler
            },
        },
    }

    // --- neowxChartColors ---------------------------------------------------
    // Resolves skin.conf per-series color references into concrete colors.
    // Accepted per entry: "N" (Nth color of the ACTIVE palette, 1-based),
    // "paletteN:M" (Mth color of built-in palette N), a literal CSS color,
    // or "default" (keep the active palette's positional color).
    // Palette tables mirror getThemePalettes() in
    // js/vendor/apexcharts/apexcharts.js - keep in sync when upgrading the
    // vendored ApexCharts build.
    var NEOWX_PALETTES = {
        palette1: ["#008FFB", "#00A86F", "#CA8501", "#FF4560", "#846DD5"],
        palette2: ["#6978CB", "#039DE2", "#49A84D", "#B39105", "#D68000"],
        palette3: ["#209FCC", "#648291", "#D4526E", "#0FA783", "#A19285"],
        palette4: ["#2FA59D", "#73A20B", "#099DE1", "#FD5D5D", "#648291"],
        palette5: ["#2B908F", "#F56566", "#2EAB16", "#FA4443", "#1EA2BD"],
        palette6: ["#449DD1", "#F86624", "#EA3A4A", "#9C63D1", "#899E2A"],
        palette7: ["#DF475B", "#1B998B", "#7E75B7", "#F46036", "#B1911B"],
        palette8: ["#9C63D1", "#F86624", "#B38F04", "#EA3A4A", "#2FA2B3"],
        palette9: ["#98776F", "#A19285", "#A8705E", "#BA6560", "#A0927F"],
        palette10: ["#C91EFF", "#A94BFD", "#6C6AFE", "#2983FF", "#009ED8"]
    };

    // ref must be a string (callers pass String(...).trim()).
    function neowxResolveColorRef(ref, activePalette, fallback, chartName) {
        if (!ref || ref.toLowerCase() === 'default') { return fallback; }
        if (/^\d+$/.test(ref)) {
            var n = parseInt(ref, 10);
            if (n >= 1 && n <= activePalette.length) { return activePalette[n - 1]; }
            console.warn('neowx-material: chart "' + chartName + '": color slot ' + ref +
                ' out of range (1-' + activePalette.length + ') - using default');
            return fallback;
        }
        // No CSS color syntax contains ':', so this never shadows a literal color.
        if (ref.indexOf(':') !== -1) {
            var m = /^palette(\d+):(\d+)$/i.exec(ref);
            var pal = m ? NEOWX_PALETTES['palette' + m[1]] : null;
            var slot = m ? parseInt(m[2], 10) : 0;
            if (pal && slot >= 1 && slot <= pal.length) { return pal[slot - 1]; }
            if (pal) {
                console.warn('neowx-material: chart "' + chartName + '": color slot ' + m[2] +
                    ' out of range (1-' + pal.length + ') - using default');
            } else {
                console.warn('neowx-material: chart "' + chartName + '": unrecognized palette reference "' +
                    ref + '" - using default');
            }
            return fallback;
        }
        if (typeof CSS !== 'undefined' && CSS.supports && !CSS.supports('color', ref)) {
            console.warn('neowx-material: chart "' + chartName + '": invalid color "' +
                ref + '" - using default');
            return fallback;
        }
        return ref;
    }

    function neowxChartColors(refs, seriesCount, chartName) {
        var active = NEOWX_PALETTES.palette1;
        try {
            var palName = window.Apex.theme.palette;
            if (NEOWX_PALETTES[palName]) { active = NEOWX_PALETTES[palName]; }
        } catch (e) {}
        var out = [];
        for (var i = 0; i < seriesCount; i++) {
            var fallback = active[i % active.length];
            var ref = (refs && i < refs.length && refs[i] != null) ? String(refs[i]).trim() : '';
            out.push(neowxResolveColorRef(ref, active, fallback, chartName));
        }
        return out;
    }
    // --- end neowxChartColors -----------------------------------------------

    ## Ordinals conversion for W0CHP's custom WindDir Charts:
    function getOrdinalDirection(degrees) {
        var sectors = [
            '$Extras.hemispheres[0]',
            '$Extras.hemispheres[1]',
            '$Extras.hemispheres[2]',
            '$Extras.hemispheres[3]',
            '$Extras.hemispheres[4]',
            '$Extras.hemispheres[5]',
            '$Extras.hemispheres[6]',
            '$Extras.hemispheres[7]'
        ];
        var index = Math.round(degrees / (360 / sectors.length));
        return sectors[index % sectors.length];
    }
</script>
