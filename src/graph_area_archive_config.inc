theme: {
  mode: theme_mode,
},
chart: {
  type: 'area',
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
  locales: [
    #include raw "js/vendor/apexcharts/locales/" + $Extras.language + ".json"
  ],
  defaultLocale: '$Extras.language',
},
dataLabels: {
  enabled: false
},
markers: {
  size: 0,
},
fill: {
  type: 'gradient',
  gradient: {
    shadeIntensity: 1,
    inverseColors: false,
    opacityFrom: 0.6,
    opacityTo: 0,
    stops: [0, 75, 100]
  },
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
      return moment.unix(timestamp).format("$Extras.Formatting.datetime_graph_archive");
    }
  }
},
tooltip: {
  x: {
    format: "$Extras.Formatting.datetime_graph_tooltip"
  }
},