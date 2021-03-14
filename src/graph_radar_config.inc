#errorCatcher Echo
#encoding UTF-8

chart: {
    type: 'radar',
},
fill: {
    opacity: 0.1
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