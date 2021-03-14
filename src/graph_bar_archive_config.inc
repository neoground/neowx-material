#errorCatcher Echo
#encoding UTF-8

chart: {
    type: 'bar'
},
fill: {
    opacity: 0.1
},
stroke: {
    curve: 'smooth'
},
xaxis: {
    type: 'datetime',
    hideOverlappingLabels: true,
    tickAmount: 8,
    labels: {
        formatter: function(val, timestamp) {
            return moment.unix(timestamp).format("$Extras.Formatting.datetime_graph_archive");
        }
    }
},
tooltip: {
    x: {
        format: "$Extras.Formatting.datetime_graph_tooltip"
    }
},