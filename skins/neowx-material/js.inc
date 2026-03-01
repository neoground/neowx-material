#encoding UTF-8
## +-------------------------------------------------------------------------+
## |    js.inc                   Common javascript code for all templates    |
## +-------------------------------------------------------------------------+

## jQuery
<script type="text/javascript" src="js/jquery.min.js"></script>
## Bootstrap tooltips / JS
<script type="text/javascript" src="js/popper.min.js"></script>
<script type="text/javascript" src="js/bootstrap.min.js"></script>
## MDB core JavaScript
<script type="text/javascript" src="js/mdb.min.js"></script>
## Vendor JS
#if $lang == "en"
    <script src="js/vendor/moment.min.js"></script>
#else
    <script src="js/vendor/moment-with-locales.min.js"></script>
    <script type="text/javascript">moment.locale('$lang')</script>
#end if
<script src="js/vendor/apexcharts/apexcharts.min.js"></script>
## NeoWX Material App
<script type="text/javascript" src="js/app.js"></script>

<script type="text/javascript">
    // Show/update status label only if configured threshold > 0 in skin.conf
    const isArchivePage = "$active_nav" === "archive"; // do not show status on archive page
    if (!isArchivePage && $Extras.Header.offline_threshold_minutes > 0) {
        let lastUpdateTimestampString = '$current.dateTime.raw';
        if (lastUpdateTimestampString === "" ) {
            lastUpdateTimestampString = "4891363200"; // Fallback to 2125 if not available
        }
        const lastUpdateTimestamp =  Number(lastUpdateTimestampString) ;  // in seconds
        const nowTimestamp = new Date() / 1000;             // now in seconds
        const diffMinutes = (nowTimestamp - lastUpdateTimestamp) / 60; // Difference in minutes

        const statusLabel = document.getElementById("status-label");
        if (statusLabel && diffMinutes > $Extras.Header.offline_threshold_minutes) {
            statusLabel.textContent = "$gettext('OFFLINE')";
            } else {
            // enable if you want to show also ONLINE (which should always be the excepted state)
            // statusLabel.textContent = "$gettext('ONLINE')";
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

## Global apexcharts config
<script type="text/javascript">
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
                export: {
                    csv: {
                        headerCategory: 'DateTime',
                        dateFormatter(timestamp) {
                            return moment.unix(timestamp).toISOString()
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
        },
    }

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
