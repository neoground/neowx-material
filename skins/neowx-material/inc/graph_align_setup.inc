#encoding UTF-8
## ---------------------------------------------------------------------------
## Resolve the page's tick spec into the align globals (once per page). Set
## $align_start and $align_stop (epoch seconds) BEFORE including. This sets:
##   $pg_align_mid   - True when tick_style_<page> is 'align' / 'align:N'
##   $align_count    - N from align:N, else 0 (auto)
##   $pg_align_count - stable snapshot of $align_count (for per-chart overrides)
## graph_align_axis.inc then turns start/stop/count into min/max/tickAmount
## (auto hour-/day-/month-granularity by span - see that file). Keyed off
## $page, so it works on every chart page including the archives.
## ---------------------------------------------------------------------------
#set global $pg_align_mid = False
#set global $align_count = 0
#set $_spec = $Extras.Charts.get('tick_style_' + $page, $Extras.Charts.get('tick_style', 'auto'))
#set $_spec = $_spec.strip().lower() if (not isinstance($_spec, list) and $_spec) else 'auto'
#set $_parts = $_spec.split(':')
#if len($_parts) >= 1 and $_parts[0] == 'align'
    #set global $pg_align_mid = True
    #set global $align_count = int($_parts[1]) if (len($_parts) >= 2 and $_parts[1].strip().isdigit()) else 0
    #if len($_parts) >= 2 and $_parts[1].strip() and not $_parts[1].strip().isdigit()
    // neowx-material: ignored tick count in tick_style "$_spec" on page $page - using auto count
    #end if
#end if
## Snapshot the page's align count so per-chart overrides (chart_tick_axis.inc)
## can restore it for charts that don't set their own tick_style.
#set global $pg_align_count = $align_count
