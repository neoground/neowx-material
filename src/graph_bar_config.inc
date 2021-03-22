#encoding UTF-8
## +-------------------------------------------------------------------------+
## |    graph_bar_config.inc             JS config for default bar charts    |
## +-------------------------------------------------------------------------+

chart: {
    type: 'bar'
},
fill: {
    opacity: 0.1
},
xaxis: {
    type: 'datetime',
    hideOverlappingLabels: true,
    tickAmount: 8,
    labels: {
        formatter: function(val, timestamp) {
            return moment.unix(timestamp).format("$Extras.Formatting.datetime_graph_label");
        }
    }
},
tooltip: {
    x: {
        format: "$Extras.Formatting.datetime_graph_tooltip"
    }
},