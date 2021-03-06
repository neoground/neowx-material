#errorCatcher Echo
#encoding UTF-8

  theme: {
    mode: theme_mode,
    palette: '$Extras.chartcolor'
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
      autoSelected: 'zoom',
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
        return moment.unix(timestamp).format("$Extras.Formatting.datetime_graph_archive");
      }
    }
  },
  tooltip: {
    x: {
      format: "$Extras.Formatting.datetime_graph_tooltip"
    }
  },