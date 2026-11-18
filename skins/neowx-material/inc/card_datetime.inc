#encoding UTF-8
## ---------------------------------------------------------------------------
## Resolve $card_datetime (global) for value-card min/max times on a page.
## Include once per page AFTER setting (as #set global):
##   $card_scope   - page scope (= $page): index/yesterday/week/month/year/telemetry
##   $card_default - fallback strftime string (e.g. $Extras.Formatting.datetime)
## Looks up CardPageFormats[$card_scope] (a datetime_custom_card_* name); else $card_default.
## ---------------------------------------------------------------------------
#set $cpf = $Extras.Formatting.get('CardPageFormats', {})
#set $card_key = $cpf.get($card_scope, $cpf.get('day', ''))
#set $card_key = ', '.join($card_key) if isinstance($card_key, list) else $card_key
## resolve as a named [[Formatting]] entry; if not a known name, use the value as a literal strftime
#set global $card_datetime = $Extras.Formatting.get($card_key, $card_key) if $card_key else $card_default
#if isinstance($card_datetime, list)
    #set global $card_datetime = ', '.join($card_datetime)
#end if
