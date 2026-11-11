#encoding UTF-8
## +-----------------------------------------------------------------------------------+
## |    graph_line_config.inc             JS config for default line charts    |
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
                categoryFormatter(x) {
                    return new Date(x).getTime()
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
    #include "inc/graph_tick_amount.inc"
    trim: true,
    labels: {
        hideOverlappingLabels: true,
        rotateAlways: false,
        hideOverlappingLabels: true,
        showDuplicates: false,
        trim: true,
        formatter: function(val, timestamp) {
            #set global $gdf_base = 'datetime_graph_label'
            #include "inc/graph_date_format.inc"
            var fmt = "$gdf_fmt";
            // If the format uses uppercase MM for minutes (common mistake), convert
            // only occurrences of ":MM" to Moment's minutes token ":mm" so we don't
            // change months (MM) to minutes.
            fmt = fmt.replace(/:MM/g, ':mm');
            return moment(timestamp).format(fmt);
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
        formatter: function(val) {
            #set global $gdf_base = 'datetime_graph_tooltip'
            #include "inc/graph_date_format.inc"
            var fmt = "$gdf_fmt";
            fmt = fmt.replace(/:MM/g, ':mm');
            return moment(val).format(fmt);
        }
    }
},
