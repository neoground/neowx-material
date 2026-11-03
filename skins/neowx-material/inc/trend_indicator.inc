#encoding UTF-8
## ---------------------------------------------------------------------------
## Trend indicator for observation $name (the "current" page).
## A Cheetah #include has its own scope and cannot see the caller's local
## $name, so the caller passes it via the $_trend_obs global. The caller only
## guarantees $name is in show_trend_on.
##
## We compute the change OURSELVES rather than use WeeWX $trend (which returns
## None unless an archive record lands within a small grace window of exactly
## now - time_delta): (current reading) - (oldest reading in the last N hours,
## via span(hour_delta=N).<obs>.first). That has a value whenever there is any
## recent data.
##
## The direction is decided in a CANONICAL unit per unit-family, derived from
## the obs's current display unit, so trends behave identically in metric and
## imperial for ANY observation. Units not listed (percent, UV index, ppm,
## ug/m3, W/m2, counts) are already comparable across systems -> no conversion.
##
## Barometric pressure can use richer tendency categories selected by
## $Extras.Appearance.trend_type:
##   multi  - WMO/NWS-style steady/slowly/rising/rapidly (default); the arrow
##            steepens as it climbs and is doubled for the rapid tier.
##   davis  - Davis Vantage steady/slowly/rapidly thresholds.
##   strict - simple up/steady/down, same as every other obs (no rapid tier).
## Every obs (including pressure in strict) still decides direction in the
## canonical unit, and the tooltip shows the change in the user's display units.
## ---------------------------------------------------------------------------
#set $name = $_trend_obs
#set $_hours = 3
#set $_mode = str($getVar('Extras.Appearance.trend_type', 'multi')).lower()
#if $_mode not in ('strict', 'multi', 'davis')
    #set $_mode = 'multi'
#end if
#set $_pressure = $name in ('barometer', 'pressure', 'altimeter')
## display unit -> canonical unit (only families whose unit varies by system)
#set $_canon = {
    'inHg':'mbar', 'mmHg':'mbar', 'kPa':'mbar', 'hPa':'mbar', 'mbar':'mbar',
    'degree_F':'degree_C', 'degree_K':'degree_C', 'degree_C':'degree_C',
    'inch':'mm', 'cm':'mm', 'mm':'mm',
    'inch_per_hour':'mm_per_hour', 'cm_per_hour':'mm_per_hour', 'mm_per_hour':'mm_per_hour',
    'mile_per_hour':'km_per_hour', 'knot':'km_per_hour', 'meter_per_second':'km_per_hour', 'beaufort':'km_per_hour', 'km_per_hour':'km_per_hour',
    'foot':'meter', 'meter':'meter', 'mile':'km', 'km':'km'
}
## "steady" dead-band per canonical unit (default 0.5 for unit-agnostic obs)
#set $_deadbands = {'degree_C':0.5, 'mm':0.2, 'mm_per_hour':0.2, 'km_per_hour':1.0, 'meter':10.0, 'km':0.5}
#set $_du   = $getVar('unit.unit_type.' + $name, '')
#set $_unit = $_canon.get($_du, '')
#if $_pressure
    #set $_unit = 'mbar'
#end if
#set $_band = $_deadbands.get($_unit, 0.5)
## current reading and the oldest reading in the last $_hours hours
#set $_curvh  = $getattr($current, $name)
#set $_binder = $getattr($span($hour_delta=$_hours), $name)
#set $_oldvh  = $_binder.first
## canonical-unit delta (decision) and display-unit delta (tooltip)
#set $_tv = None
#set $_disp = None
#set $_elapsed = None
#try
    #if $_unit
        #set $_tv = $getattr($_curvh, $_unit).raw - $getattr($_oldvh, $_unit).raw
    #else
        #set $_tv = $_curvh.raw - $_oldvh.raw
    #end if
    #set $_disp = $_curvh.raw - $_oldvh.raw
    ## seconds (then hours) actually covered by the data (first reading -> now),
    ## which after a restart / new install / data gap can be far less than the
    ## requested span. Kept in seconds so the snap test below is exact.
    #set $_secs    = float($current.dateTime.raw) - float($_binder.firsttime.raw)
    #set $_elapsed = $_secs / 3600.0
#except
    #set $_tv = None
#end try
## Require enough recent history for a meaningful tendency: with only a few
## minutes of data the 3-hour pressure categories and "over N h" label would be
## misleading, so suppress the indicator until the window is wide enough.
#set $_min_hours = 1.0
#if $_tv is not None and $_elapsed is not None and $_elapsed >= $_min_hours
    #set $_lbl    = $getVar('unit.label.' + $name, '')
    #set $_disp_r = round($_disp, 2)
    ## Effective window: the actual elapsed span, but snapped to the full target
    ## when within 5 minutes of it (archive timing rarely lands exactly on now-3h),
    ## so the normal case reads a clean "3 h" with nominal thresholds.
    #set $_win_h = $_elapsed
    #if abs($_secs - float($_hours) * 3600.0) <= 300.0
        #set $_win_h = float($_hours)
    #end if
    #set $_win = '%g h' % round($_win_h, 1)
    #if $_pressure
        ## Decimals per pressure display unit (small inHg/kPa deltas need more).
        #set $_dec    = {'inHg': 3, 'kPa': 2, 'mmHg': 2, 'hPa': 1, 'mbar': 1}.get($_du, 2)
        #set $_disp_s = ('%+.' + str($_dec) + 'f') % $_disp
    #end if
    #if $_pressure and $_mode != 'strict'
        ## Pressure tendency categories, in canonical mbar. The thresholds are
        ## defined per 3 h, so scale them to the effective (snapped) span.
        #set $_scale = $_win_h / 3.0
        #if $_scale <= 0
            #set $_scale = 1.0
        #end if
        #set $_mag = abs($_tv)
        ## $_ud feeds the (lowercase) Weather-Icons class.
        #set $_ud  = 'up' if $_tv > 0 else 'down'
        #if $_mode == 'davis'
            ## Davis Vantage (3 h, hPa): steady <0.7, slowly 0.7-2.0, rapidly >=2.0
            #if $_mag < 0.7 * $_scale
                <i class="wi wi-direction-right" title="$pgettext("Trend", "steady"): $_disp_s$_lbl ($pgettext("Trend", "over") $_win)" data-toggle="tooltip" data-html="true"></i>
            #else
                #if $_mag >= 2.0 * $_scale
                    #set $_wic  = 'wi-direction-' + $_ud
                    #set $_word = $pgettext("Trend", "rising_rapidly") if $_tv > 0 else $pgettext("Trend", "falling_rapidly")
                #else
                    #set $_wic  = 'wi-direction-' + $_ud + '-right'
                    #set $_word = $pgettext("Trend", "rising_slowly") if $_tv > 0 else $pgettext("Trend", "falling_slowly")
                #end if
                <i class="wi $_wic" title="$_word: $_disp_s$_lbl ($pgettext("Trend", "over") $_win)" data-toggle="tooltip" data-html="true"></i>
            #end if
        #else
            ## multi / WMO-style (3 h, hPa): steady <0.7, slowly 0.7-2.0,
            ## rising 2.0-5.0, rapidly >=5.0 (doubled arrow).
            #if $_mag < 0.7 * $_scale
                <i class="wi wi-direction-right" title="$pgettext("Trend", "steady"): $_disp_s$_lbl ($pgettext("Trend", "over") $_win)" data-toggle="tooltip" data-html="true"></i>
            #else
                #set $_rising = $_tv > 0
                #set $_double = False
                #if $_mag >= 5.0 * $_scale
                    ## rapidly: two straight arrows
                    #set $_wic  = 'wi-direction-' + $_ud
                    #set $_double = True
                    #set $_word = $pgettext("Trend", "rising_rapidly") if $_rising else $pgettext("Trend", "falling_rapidly")
                #else if $_mag >= 2.0 * $_scale
                    ## rising: one straight arrow
                    #set $_wic  = 'wi-direction-' + $_ud
                    #set $_word = $pgettext("Trend", "rising") if $_rising else $pgettext("Trend", "falling")
                #else
                    ## slowly: one diagonal arrow
                    #set $_wic  = 'wi-direction-' + $_ud + '-right'
                    #set $_word = $pgettext("Trend", "rising_slowly") if $_rising else $pgettext("Trend", "falling_slowly")
                #end if
                #if $_double
                    <span title="$_word: $_disp_s$_lbl ($pgettext("Trend", "over") $_win)" data-toggle="tooltip" data-html="true"><i class="wi $_wic"></i><i class="wi $_wic"></i></span>
                #else
                    <i class="wi $_wic" title="$_word: $_disp_s$_lbl ($pgettext("Trend", "over") $_win)" data-toggle="tooltip" data-html="true"></i>
                #end if
            #end if
        #end if
    #else
        ## Simple up / steady / down: strict mode for every obs, plus the
        ## non-pressure path for multi/davis. Canonical dead-band per family.
        #set $_simp = $_disp_s if $_pressure else $_disp_r
        #if $_tv > $_band
            <i class="wi wi-direction-up-right" title="$pgettext("Trend", "rising"): $_simp$_lbl ($pgettext("Trend", "over") $_win)" data-toggle="tooltip" data-html="true"></i>
        #else if $_tv < -$_band
            <i class="wi wi-direction-down-right" title="$pgettext("Trend", "falling"): $_simp$_lbl ($pgettext("Trend", "over") $_win)" data-toggle="tooltip" data-html="true"></i>
        #else
            <i class="wi wi-direction-right" title="$pgettext("Trend", "steady"): $_simp$_lbl ($pgettext("Trend", "over") $_win)" data-toggle="tooltip" data-html="true"></i>
        #end if
    #end if
#end if
