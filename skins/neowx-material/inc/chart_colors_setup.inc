#encoding UTF-8
## ---------------------------------------------------------------------------
## Resolve a custom chart's optional per-series color references into the
## chart-scoped global $cc_color_refs. Include from the customChart* loop
## AFTER setting (as #set global):
##   $cc_name  - the customChart* key (e.g. customChartOutTemp)
##   $cc_scope - per-page subsection name: current (index) / yesterday / week /
##               month / year
## The value (page subsection -> top-level) is a list of per-series entries:
## a bare active-palette slot number, paletteN:M, a literal CSS color, or
## "default". Entries are cleaned here; resolution to concrete colors happens
## client-side in neowxChartColors (js.inc). Quotes and backslashes are
## stripped because entries are emitted inside JS string literals.
## Output (global): $cc_color_refs - list of cleaned reference strings,
## empty when the chart configures no colors.
## ---------------------------------------------------------------------------
#set global $cc_color_refs = []
#set $_ccc      = $Extras.Appearance.get($cc_name, {})
#set $_ccc_page = $_ccc.get($cc_scope, {})
#set $_ccc_raw  = $_ccc_page.get('colors', $_ccc.get('colors', []))
#if isinstance($_ccc_raw, list)
    #set global $cc_color_refs = [str(c).strip().replace('"', '').replace('\\', '') for c in $_ccc_raw if str(c).strip()]
#else
    #set global $cc_color_refs = [c.strip().replace('"', '').replace('\\', '') for c in str($_ccc_raw).split(',') if c.strip()]
#end if
