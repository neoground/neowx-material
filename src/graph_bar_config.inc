  theme: {
    mode: theme_mode,
  },
  chart: {
    type: 'bar',
    stacked: false,
    height: 300,
    zoom: {
      type: 'x',
      enabled: true,
      autoScaleYaxis: true
    },
    toolbar: {
      autoSelected: 'zoom'
    },
  },
  dataLabels: {
    enabled: false
  },
  markers: {
    size: 0,
  },
  fill: {
    opacity: 0.1
  },
  stroke: {
    curve: 'smooth',
    width: 2
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