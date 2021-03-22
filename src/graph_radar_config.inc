#encoding UTF-8
## +-------------------------------------------------------------------------+
## |    graph_radar_config.inc         JS config for default radar charts    |
## +-------------------------------------------------------------------------+

chart: {
    type: 'radar',
},
fill: {
    opacity: 0.1
},
xaxis: {
    categories: [
      '$Extras.Translations[$Extras.language]["hemispheres"][0]',
      '$Extras.Translations[$Extras.language]["hemispheres"][1]',
      '$Extras.Translations[$Extras.language]["hemispheres"][2]',
      '$Extras.Translations[$Extras.language]["hemispheres"][3]',
      '$Extras.Translations[$Extras.language]["hemispheres"][4]',
      '$Extras.Translations[$Extras.language]["hemispheres"][5]',
      '$Extras.Translations[$Extras.language]["hemispheres"][6]',
      '$Extras.Translations[$Extras.language]["hemispheres"][7]'
    ]
},
tooltip: {
    x: {
        formatter: function(value, { series, seriesIndex, dataPointIndex, w }) {
            return value
        }
    }
},