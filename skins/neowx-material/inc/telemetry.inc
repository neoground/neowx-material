#encoding UTF-8
## Shared telemetry rendering: gauges, telemetry charts, and their JavaScript.
##
## Cheetah's #include compiles a separate template, so the defs that render a
## telemetry item live here beside the code that calls them.  The host cannot
## call them - it sets these globals and includes this file:
##
##   $tele_mode            'card' | 'chart' | 'js'
##   $tele_item            item name                    (card, chart)
##   $tele_section         section slug                 (card, chart)
##   $tele_chart_items     list of chart items          (js)
##   $tele_chart_span      the page's TimespanBinder    (js, weather pages)
##   $tele_chart_interval  seconds between data points  (js, weather pages)
##
## The last two are optional: a weather page hands over its own window so a
## telemetry chart there fetches what the weather charts beside it fetch.
## The telemetry page omits them and keeps its [[Telemetry]] chart_days /
## default_interval / chart_interval window.
##
## The reverse is NOT true, and it is the trap in this file: when code inside
## this include calls $name and the HOST defines a def by that name, the
## host's def wins.  So every def here must have a name no including template
## uses.  The chart defs are teleChartCard / teleChartData / teleChartJsCode
## for exactly that reason - the weather templates define chartCard,
## getChartData and getChartJsCode with different signatures, and the
## unprefixed names were silently (chart mode) or loudly (js mode) resolving
## to those.  tests/telemetry-collision-check.py guards the whole name set.
##
## What this file resolves for itself, so no page has to: value_position.
## batteryCard reads $_tele_val_pos, and it used to be a page-level global -
## rendered anywhere else it would have silently fallen back to 'bottom'.
##
## value_position is a single [[Telemetry]]-level setting; resolve it once.
## #set GLOBAL, not a plain #set: a top-level plain #set is local to this
## template's respond() and invisible to its own defs, so batteryCard would
## fall back to 'bottom' exactly as it did on every non-telemetry page before.
## The global leaks into the host harmlessly - it is what the page did today.
#set global $_tele_val_pos = str($getVar('Extras.Telemetry.value_position', 'bottom')).strip().lower()
#if $_tele_val_pos not in ('bottom', 'left', 'right', 'none')
    #set global $_tele_val_pos = 'bottom'
#end if
##
#def formatBatteryValue($value)
    #if $value is None
        –
    #else
        #try
            ## Strip any unit/label the value carries (e.g. "0,0 V" -> "0") via
            ## parseNumericValue, so a status field stored as volts shows a clean number.
            ${"%.0f" % $parseNumericValue($value)}
        #except
            $value
        #end try
    #end if
#end def
##
## +-------------------------------------------------------------------------+
## | Template for display card of battery symbol values (top section)        |
## +-------------------------------------------------------------------------+
##
#def batteryCard($name, $section = "")
    #if $getVar('day.' + name + '.has_data') and ($getVar('day.' + name + '.max.raw') > 0 or $Extras.Telemetry.allow_zero_values == "yes")
        <div class="col-12 col-md-6 col-xl-4 mb-4">
            ## data-nwm-gauge is a contract with js/weewx-mqtt.js (updateGaugeCard).
            ## The MQTT updater would otherwise rewrite this heading with a bare
            ## number and unit, turning a mapped state like "OK" into "0.0 V"
            ## while the drawn gauge went stale. The spec carries everything a
            ## live value needs - see gaugeSpec below.
            <div class="card card-value h-100" data-name="$name" data-section="$section" data-nwm-gauge='$gaugeSpec($name)'>
                <div class="card-body text-center">
                    <h3 class="h5-responsive $Extras.color-text">
                        $getVar('obs.label.' + name)
                    </h3>

                    <div class="row">
                        <div class="col-3 text-muted font-small lo-text">
                            #set min_val = $getVar('day.' + name + '.min')
                            #set min_mapped = $mapBatteryValue($name, $min_val)
                            $min_mapped <br>
                            ($getVar('day.' + name + '.mintime').format($card_datetime))
                        </div>

                        <div class="col-6">
                            #set current_value = $getVar('current.' + name)
                            #set display_value = $mapBatteryValue($name, $current_value)

                            <h4 class="h2-responsive mb-2">
                                $display_value
                            </h4>

                            ## Value line: value_position comes from the [[Telemetry]] level (default bottom).
                            ## show_value is yes|no per field (legacy 'pad' treated as yes).
                            #set $_show_value = str($getVar('Extras.Telemetry.' + $name + '.show_value', 'yes')).strip().lower()
                            #if $_show_value != 'no'
                                #set $_show_value = 'yes'
                            #end if
                            ## value_position (a single [[Telemetry]]-level setting) is resolved once per
                            ## page into $_tele_val_pos; fall back to bottom if rendered out of context.
                            #set $_val_pos = $getVar('_tele_val_pos', 'bottom')

                            #set $_type = $sensorType($name)
                            #if $_type == 'signal'
                                #set signal_percentage = $calculateSignalPercentage($name, $current_value)
                                #set signal_color = $getSignalColor($name, $signal_percentage)
                                #set $_val_text = str($signal_percentage) + '%'
                                #set $_gauge_mode = 'signal'
                            #else if $_type == 'voltage'
                                #set battery_percentage = $calculateBatteryPercentage($name, $current_value)
                                #if $battery_percentage is None
                                    ## No usable max_voltage/min_voltage range: show raw value, no gauge
                                    #set $_gauge_mode = 'none'
                                #else
                                    #set battery_color = $getBatteryColor($name, $battery_percentage)
                                    #set fill_width = $battery_percentage
                                    #set $_val_text = str($battery_percentage) + '%'
                                    #set $_gauge_mode = 'bar'
                                #end if
                            #else if $_type == 'percent'
                                #set battery_percentage = $percentValue($current_value)
                                #set battery_color = $getPercentColor($name, $battery_percentage)
                                #set fill_width = $battery_percentage
                                #set $_val_text = str($battery_percentage) + '%'
                                #set $_gauge_mode = 'bar'
                            #else if $_type == 'status'
                                #set $_val_text = $formatBatteryValue($current_value)
                                #set state_percentage = $calculateStatePercentage($name, $current_value)
                                #if $state_percentage is not None
                                    #set battery_percentage = $state_percentage
                                    #set is_low = $isLowState($name, $current_value)
                                    #if $is_low
                                        #set battery_color = "#f44336"
                                    #else
                                        #set battery_color = "#4caf50"
                                    #end if
                                    #set fill_width = $getStateFillWidth($name, $battery_percentage, $is_low)
                                    #set $_gauge_mode = 'bar'
                                #else
                                    ## No chart position mapping for this value: show raw, no gauge
                                    #set $_gauge_mode = 'none'
                                #end if
                            #else
                                ## none -> raw display, no gauge
                                #set $_gauge_mode = 'none'
                            #end if

                            #if $_gauge_mode == 'none'
                                ## Raw value only (it's the big value above). Reserve the gauge's height
                                ## always, plus the value line's height when value_position = bottom, so a
                                ## sensor_type = none card lines up with the gauge cards in its row for
                                ## any value_position. (Gauge bars and the signal cone are both 24px tall.)
                                <div style="visibility: hidden;" aria-hidden="true">
                                    <div style="width: max-content; margin: 0 auto;">
                                        <div class="battery-container" style="border: 2px solid #555; border-radius: 4px; width: 100px; height: 24px; margin: 0 auto;"></div>
                                    </div>
                                    #if $_val_pos == 'bottom'
                                        <div class="text-muted font-small mt-1">&nbsp;</div>
                                    #end if
                                </div>
                            #else
                                <div style="width: max-content; margin: 0 auto; text-align: center;">
                                    <div style="position: relative; display: inline-block;">
                                        #if $_gauge_mode == 'signal'
                                            $signalCone($name, $signal_percentage, $signal_color)
                                        #else
                                            <!-- Visual battery gauge -->
                                            <div class="battery-container" style="border: 2px solid #555; border-radius: 4px; width: 100px; height: 24px; margin: 0 auto; position: relative;">
                                                <div class="battery-fill" style="background-color: $battery_color;width: $fill_width%;height: calc(100% - 2px);margin: 1px 0;border-radius: 2px;transition: width 0.3s ease, background-color 0.3s ease;"></div>
                                                <div style="position: absolute;top: 4px;left: 100%;width: 6px;height: 14px;background: #555;border-radius: 2px;margin-left: 2px;"></div>
                                            </div>
                                        #end if
                                        #if $_show_value != 'no'
                                            #if $_val_pos == 'left'
                                                <span class="text-muted font-small" data-nwm-gauge-text style="position: absolute; right: 100%; margin-right: 16px; top: 50%; transform: translateY(-50%); white-space: nowrap;">$_val_text</span>
                                            #else if $_val_pos == 'right'
                                                <span class="text-muted font-small" data-nwm-gauge-text style="position: absolute; left: 100%; margin-left: 16px; top: 50%; transform: translateY(-50%); white-space: nowrap;">$_val_text</span>
                                            #end if
                                        #end if
                                    </div>
                                    #if $_val_pos == 'bottom'
                                        #if $_show_value != 'no'
                                            <div class="text-muted font-small mt-1" data-nwm-gauge-text>$_val_text</div>
                                        #else
                                            <div class="text-muted font-small mt-1">&nbsp;</div>
                                        #end if
                                    #end if
                                </div>
                            #end if
                        </div>

                        <div class="col-3 text-muted font-small hi-text">
                            #set max_val = $getVar('day.' + name + '.max')
                            #set max_mapped = $mapBatteryValue($name, $max_val)
                            $max_mapped <br>
                            ($getVar('day.' + name + '.maxtime').format($card_datetime))
                        </div>
                    </div>
                </div>
            </div>
        </div>
    #end if
#end def
##
## +-------------------------------------------------------------------------+
## | Template for display card of chart (bottom section)                     |
## +-------------------------------------------------------------------------+
##
#def teleChartCard($name, $section = "")
    #if $getVar('day.' + name + '.has_data') and ($getVar('day.' + name + '.max.raw') > 0 or $Extras.Telemetry.allow_zero_values == "yes")
        <div class="col-12 col-xl-6 mb-4">
            <div class="card card-chart h-100" data-name="$name" data-section="$section">
                <div class="card-body text-center">
                    <h5 class="h5-responsive $Extras.color-text">
                        $getVar('obs.label.' + name)
                    </h5>
                    <div data-chart-mount="$name"></div>
                </div>
            </div>
        </div>
    #end if
#end def
##
## +-------------------------------------------------------------------------+
## | The explicit gauge type for a field: none (default) / voltage / signal  |
## | / percent / status. Unset or unrecognized -> none.                      |
## +-------------------------------------------------------------------------+
#def sensorType($name)
    #set $_t = str($getVar('Extras.Telemetry.' + $name + '.sensor_type', 'none')).strip().lower()
    #if $_t in ('none', 'voltage', 'signal', 'percent', 'status')
        #return $_t
    #else
        #return 'none'
    #end if
#end def
##
## Clamp a (possibly unit-bearing) value to an integer percentage 0-100.
#def percentValue($value)
    #try
        #set $_p = $parseNumericValue($value)
        #if $_p < 0
            #set $_p = 0
        #else if $_p > 100
            #set $_p = 100
        #end if
        #return int($_p + 0.0000001)
    #except
        #return 0
    #end try
#end def
##
## Percent-battery color: same low_threshold logic as the shared gauge color.
#def getPercentColor($name, $percentage)
    #return $getGaugeColor($name, $percentage)
#end def
##
## +-------------------------------------------------------------------------+
## | True when the field has an explicit gauge (sensor_type != none).        |
## +-------------------------------------------------------------------------+
#def isBatteryFieldEnabled($name)
    #return $sensorType($name) != 'none'
#end def
##
## +-------------------------------------------------------------------------+
## | True when the field's sensor_type is 'voltage'.                         |
## +-------------------------------------------------------------------------+
#def isVoltageBased($name)
    #return $sensorType($name) == 'voltage'
#end def
##
## +-------------------------------------------------------------------------+
## | True when the field's sensor_type is 'signal'.                          |
## +-------------------------------------------------------------------------+
#def isSignalBased($name)
    #return $sensorType($name) == 'signal'
#end def
##
## +-------------------------------------------------------------------------+
## | Helper function to map battery values                                   |
## +-------------------------------------------------------------------------+
#def mapBatteryValue($name, $value)
    #set $_t = $sensorType($name)
    #if $_t == 'percent'
        #return str($percentValue($value)) + '%'
    #else if $_t == 'status'
        #try
            #set raw_val = str($value).strip()
            #set key = "0"
            #if raw_val.replace(".", "").replace("-", "").isdigit()
                #set key = str(int(float(raw_val) + 0.000001))
            #end if
            #set mapped = $getVar('Extras.Telemetry.' + $name + '.' + $key, None)
            #if $mapped is not None
                #return $mapped
            #else
                #return $value
            #end if
        #except
            #return $value
        #end try
    #else
        ## none / voltage / signal -> raw value
        #return $value
    #end if
#end def
##
## +-------------------------------------------------------------------------+
## | Parse a numeric value out of a string like "4,5 V" -> 4.5               |
## +-------------------------------------------------------------------------+
#def parseNumericValue($value)
    #import re
    #set value_str = str($value).strip()
    ## Keep only digits, comma, decimal point, minus; normalize comma to period
    #set numeric_str = re.sub(r'[^0-9,.\-]', '', value_str)
    #set numeric_str = numeric_str.replace(',', '.')
    #return float($numeric_str)
#end def
##
## +-------------------------------------------------------------------------+
## | Generic range -> percentage (0-100) for a configured max/min key pair.  |
## | Returns None when the max key is absent/blank or the range is invalid.  |
## +-------------------------------------------------------------------------+
#def rangePercentage($name, $value, $max_key, $min_key)
    #try
        #set max_str = $getVar('Extras.Telemetry.' + $name + '.' + $max_key, None)
        #if $max_str is None or str($max_str).strip() == '' or str($max_str) == 'None'
            #return None
        #end if
        #set raw_val = $parseNumericValue($value)
        #set max_v = float(str($max_str).strip())
        #set min_str = $getVar('Extras.Telemetry.' + $name + '.' + $min_key, '0')
        #set min_v = float(str($min_str).strip())
        #if $max_v > $min_v
            #set percentage = (($raw_val - $min_v) / ($max_v - $min_v)) * 100
            #if $percentage < 0
                #set percentage = 0
            #elif $percentage > 100
                #set percentage = 100
            #end if
            ## Tiny epsilon so float results like 56.99999 still read as 57
            #return int($percentage + 0.0000001)
        #else
            #return None
        #end if
    #except
        #return None
    #end try
#end def
##
## +-------------------------------------------------------------------------+
## | Battery percentage from voltage vs the max_voltage/min_voltage range.    |
## | Returns None when no usable range is configured, so the caller can show  |
## | the raw value with no gauge instead of a misleading full bar.           |
## +-------------------------------------------------------------------------+
#def calculateBatteryPercentage($name, $value)
    #return $rangePercentage($name, $value, 'max_voltage', 'min_voltage')
#end def
##
## +-------------------------------------------------------------------------+
## | Helper function to calculate signal percentage                          |
## | Returns percentage (0-100) based on current value vs max_signal.        |
## | If max_signal is not configured, the raw value is used directly as a    |
## | percentage (clamped to 0-100 by percentValue).                          |
## +-------------------------------------------------------------------------+
#def calculateSignalPercentage($name, $value)
    #set p = $rangePercentage($name, $value, 'max_signal', 'min_signal')
    #if $p is None
        #return $percentValue($value)
    #else
        #return $p
    #end if
#end def
##
## +-------------------------------------------------------------------------+
## | Battery percentage from the state's chart position:                     |
## | (position + 1) / (max_chart_position + 1) * 100, flip-aware.            |
## | Returns None when the field/value has no chart position mapping.        |
## +-------------------------------------------------------------------------+
#def calculateStatePercentage($name, $value)
    #try
        #if not $isBatteryFieldEnabled($name)
            #return None
        #end if
        ## Assumes non-negative state values, like getBatteryChartValue below
        #set key = str(int($parseNumericValue($value) + 0.000001))
        #set chart_pos = $getVar('Extras.Telemetry.' + $name + '.chart_position_' + $key, None)
        #if $chart_pos is None
            #return None
        #end if
        #set pos = int($chart_pos)
        #set max_pos = int($getVar('Extras.Telemetry.' + $name + '.max_chart_position', '1'))
        #if $getVar('Extras.Telemetry.' + $name + '.flip_values', 'no') == 'yes'
            #set pos = $max_pos - $pos
        #end if
        ## Same epsilon trick as rangePercentage
        #return int((($pos + 1.0) / ($max_pos + 1.0)) * 100 + 0.0000001)
    #except
        #return None
    #end try
#end def
##
## +-------------------------------------------------------------------------+
## | Is the raw value on the LOW side of the low_state threshold?            |
## | low_when picks the direction: at_or_above (default) or at_or_below.     |
## | Always False when low_state is not configured.                          |
## +-------------------------------------------------------------------------+
#def isLowState($name, $value)
    #try
        #set low_str = $getVar('Extras.Telemetry.' + $name + '.low_state', None)
        #if $low_str is None or str($low_str).strip() == '' or str($low_str) == 'None'
            #return False
        #end if
        #set low_v = float(str($low_str).strip())
        #set raw_v = $parseNumericValue($value)
        #set direction = str($getVar('Extras.Telemetry.' + $name + '.low_when', 'at_or_above')).strip().lower()
        #if $direction == 'at_or_below'
            #return $raw_v <= $low_v
        #else
            #return $raw_v >= $low_v
        #end if
    #except
        #return False
    #end try
#end def
##
## +-------------------------------------------------------------------------+
## | Fill width for the status battery bar. A low 2-state battery fills      |
## | the whole bar red; everything else fills proportionally.                |
## +-------------------------------------------------------------------------+
#def getStateFillWidth($name, $percentage, $is_low)
    #if not $is_low
        #return $percentage
    #end if
    #set max_pos = 1
    #try
        #set max_pos = int($getVar('Extras.Telemetry.' + $name + '.max_chart_position', '1'))
    #except
        ## Unreadable max_chart_position - treat as 2-state
        #pass
    #end try
    #if $max_pos == 1
        #return 100
    #else
        #return $percentage
    #end if
#end def
##
## +-------------------------------------------------------------------------+
## | Gauge color: red below low_threshold (default 20), otherwise green.      |
## | The percentage is already computed from the field's range (or the raw   |
## | value when no range is set), so no max_* key is needed here.            |
## +-------------------------------------------------------------------------+
#def getGaugeColor($name, $percentage)
    #try
        #set threshold = float($getVar('Extras.Telemetry.' + $name + '.low_threshold', '20'))
        #if float($percentage) < $threshold
            #return "#f44336"  ## Red
        #else
            #return "#4caf50"  ## Green
        #end if
    #except
        #return "#4caf50"  ## Default green
    #end try
#end def
##
## +-------------------------------------------------------------------------+
## | Helper function to determine battery gauge color (voltage threshold)    |
## +-------------------------------------------------------------------------+
#def getBatteryColor($name, $percentage)
    #return $getGaugeColor($name, $percentage)
#end def
##
## +-------------------------------------------------------------------------+
## | Helper function to determine signal cone color (signal threshold)       |
## +-------------------------------------------------------------------------+
#def getSignalColor($name, $percentage)
    #return $getGaugeColor($name, $percentage)
#end def
##
## +-------------------------------------------------------------------------+
## | The range a voltage/signal gauge is scaled over, as [min, max] floats,  |
## | or [min, None] when max_<kind> is absent or the range is invalid - the  |
## | same rule rangePercentage applies, so the live spec below and the drawn |
## | card agree on when there is a gauge at all.                             |
## +-------------------------------------------------------------------------+
#def rangeBounds($name, $max_key, $min_key)
    #try
        #set $min_v = float(str($getVar('Extras.Telemetry.' + $name + '.' + $min_key, '0')).strip())
    #except
        #set $min_v = 0.0
    #end try
    #try
        #set $max_str = $getVar('Extras.Telemetry.' + $name + '.' + $max_key, None)
        #if $max_str is None or str($max_str).strip() == '' or str($max_str) == 'None'
            #return [$min_v, None]
        #end if
        #set $max_v = float(str($max_str).strip())
        #if $max_v > $min_v
            #return [$min_v, $max_v]
        #end if
        #return [$min_v, None]
    #except
        #return [$min_v, None]
    #end try
#end def
##
## +-------------------------------------------------------------------------+
## | The gauge spec the MQTT updater applies in the browser (weewx-mqtt.js   |
## | updateGaugeCard). Everything a live value needs is decided HERE, by the |
## | same helpers that drew the card: for a status field a complete table of |
## | every configured state - label, bar percentage, fill width and colour,  |
## | with flip_values and low_state already applied - so no [[Telemetry]]    |
## | parsing exists in JavaScript; for range fields just min/max and the     |
## | low_threshold. Emitted as JSON inside a single-quoted attribute, hence  |
## | the escaping at the end.                                                |
## +-------------------------------------------------------------------------+
#def gaugeSpec($name)
    #import json
    #set $_type = $sensorType($name)
    #set $_show = str($getVar('Extras.Telemetry.' + $name + '.show_value', 'yes')).strip().lower() != 'no'
    #set $spec = {'type': $_type, 'show_value': $_show, 'val_pos': str($getVar('_tele_val_pos', 'bottom'))}
    #if $_type == 'status'
        ## Every state the config knows: a label key ("0 = OK") or a
        ## chart_position_<n> key, either is enough to be worth a table row.
        #set $field = $getVar('Extras.Telemetry.' + $name, {})
        #set $keys = set()
        #for $k in list($field.keys())
            #set $ks = str($k).strip()
            #if $ks.isdigit()
                #silent $keys.add($ks)
            #else if $ks.startswith('chart_position_') and $ks.split('chart_position_', 1)[1].isdigit()
                #silent $keys.add($ks.split('chart_position_', 1)[1])
            #end if
        #end for
        #set $states = {}
        #for $k in sorted($keys, key=int)
            #set $pct = $calculateStatePercentage($name, $k)
            #set $entry = {'label': str($mapBatteryValue($name, $k)).strip(), 'pct': $pct, 'fill': None, 'color': None}
            #if $pct is not None
                #set $low = $isLowState($name, $k)
                #silent $entry.update({'fill': $getStateFillWidth($name, $pct, $low), 'color': '#f44336' if $low else '#4caf50'})
            #end if
            #silent $states.update({$k: $entry})
        #end for
        #silent $spec.update({'states': $states})
    #else if $_type in ('voltage', 'signal', 'percent')
        #try
            #set $thr = float($getVar('Extras.Telemetry.' + $name + '.low_threshold', '20'))
        #except
            #set $thr = 20.0
        #end try
        #silent $spec.update({'low_threshold': $thr})
        #if $_type == 'voltage'
            #set $bounds = $rangeBounds($name, 'max_voltage', 'min_voltage')
            #silent $spec.update({'min': $bounds[0], 'max': $bounds[1]})
        #else if $_type == 'signal'
            #set $bounds = $rangeBounds($name, 'max_signal', 'min_signal')
            #silent $spec.update({'min': $bounds[0], 'max': $bounds[1]})
        #end if
    #end if
    #return json.dumps($spec).replace('&', '&amp;').replace("'", '&#39;').replace('<', '&lt;').replace('>', '&gt;')
#end def
##
## +-------------------------------------------------------------------------+
## | Render the signal "cone" as a static SVG: two parenthesis-style wings   |
## | around a center dot, filled center-outward by percentage (0-100).       |
## | $name is unused for now.                                                |
## +-------------------------------------------------------------------------+
#def signalCone($name, $percentage, $color)
    #import math
    ## Center y is always 12 in the paths below
    #set R = 14.6
    #set hh_i = 4.75
    #set hh_o = 11.0
    #set f = float($percentage) / 100.0
    #if $f < 0.0
        #set f = 0.0
    #end if
    #if $f > 1.0
        #set f = 1.0
    #end if
    ## Sagitta (bulge depth) for inner, outer, and frontier edges.
    #set si = $R - math.sqrt($R * $R - $hh_i * $hh_i)
    #set so = $R - math.sqrt($R * $R - $hh_o * $hh_o)
    #set hhf = $hh_i + $f * ($hh_o - $hh_i)
    #set sf = $R - math.sqrt($R * $R - $hhf * $hhf)
    ## Frontier x for each wing (inner -> outer).
    #set xfL = 52.0 + $f * (12.0 - 52.0)
    #set xfR = 68.0 + $f * (108.0 - 68.0)
    ## Constant y edges: top_i=7.25 bot_i=16.75 top_o=1 bot_o=23, center=12.
    ## data-nwm-gauge-cone / data-nwm-cone are hooks for weewx-mqtt.js
    ## drawSignalCone(), which recomputes the two filled wings for a live value
    ## with the same geometry as below.
    <svg data-nwm-gauge-cone viewBox="0 0 120 24" width="120" height="24" style="display:block;margin:0 auto;" xmlns="http://www.w3.org/2000/svg">
        <path d="M 52 7.25 L 12 1 Q ${"%.3f" % (12.0 - 2 * $so)} 12 12 23 L 52 16.75 Q ${"%.3f" % (52.0 - 2 * $si)} 12 52 7.25 Z" fill="#4a4a4a"/>
        <path d="M 68 7.25 L 108 1 Q ${"%.3f" % (108.0 + 2 * $so)} 12 108 23 L 68 16.75 Q ${"%.3f" % (68.0 + 2 * $si)} 12 68 7.25 Z" fill="#4a4a4a"/>
        <path data-nwm-cone="l" d="M 52 7.25 L ${"%.3f" % $xfL} ${"%.3f" % (12.0 - $hhf)} Q ${"%.3f" % ($xfL - 2 * $sf)} 12 ${"%.3f" % $xfL} ${"%.3f" % (12.0 + $hhf)} L 52 16.75 Q ${"%.3f" % (52.0 - 2 * $si)} 12 52 7.25 Z" fill="$color"/>
        <path data-nwm-cone="r" d="M 68 7.25 L ${"%.3f" % $xfR} ${"%.3f" % (12.0 - $hhf)} Q ${"%.3f" % ($xfR + 2 * $sf)} 12 ${"%.3f" % $xfR} ${"%.3f" % (12.0 + $hhf)} L 68 16.75 Q ${"%.3f" % (68.0 + 2 * $si)} 12 68 7.25 Z" fill="$color"/>
        <circle cx="60" cy="12" r="3.5" fill="#ddd"/>
    </svg>
#end def
##
## +-------------------------------------------------------------------------+
## | Helper function to get chart value for battery fields                   |
## | Returns the numeric chart position based on mapping order               |
## | For voltage- or signal-based sensors, returns the raw value             |
## +-------------------------------------------------------------------------+
#def getBatteryChartValue($name, $raw_value)
    #if not $isBatteryFieldEnabled($name)
        #return $raw_value
    #end if

    ## Don't convert voltage-based sensors - use raw voltage in charts
    #if $isVoltageBased($name)
        #return $raw_value
    #end if

    ## Don't convert signal-based sensors - use raw value in charts
    #if $isSignalBased($name)
        #return $raw_value
    #end if

    ## Percent sensors: plot the raw 0-100 value directly
    #if $sensorType($name) == 'percent'
        #return $raw_value
    #end if

    ## Only convert status-based sensors to chart positions
    #try
        #set key = str(int(float($raw_value) + 0.000001))

        ## Check if flip is enabled for this specific field
        #set flip = False
        #try
            #if $getVar('Extras.Telemetry.' + $name + '.flip_values', 'no') == 'yes'
                #set flip = True
            #end if
        #except
            #pass
        #end try

        ## Get chart position based on key
        #set chart_pos = $getVar('Extras.Telemetry.' + $name + '.chart_position_' + $key, None)
        #if $chart_pos is not None
            #if $flip
                ## Invert the position if flip is enabled
                #set max_pos = int($getVar('Extras.Telemetry.' + $name + '.max_chart_position', '1'))
                #set chart_pos = str($max_pos - int($chart_pos))
            #end if
            #return $chart_pos
        #else
            ## Fallback: use the key itself as position
            #return $key
        #end if
    #except
        #return $raw_value
    #end try
#end def
##
        ## +-------------------------------------------------------------------------+
        ## | Chart generation methods (get data for charts + their JS code)          |
        ## +-------------------------------------------------------------------------+
        ##
        ## +-------------------------------------------------------------------------+
        ## | Get data array for a chart                                              |
        ## |                                                                         |
        ## | string  $name    the name of the database field  (e.g. outTemp)         |
        ## | string  $column  the column of the display value (e.g. min, max, avg)   |
        ## +-------------------------------------------------------------------------+
        #def teleChartData($name, $column, $chart_interval=300)
            ## Get chart timespan (default to current_timespan if not specified)
            #set chart_days = 1
            #try
                #set chart_days = int($getVar('Extras.Telemetry.chart_days', '1'))
            #except
                #set chart_days = 1
            #end try

            ## Check if this is an enabled battery field
            #set is_battery = $isBatteryFieldEnabled($name)

            ## A host page may hand over its own window as tele_chart_span (a
            ## TimespanBinder) so a telemetry chart on Week fetches the week
            ## at the week's interval - the same data the weather charts
            ## beside it fetch - instead of chart_days at default_interval,
            ## which is 8,640 queries per chart and was doubling report time.
            ## The telemetry page hands over nothing and keeps its own window.
            #set _page_span = $getVar('tele_chart_span', None)
            #if $_page_span is not None
                #set records = list($_page_span.spans(interval=$chart_interval))
            #else
                #set records = list($span($day_delta=$chart_days, $boundary='midnight').spans(interval=$chart_interval))
            #end if
            #for idx, record in enumerate($records)
                #try
                    #set val = $getattr($record, $name)
                    #set data = $getattr($val, $column).format(add_label=False, localize=False, None_string="null")

                    ## For battery fields, get the chart position value
                    #if $is_battery and $data != "null"
                        #set data = $getBatteryChartValue($name, $data)
                    #end if

                    #if $data != "null" or $idx == 0 or $idx == len($records) - 1
                        [$record.start.raw * 1000, $data],
                    #end if
                #except
                    ## Skip records with no data for this field (sparse sensors).
                    #pass
                #end try
            #end for
        #end def
##
        ## +-------------------------------------------------------------------------+
        ## | Get JavaScript code for a chart                                         |
        ## |                                                                         |
        ## | string  $name     the name of the database field   (e.g. outTemp)       |
        ## | string  $series   field name for series            (e.g. outTemp)       |
        ## | string  $column   the column of the display value  (e.g. min, max, avg) |
        ## +-------------------------------------------------------------------------+
        #def teleChartJsCode($name, $series, $column = "avg")

            #if $getVar('day.' + name + '.has_data') and ($getVar('day.' + name + '.max.raw') > 0 or $Extras.Telemetry.allow_zero_values == "yes")

                ## (Y-axis label style is decided per sensor_type below.)

                ## Read per-field chart_interval: per-field config under [[Telemetry]] takes priority, then default_interval from config, then hardcoded 300
                #set chart_interval = 300
                #try
                    #set _default_interval_str = $getVar('Extras.Telemetry.default_interval', None)
                    #if $_default_interval_str is not None and str($_default_interval_str).strip() != '' and str($_default_interval_str) != 'None'
                        #set chart_interval = int($_default_interval_str)
                    #end if
                #except
                    #set chart_interval = 300
                #end try
                #try
                    #set _field_interval = $getVar('Extras.Telemetry.' + $name + '.chart_interval', None)
                    #if $_field_interval is not None and str($_field_interval).strip() != '' and str($_field_interval) != 'None'
                        #set chart_interval = int($_field_interval)
                    #end if
                #except
                    #pass
                #end try
                ## On a weather page the host hands over its own interval as
                ## tele_chart_interval, and it wins over the [[Telemetry]]
                ## settings: the chart should match the weather charts beside
                ## it, and chart_days / default_interval / chart_interval
                ## describe the telemetry page's window, not this page's.
                #set _page_interval = $getVar('tele_chart_interval', None)
                #if $_page_interval is not None
                    #set chart_interval = int($_page_interval)
                #end if

                ## Per-field chart line color: Extras.Telemetry.<field>.colors
                #set $tc_color_ref = ''
                #try
                    #set $_fc_colors = $getVar('Extras.Telemetry.' + $name + '.colors', None)
                    #if $_fc_colors is not None
                        #if isinstance($_fc_colors, list)
                            #set $tc_color_ref = str($_fc_colors[0]).strip().replace('"', '').replace('\\', '') if len($_fc_colors) > 0 else ''
                        #else
                            #set $tc_color_ref = str($_fc_colors).strip().replace('"', '').replace('\\', '')
                        #end if
                    #end if
                #except
                    #set $tc_color_ref = ''
                #end try

                // currentvalue: --> $chart_interval <--

                document.querySelectorAll('[data-chart-mount="$name"]').forEach(function (chartElement) {
                new ApexCharts(chartElement, {
                    ...graph_area_config,
                    #if $pg_align_mid
                    xaxis: {
                        ...graph_area_config.xaxis,
                        #include "inc/graph_align_axis.inc"
                    },
                    #else
                    xaxis: {
                        ...graph_area_config.xaxis,
                        #include "inc/graph_tick_amount.inc"
                    },
                    #end if
                    yaxis: {
                        #if $sensorType($name) == 'status'
                            ## Status sensor: use custom position labels
                            #try
                                #set max_pos = int($getVar('Extras.Telemetry.' + $name + '.max_chart_position', '1'))
                            #except
                                #set max_pos = 1
                            #end try
                            min: 0,
                            max: $max_pos,
                            tickAmount: $max_pos,
                            labels: {
                                formatter: function (val) {
                                    ## Generate label mapping from config for this specific field
                                    #set flip = False
                                    #try
                                        #if $getVar('Extras.Telemetry.' + $name + '.flip_values', 'no') == 'yes'
                                            #set flip = True
                                        #end if
                                    #except
                                        #pass
                                    #end try

                                    #for $i in range($max_pos + 1)
                                        #set pos = $i
                                        #if $flip
                                            #set pos = $max_pos - $i
                                        #end if
                                        #set label_key = None
                                        ## Find which key maps to this position
                                        #for $j in range(10)
                                            #try
                                                #set check_pos = $getVar('Extras.Telemetry.' + $name + '.chart_position_' + str($j), None)
                                                #if $check_pos is not None and int($check_pos) == $pos
                                                    #set label_key = str($j)
                                                    #break
                                                #end if
                                            #except
                                                #pass
                                            #end try
                                        #end for
                                        #if $label_key is not None
                                            #try
                                                #set label_text = $getVar('Extras.Telemetry.' + $name + '.' + $label_key, $label_key)
                                            #except
                                                #set label_text = $label_key
                                            #end try
                                            if (val === $i) return "$label_text";
                                        #end if
                                    #end for
                                    return val;
                                }
                            }
                        #else
                            ## Voltage-based, signal-based, or regular sensor: use normal numeric labels
                            labels: {
                                formatter: function (val) {
                                    return formatNumber(val, "$getVar('unit.format.' + name)") + "$getVar('unit.label.' + name)";
                                }
                            }
                        #end if
                    },
                    #if $tc_color_ref
                    colors: neowxChartColors(["$tc_color_ref"], 1, "$name"),
                    #end if
                    series: [
                        {
                            name: "$getVar('obs.label.' + series)",
                            data: [ $teleChartData(series, column, chart_interval) ]
                        }
                    ]
                }).render()
                });

            #end if

        #end def
##
#if $tele_mode == 'js'
    #for $x in $tele_chart_items
        $teleChartJsCode($x, $x)
    #end for
#else if $tele_mode == 'chart'
    $teleChartCard($tele_item, $tele_section)
#else
    $batteryCard($tele_item, $tele_section)
#end if
