#encoding UTF-8
## +-------------------------------------------------------------------------+
## |    header.inc                 Header section (nav) for all templates    |
## +-------------------------------------------------------------------------+

## Add google tag manager if enabled
#if $Extras.Header.google_tagmanager_enable == "yes"
<!-- Google Tag Manager (noscript) -->
<noscript><iframe src="https://www.googletagmanager.com/ns.html?id=$Extras.Header.google_tagmanager_id"
                  height="0" width="0" style="display:none;visibility:hidden"></iframe></noscript>
<!-- End Google Tag Manager (noscript) -->
#end if

<header>

    <nav class="navbar navbar-expand-lg navbar-dark $Extras.color">
            <span class="navbar-brand d-flex">
                #if $Extras.Header.show_logo == "yes"
                    <a class="mr-2" href="index.html">
                        #if $Extras.Header.custom_logo_url != ""
                            <img src="$Extras.Header.custom_logo_url" style="width: 4rem">
                        #else
                            <img src="img/icon-alpha-1x.png" srcset="img/icon-alpha-1x.png 1x, img/icon-alpha-2x.png 2x" style="width: 4rem">
                        #end if
                    </a>
                #end if
                <span>
                    <strong>$station.location</strong><br>
                    #if $active_nav == 'archive'
                        <span class="font-small"></span>
                    #else
                        <span class="font-small" id="current-datetime" data-timestamp="$current.dateTime.raw">$current.dateTime</span>
                        #if ($Extras.MQTT.enabled == "true" or $Extras.MQTT.enabled == True) and $active_nav == 'current'
                            <span id="mqtt-status" class="font-small" style="margin-left: 8px; display: inline-block;">
                                <i id="mqtt-indicator" class="fas fa-share-nodes" data-toggle="tooltip" data-placement="bottom" style="vertical-align: middle;" title=""></i>
                            </span>
                        #end if
                    #end if
                    <span id="status-label" class="font-small red"><strong></strong></span>
                </span>
            </span>
        <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarSupportedContent"
                aria-controls="navbarSupportedContent" aria-expanded="false" aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
        </button>
        <div class="ml-3 collapse navbar-collapse" id="navbarSupportedContent">
            <ul class="navbar-nav ml-auto mr-auto">

                #if $Extras.Header.current_nav_link == "yes"
                #if $active_nav == 'current'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="index.html">$gettext("current")</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="index.html">$gettext("current")</a>
                </li>
                #end if
                #end if

                #if $Extras.Header.yesterday_nav_link == "yes"
                #if $active_nav == 'yesterday'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="yesterday.html">$gettext("yesterday")</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="yesterday.html">$gettext("yesterday")</a>
                </li>
                #end if
                #end if

                #if $Extras.Header.week_nav_link == "yes"
                #if $active_nav == 'week'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="week.html">$gettext("week")</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="week.html">$gettext("week")</a>
                </li>
                #end if
                #end if

                #if $Extras.Header.month_nav_link == "yes"
                #if $active_nav == 'month'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="month.html">$gettext("month")</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="month.html">$gettext("month")</a>
                </li>
                #end if
                #end if

                #if $Extras.Header.year_nav_link == "yes"
                #if $active_nav == 'year'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="year.html">$gettext("year")</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="year.html">$gettext("year")</a>
                </li>
                #end if
                #end if

                #if $Extras.Header.archive_nav_link == "yes"
                #if $active_nav == 'archive'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="archive.html">$gettext("archive")</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="archive.html">$gettext("archive")</a>
                </li>
                #end if
                #end if
                #if $active_nav == 'history'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="history.html">$gettext("history")</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="history.html">$gettext("history")</a>
                </li>
                #end if
                #if $Extras.Header.almanac_nav_link == "yes"
                #if $active_nav == 'almanac'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="almanac.html">$gettext("almanac")</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="almanac.html">$gettext("almanac")</a>
                </li>
                #end if
                #end if

                #if $Extras.Header.custom1_url != ""
                <li class="nav-item mr-3">
                    <a class="nav-link" href="$Extras.Header.custom1_url">$Extras.Header.custom1_label</a>
                </li>
                #end if

                #if $Extras.Header.custom2_url != ""
                <li class="nav-item mr-3">
                    <a class="nav-link" href="$Extras.Header.custom2_url">$Extras.Header.custom2_label</a>
                </li>
                #end if

            </ul>
        </div>

        #if $Extras.Header.show_almanac == "yes"
        #if $almanac.hasExtras
        <ul class="navbar-nav nav-flex-icons" style="line-height: 1.5; padding-top: .5rem; font-size: 11pt">
            <li class="nav-item mr-4 text-white">
                <i class="wi wi-sunrise mr-1" style="opacity: .75"></i> $almanac.sun.rise <br>
                <i class="wi wi-sunset mr-1" style="opacity: .75"></i> $almanac.sun.set
            </li>
            <li class="nav-item mr-4 text-white" style="line-height: 1.5">
                <i class="wi wi-moonrise mr-1" style="opacity: .75"></i> $almanac.moon.rise <br>
                <i class="wi wi-moonset mr-1" style="opacity: .75"></i> $almanac.moon.set
            </li>
            <li class="nav-item mr-4 text-white text-center" style="line-height: 1.5" >
                        <span title="$almanac.moon_phase" data-toggle="tooltip" data-html="true">
                            #if $almanac.moon_index == 0
                                <i class="wi wi-moon-new mr-1" style="opacity: .75"></i>
                            #else if $almanac.moon_index == 1
                                <i class="wi wi-moon-waxing-crescent-4 mr-1" style="opacity: .75"></i>
                            #else if $almanac.moon_index == 2
                                <i class="wi wi-moon-first-quarter mr-1" style="opacity: .75"></i>
                            #else if $almanac.moon_index == 3
                                <i class="wi wi-moon-waxing-gibbous-4 mr-1" style="opacity: .75"></i>
                            #else if $almanac.moon_index == 4
                                <i class="wi wi-moon-full mr-1" style="opacity: .75"></i>
                            #else if $almanac.moon_index == 5
                                <i class="wi wi-moon-waning-gibbous-4 mr-1" style="opacity: .75"></i>
                            #else if $almanac.moon_index == 6
                                <i class="wi wi-moon-third-quarter mr-1" style="opacity: .75"></i>
                            #else if $almanac.moon_index == 7
                                <i class="wi wi-moon-waning-crescent-4 mr-1" style="opacity: .75"></i>
                            #else
                                <i class="wi wi-moon-waning-crescent-3 mr-1" style="opacity: .75"></i>
                            #end if
                            $almanac.moon_fullness%
                        </span><br>
                <a href="almanac.html" class="text-white" style="opacity: .6">
                    $gettext("more")
                </a>
            </li>
        </ul>
        #else
        <ul class="navbar-nav nav-flex-icons" style="line-height: 1.5; padding-top: .5rem; font-size: 11pt">
            <li class="nav-item mr-4 text-white">
                <i class="wi wi-sunrise mr-1" style="opacity: .75"></i> $almanac.sunrise <br>
                <i class="wi wi-sunset mr-1" style="opacity: .75"></i> $almanac.sunset
            </li>
        </ul>
        #end if
        #end if
    </nav>

</header>
