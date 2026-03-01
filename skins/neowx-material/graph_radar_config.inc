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
        '$Extras.hemispheres[0]',
        '$Extras.hemispheres[1]',
        '$Extras.hemispheres[2]',
        '$Extras.hemispheres[3]',
        '$Extras.hemispheres[4]',
        '$Extras.hemispheres[5]',
        '$Extras.hemispheres[6]',
        '$Extras.hemispheres[7]'
    ]
},
tooltip: {
    x: {
        formatter: function(value, { series, seriesIndex, dataPointIndex, w }) {
            return value
        }
    }
},