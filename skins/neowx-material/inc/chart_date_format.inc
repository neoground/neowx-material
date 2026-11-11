#encoding UTF-8
## ---------------------------------------------------------------------------
## Resolve $cc_label_fmt / $cc_tooltip_fmt (globals) for a custom chart's date
## overrides. Include from the customChart* loop AFTER setting (as #set global):
##   $cc_name  - the customChart* key (e.g. customChartOutTemp)
##   $cc_scope - page scope key: current / yesterday / week / month / year
## A value is a [[Formatting]] named constant if defined, else a literal format.
## ---------------------------------------------------------------------------
#set $_cc   = $Extras.Appearance.get($cc_name, {})
#set $_page = $_cc.get($cc_scope, {})
#set $_lk = $_page.get('datetime_label_format',   $_cc.get('datetime_label_format',   ''))
#set $_tk = $_page.get('datetime_tooltip_format', $_cc.get('datetime_tooltip_format', ''))
#set $_lk = ', '.join($_lk) if isinstance($_lk, list) else $_lk
#set $_tk = ', '.join($_tk) if isinstance($_tk, list) else $_tk
#set $_lr = $Extras.Formatting.get($_lk, $_lk) if $_lk else ''
#set $_tr = $Extras.Formatting.get($_tk, $_tk) if $_tk else ''
#set global $cc_label_fmt   = ', '.join($_lr) if isinstance($_lr, list) else $_lr
#set global $cc_tooltip_fmt = ', '.join($_tr) if isinstance($_tr, list) else $_tr
