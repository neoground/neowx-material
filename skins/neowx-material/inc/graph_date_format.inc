#encoding UTF-8
## ---------------------------------------------------------------------------
## Resolve a per-page graph date format into the global $gdf_fmt.
## Set before including (as #set global):
##   $gdf_base - base key: 'datetime_graph_label' or 'datetime_graph_tooltip'
## Looks up the per-page key <base>_<page> (uses upstream's $page), falling back
## to <base>. The value may be a [[Formatting]] named constant or a literal format.
## ---------------------------------------------------------------------------
#set $_gv = $Extras.Formatting.get($gdf_base + '_' + $page, $Extras.Formatting.get($gdf_base, ''))
#set $_gv = ', '.join($_gv) if isinstance($_gv, list) else $_gv
#set $_gf = $Extras.Formatting.get($_gv, $_gv)
#set global $gdf_fmt = ', '.join($_gf) if isinstance($_gf, list) else $_gf
