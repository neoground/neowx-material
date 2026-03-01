#encoding UTF-8
## +-----------------------------------------------------------------------------------+
## |    graph_line_archive_config.inc             JS config for default line charts    |
## +-----------------------------------------------------------------------------------+

#set $allowMouseWheelZoom = "false"
#if $Extras.Appearance.get('allowMouseWheelZoom') == "true"
    #set $allowMouseWheelZoom = "true"
#end if

#set $defaultChartBehavior = "zoom"
#if $Extras.Appearance.get('defaultChartBehavior') == "pan"
    #set $defaultChartBehavior = "pan"
#end if

chart: {
    type: 'line',
    zoom: {
        enabled: false,
        type: 'x',
        autoScaleYaxis: false,
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
},

dataLabels: {
    enabled: false
},

stroke: {
    width: 0,
},

plotOptions: {
    line: {
        borderRadius: 2,
    }
},

fill: {
    opacity: 0.7
},

markers: {
    size: 4,
    strokeWidth: 1,
    strokeOpacity: 0.5,
    strokeDashArray: 0,
    fillOpacity: 0.7,
    shape: "circle",
    radius: 1,
},

xaxis: {
    type: 'datetime',
    tickAmount: 8,
    trim: true,
    labels: {
        hideOverlappingLabels: true,
        tickAmount: 8,
        rotateAlways: false,
        hideOverlappingLabels: true,
        showDuplicates: false,
        trim: true,
        formatter: function(val, timestamp) {
            return moment.unix(timestamp).format("$Extras.Formatting.datetime_graph_archive");
        }
    }
},

grid: {
    show: true,
    strokeDashArray: 0,
    position: 'back',
    xaxis: {
        lines: {
            show: true
        }
    },
    yaxis: {
        lines: {
            show: true
        }
    },
},

tooltip: {
    x: {
        format: "$Extras.Formatting.datetime_graph_tooltip"
    }
},
