#encoding UTF-8
## ---------------------------------------------------------------------------
## Resolve the per-page x-axis tick spec and emit the matching ApexCharts
## datetime config. Keys live in [[Charts]]:
##     tick_style_<page>  ->  tick_style  ->  auto
## Supported values:
##   auto            (default) no tickAmount - ApexCharts chooses the count
##   fixed:N (or N)  exactly N tick intervals (N+1 evenly-spaced labels)
##   align           aligned ticks, auto day/month by span - handled per page
##                   (it needs the page's time span), so nothing is emitted here
## $page is one of: day (index + yesterday), week, month, year, telemetry.
## ---------------------------------------------------------------------------
#set $_spec = $Extras.Charts.get('tick_style_' + $page, $Extras.Charts.get('tick_style', 'auto'))
#if isinstance($_spec, list) or not $_spec
    #set $_spec = 'auto'
#end if
#set $_spec = $_spec.strip().lower()
#set $_num = ''
#if $_spec.startswith('fixed:')
    #set $_num = $_spec[6:].strip()
#else if $_spec.isdigit()
    #set $_num = $_spec
#end if
#if $_num.isdigit() and int($_num) > 0
    tickAmount: $int($_num),
#else if $_spec.startswith('fixed:') or ($_spec and $_spec != 'auto' and not $_spec.startswith('align') and not $_spec.isdigit())
    // neowx-material: ignored unparseable tick_style "$_spec" on page $page - using auto
#end if
