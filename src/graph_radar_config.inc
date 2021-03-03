#errorCatcher Echo
#encoding UTF-8

  theme: {
    mode: theme_mode,
  },
  chart: {
    type: 'radar',
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
    width: 2
  },
  xaxis: {
    categories: ['N', 'NE', 'E', 'SE', 'S', 'SW', 'W', 'NW']
  },
  tooltip: {
    x: {
        formatter: function(value, { series, seriesIndex, dataPointIndex, w }) {
            return value
        }
    }
  },