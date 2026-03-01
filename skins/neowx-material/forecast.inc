#encoding UTF-8
## +-------------------------------------------------------------+
## |    forecast.inc                       Forecast templates    |
## +-------------------------------------------------------------+

#set $weatherMap = {
0: "wi-day-sunny",
1: "wi-day-sunny-overcast",
2: "wi-day-cloudy",
3: "wi-cloudy",
45: "wi-day-fog",
48: "wi-day-fog",
51: "wi-day-sprinkle",
53: "wi-day-sprinkle",
55: "wi-day-sprinkle",
56: "wi-day-sprinkle",
57: "wi-day-sprinkle",
61: "wi-showers",
63: "wi-showers",
65: "wi-rain",
66: "wi-day-showers",
67: "wi-day-rain",
71: "wi-day-snow",
73: "wi-day-snow",
75: "wi-day-snow",
77: "wi-day-snow",
80: "wi-day-showers",
81: "wi-day-showers",
82: "wi-day-showers",
85: "wi-day-snow",
86: "wi-day-snow",
95: "wi-day-storm-showers",
96: "wi-day-thunderstorm",
99: "wi-day-thunderstorm"
}

#set $vars = $Extras.Forecast.variables
#if not isinstance($vars, list)
    #if len($vars) > 0
        #set $vars = [$vars]
    #else
        #set $vars = []
    #end if
#end if

#if $forecast is not None
    #if $Extras.Forecast.forecast_separator == "yes" and $Extras.Appearance.values_order[0] != "forecast"
        <div class="col-12"><hr class="m-0 mb-4" /></div>
    #end if
    #for $i, $x in $enumerate($forecast.daily)
        <div class="col-12 col-md-6 col-xl-4 mb-4">
            <div class="card">
                <div class="card-body text-center pb-1">
                    <h3 class="h5-responsive $Extras.color-text">
                        #if $Extras.Forecast.show_today_tomorrow == "yes" and $i == 0
                            $pgettext("Forecast","forecast") - $pgettext("Forecast","today")
                        #else if $Extras.Forecast.show_today_tomorrow == "yes" and $i == 1
                            $pgettext("Forecast","forecast") - $pgettext("Forecast","tomorrow")
                        #else
                            $pgettext("Forecast","forecast") - $x[0].format("%A")
                        #end if
                    </h3>
                    <div class="row flex-fill font-small">
                        #if $Extras.Forecast.show_description == "yes"
                            <div class="col-12 text-center pb-2">$pgettext("Forecast",str($x[1].weather_code))</div>
                        #end if
                        #if $Extras.Forecast.show_icon == "yes"
                            #if $vars and len($vars) > 0
                                <div class="col-5">
                                    <i class="display-3 py-4 wi $weatherMap.get($x[1].weather_code, "wi-na")" title="$pgettext("Forecast",str($x[1].weather_code))" data-toggle="tooltip" data-html="true"></i>
                                </div>
                                <div class="col-7 text-nowrap align-content-center">
                                    <div class="row">
                                        #for $variable in $vars
                                            #if $variable == "temperature"
                                                <div class="col-2 p-0">
                                                    <i class="wi wi-thermometer $Extras.color-text" title="$pgettext("Forecast","min_temp") / $pgettext("Forecast","max_temp")" data-toggle="tooltip" data-html="true"></i>
                                                </div>
                                                <div class="col-10 text-left">
                                                    $x[1].temperature.min.format("%.0f") / $x[1].temperature.max.format("%.0f")
                                                </div>
                                            #end if
                                            #if ($variable == "sun") or ($variable == "uv-sun")
                                                <div class="col-2 p-0">
                                                    #if ($variable == "sun")
                                                        <i class="wi wi-day-sunny $Extras.color-text" title="$pgettext("Forecast","sunshine_duration") / $pgettext("Forecast","daylight_duration")" data-toggle="tooltip" data-html="true"></i>
                                                    #else
                                                        <i class="wi wi-day-sunny $Extras.color-text" title="$pgettext("Forecast","uv") / $pgettext("Forecast","sunshine_duration") / $pgettext("Forecast","daylight_duration")" data-toggle="tooltip" data-html="true"></i>
                                                    #end if
                                                </div>
                                                <div class="col-10 text-left">
                                                    #set daylight_hours = int($x[1].sun.daylight_duration.raw / 3600)
                                                    #set daylight_minutes = int(($x[1].sun.daylight_duration.raw % 3600) / 60)
                                                    #set sunshine_hours = int($x[1].sun.sunshine_duration.raw / 3600)
                                                    #set sunshine_minutes = int(($x[1].sun.sunshine_duration.raw % 3600) / 60)
                                                    #if ($variable == "sun")
                                                        ${sunshine_hours}:${'%02d' % $sunshine_minutes}h / ${daylight_hours}:${'%02d' % $daylight_minutes}h
                                                    #else
                                                        $x[1].sun.uv.format("%.0f") / ${sunshine_hours}:${'%02d' % $sunshine_minutes}h / ${daylight_hours}:${'%02d' % $daylight_minutes}h
                                                    #end if
                                                </div>
                                            #end if
                                            #if $variable == "precipitation"
                                                <div class="col-2 p-0">
                                                    <i class="wi wi-umbrella $Extras.color-text" title="$pgettext("Forecast","precipitation") ($pgettext("Forecast","probability"))" data-toggle="tooltip" data-html="true"></i>
                                                </div>
                                                <div class="col-10 text-left">
                                                    $x[1].precipitation.sum ($x[1].precipitation.probability)
                                                </div>
                                            #end if
                                            #if $variable == "evapotranspiration"
                                                <div class="col-2 p-0">
                                                    <i class="wi wi-hot $Extras.color-text" title="$pgettext("Forecast","evapotranspiration")" data-toggle="tooltip" data-html="true"></i>
                                                </div>
                                                <div class="col-10 text-left">
                                                    $x[1].et0_fao_evapotranspiration.format("%.1f")
                                                </div>
                                            #end if
                                            #if $variable == "uv"
                                                <div class="col-2 p-0">
                                                    <i class="wi wi-day-sunny $Extras.color-text" title="$pgettext("Forecast","uv")" data-toggle="tooltip" data-html="true"></i>
                                                </div>
                                                <div class="col-10 text-left">
                                                    $x[1].sun.uv.format("%.0f")
                                                </div>
                                            #end if
                                            #if $variable == "wind"
                                                <div class="col-2 p-0">
                                                    <i class="wi wi-strong-wind $Extras.color-text" title="$pgettext("Forecast","wind_speed") & $pgettext("Forecast","wind_dir") ($pgettext("Forecast","wind_gust"))" data-toggle="tooltip" data-html="true"></i>
                                                </div>
                                                <div class="col-10 text-left">
                                                    $x[1].wind.speed.format("%.0f") <i class="wi wi-wind from-$x[1].wind.direction.formatted-deg mr-2" title="$x[1].wind.direction.formatted°" data-toggle="tooltip" data-html="true"></i> ($x[1].wind.gusts.format("%.0f"))
                                                </div>
                                            #end if
                                        #end for
                                    </div>
                                </div>
                            #else
                                <div class="col-12">
                                    <i class="display-3 py-4 wi $weatherMap.get($x[1].weather_code, "wi-na")" title="$pgettext("Forecast",str($x[1].weather_code))" data-toggle="tooltip" data-html="true"></i>
                                </div>
                            #end if
                        #else
                            #for $variable in $vars
                                <div class="col-6 text-left text-nowrap">
                                    #if $variable == "temperature"
                                        <div class="text-center d-inline-block">
                                            <i style="width: 20px;" class="wi wi-thermometer $Extras.color-text" title="$pgettext("Forecast","min_temp") / $pgettext("Forecast","max_temp")" data-toggle="tooltip" data-html="true"></i>
                                            <span>$x[1].temperature.min.format("%.0f") / $x[1].temperature.max.format("%.0f")</span>
                                        </div>
                                    #end if
                                    #if ($variable == "sun") or ($variable == "uv-sun")
                                        <div class="text-center d-inline-block">
                                            #if ($variable == "sun")
                                                <i class="wi wi-day-sunny $Extras.color-text" title="$pgettext("Forecast","sunshine_duration") / $pgettext("Forecast","daylight_duration")" data-toggle="tooltip" data-html="true"></i>
                                            #else
                                                <i class="wi wi-day-sunny $Extras.color-text" title="$pgettext("Forecast","uv") / $pgettext("Forecast","sunshine_duration") / $pgettext("Forecast","daylight_duration")" data-toggle="tooltip" data-html="true"></i>
                                            #end if
                                            #set daylight_hours = int($x[1].sun.daylight_duration.raw / 3600)
                                            #set daylight_minutes = int(($x[1].sun.daylight_duration.raw % 3600) / 60)
                                            #set sunshine_hours = int($x[1].sun.sunshine_duration.raw / 3600)
                                            #set sunshine_minutes = int(($x[1].sun.sunshine_duration.raw % 3600) / 60)
                                            <span>
                                                #if ($variable == "sun")
                                                    ${sunshine_hours}:${'%02d' % $sunshine_minutes}h / ${daylight_hours}:${'%02d' % $daylight_minutes}h
                                                #else
                                                    $x[1].sun.uv.format("%.0f") / ${sunshine_hours}:${'%02d' % $sunshine_minutes}h / ${daylight_hours}:${'%02d' % $daylight_minutes}h
                                                #end if
                                                <span>
                                                </div>
                                            #end if
                                            #if $variable == "precipitation"
                                                <div class="text-center d-inline-block">
                                                    <i style="width: 20px;" class="wi wi-umbrella $Extras.color-text" title="$pgettext("Forecast","precipitation") ($pgettext("Forecast","probability"))" data-toggle="tooltip" data-html="true"></i>
                                                    <span>$x[1].precipitation.sum ($x[1].precipitation.probability)</span>
                                                </div>
                                            #end if
                                            #if $variable == "uv"
                                                <div class="text-center d-inline-block">
                                                    <i style="width: 20px;" class="wi wi-day-sunny $Extras.color-text" title="$pgettext("Forecast","uv")" data-toggle="tooltip" data-html="true"></i>
                                                    <span>$x[1].sun.uv.format("%.0f")</span>
                                                </div>
                                            #end if
                                            #if $variable == "evapotranspiration"
                                                <div class="text-center d-inline-block">
                                                    <i style="width: 20px;" class="wi wi-hot $Extras.color-text" title="$pgettext("Forecast","evapotranspiration")" data-toggle="tooltip" data-html="true"></i>
                                                    <span>$x[1].et0_fao_evapotranspiration.format("%.1f")</span>
                                                </div>
                                            #end if
                                            #if $variable == "wind"
                                                <div class="text-center d-inline-block">
                                                    <i style="width: 20px;" class="wi wi-strong-wind $Extras.color-text" title="$pgettext("Forecast","wind_speed") & $pgettext("Forecast","wind_dir") ($pgettext("Forecast","wind_gust"))" data-toggle="tooltip" data-html="true"></i>
                                                    <span>$x[1].wind.speed.format("%.0f") <i class="wi wi-wind from-$x[1].wind.direction.formatted-deg mr-2" title="$x[1].wind.direction.formatted°" data-toggle="tooltip" data-html="true"></i> ($x[1].wind.gusts.format("%.0f"))</span>
                                                </div>
                                            #end if
                                        </div>
                                    #end for
                                #end if
                            </div>

                            #if $Extras.Forecast.show_hourly_icons == "yes"
                                #set $size = "font-extra-small" if int($Extras.Forecast.hourly_icons_interval) == 1 else "font-small"
                                #set $hour_line_height = "0.6rem" if int($Extras.Forecast.hourly_icons_interval) == 1 else "1.1rem"
                                #set $icon_line_height = "0.8rem" if int($Extras.Forecast.hourly_icons_interval) == 1 else "1.4rem"
                                <table class="text-center w-100 mb-1">
                                    <tr style="line-height: $hour_line_height;">
                                        #for $hour_index, $hourly_weather_code in enumerate($x[1].hourly_weather_codes)
                                            #set $hour = $hour_index * int($Extras.Forecast.hourly_icons_interval)
                                            #set $hour_until = ($hour_index + 1) * int($Extras.Forecast.hourly_icons_interval) - 1
                                            #if $i == 0 and $hour_until < $forecast.generated.hour
                                                <th class="$size text-muted">${"%02d" % $hour}</th>
                                            #else if $i == 0 and $hour <= $forecast.generated.hour and $hour_until >= $forecast.generated.hour
                                                <th class="$size $Extras.color-text">${"%02d" % $hour}</th>
                                            #else
                                                <th class="$size">${"%02d" % $hour}</th>
                                            #end if
                                        #end for
                                    </tr>
                                    <tr style="line-height: $icon_line_height;">
                                        #for $hour_index, $hourly_weather_code in enumerate($x[1].hourly_weather_codes)
                                            #set $hour = $hour_index * int($Extras.Forecast.hourly_icons_interval)
                                            #set $hour_until = ($hour_index + 1) * int($Extras.Forecast.hourly_icons_interval) - 1
                                            <td>
                                                #if $i == 0 and $hour_until < $forecast.generated.hour
                                                    <i class="$size text-muted wi $weatherMap.get($hourly_weather_code, "wi-na")" title="$pgettext("Forecast",str($hourly_weather_code))" data-toggle="tooltip" data-html="true"></i>
                                                #else if $i == 0 and $hour <= $forecast.generated.hour and $hour_until >= $forecast.generated.hour
                                                    <i class="$size $Extras.color-text wi $weatherMap.get($hourly_weather_code, "wi-na")" title="$pgettext("Forecast",str($hourly_weather_code))" data-toggle="tooltip" data-html="true"></i>
                                                #else
                                                    <i class="$size wi $weatherMap.get($hourly_weather_code, "wi-na")" title="$pgettext("Forecast",str($hourly_weather_code))" data-toggle="tooltip" data-html="true"></i>
                                                #end if
                                            </td>
                                        #end for
                                    </tr>
                                </table>
                            #end if

                            <div class="text-muted font-extra-small text-right">
                                Forecast by&nbsp;<a class="text-muted" href="https://open-meteo.com/">Open-Meteo.com</a>
                            </div>
                        </div>
                    </div>
                </div>
            #end for
            #if $Extras.Forecast.forecast_separator == "yes" and $Extras.Appearance.values_order[-1] != "forecast"
                <div class="col-12"><hr class="m-0 mb-4" /></div>
            #end if
        #end if