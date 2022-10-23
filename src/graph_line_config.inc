#encoding UTF-8
## +---------------------------------------------------------------------------+
## |    graph_line_config.inc             JS config for default line charts    |
## +---------------------------------------------------------------------------+

chart: {
    type: 'line',
    zoom: {
        enabled: false,
        type: 'x',  
        autoScaleYaxis: false,
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
            return moment.unix(timestamp).format("$Extras.Formatting.datetime_graph_label");
        }
    }
},

yaxis: {
    max: 359,
    min: 0,
    tickAmount: 5,
    forceNiceScale: true,
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
