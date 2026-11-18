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
    #include "inc/graph_tick_amount.inc"
    labels: {
        formatter: function(val, timestamp) {
            #set global $gdf_base = 'datetime_graph_label'
            #include "inc/graph_date_format.inc"
            var fmt = "$gdf_fmt";
            // If the format uses uppercase MM for minutes (common mistake), convert
            // only occurrences of ":MM" to Moment's minutes token ":mm" so we don't
            // change months (MM) to minutes.
            fmt = fmt.replace(/:MM/g, ':mm');
            return moment(timestamp).format(fmt);
        }
    }
},
tooltip: {
    x: {
        formatter: function(val) {
            #set global $gdf_base = 'datetime_graph_tooltip'
            #include "inc/graph_date_format.inc"
            var fmt = "$gdf_fmt";
            // convert minute token if needed (':MM' -> ':mm')
            fmt = fmt.replace(/:MM/g, ':mm');
            return moment(val).format(fmt);
        }
    }
},
