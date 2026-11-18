#encoding UTF-8
## ---------------------------------------------------------------------------
## Emit the x-axis tick portion for a CUSTOM chart, honoring an optional
## per-chart override (from chart_tick_setup.inc) over the page setting.
## Include INSIDE the chart's "xaxis: { ...graph_<type>_config.xaxis, ... }"
## object, after the spread. Expects (globals):
##   $cc_tick_set/$cc_tick_align/$cc_tick_count/$cc_tick_fixed - chart override
##   $pg_align_mid/$pg_align_count                             - page setting
##   $align_start/$align_stop                                  - page time span
## graph_align_axis.inc reads $align_count, so it is set explicitly in each
## align branch to avoid a previous chart's value leaking into this one.
## ---------------------------------------------------------------------------
#if $cc_tick_set
    #if $cc_tick_align
        #set global $align_count = $cc_tick_count
        #include "inc/graph_align_axis.inc"
    #else if $cc_tick_fixed > 0
        tickAmount: $cc_tick_fixed,
    #else
        ## explicit auto: drop any inherited tickAmount so ApexCharts chooses
        tickAmount: undefined,
    #end if
#else if $pg_align_mid
    ## no per-chart override: follow the page's align setting
    #set global $align_count = $pg_align_count
    #include "inc/graph_align_axis.inc"
#end if
