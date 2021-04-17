#encoding UTF-8
## +-------------------------------------------------------------------------+
## |    header.inc                 Header section (nav) for all templates    |
## +-------------------------------------------------------------------------+
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
                <span class="font-small">$current.dateTime</span>
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
                        <a class="nav-link" href="index.html">$Extras.Translations[$Extras.language].current</a>
                    </li>
                    #else
                    <li class="nav-item mr-3">
                        <a class="nav-link" href="index.html">$Extras.Translations[$Extras.language].current</a>
                    </li>
                    #end if
                #end if

                #if $Extras.Header.yesterday_nav_link == "yes"
                    #if $active_nav == 'yesterday'
                    <li class="nav-item active mr-3">
                        <a class="nav-link" href="yesterday.html">$Extras.Translations[$Extras.language].yesterday</a>
                    </li>
                    #else
                    <li class="nav-item mr-3">
                        <a class="nav-link" href="yesterday.html">$Extras.Translations[$Extras.language].yesterday</a>
                    </li>
                    #end if
                #end if

                #if $Extras.Header.week_nav_link == "yes"
                    #if $active_nav == 'week'
                    <li class="nav-item active mr-3">
                        <a class="nav-link" href="week.html">$Extras.Translations[$Extras.language].week</a>
                    </li>
                    #else
                    <li class="nav-item mr-3">
                        <a class="nav-link" href="week.html">$Extras.Translations[$Extras.language].week</a>
                    </li>
                    #end if
                #end if

                #if $Extras.Header.month_nav_link == "yes"
                    #if $active_nav == 'month'
                    <li class="nav-item active mr-3">
                        <a class="nav-link" href="month.html">$Extras.Translations[$Extras.language].month</a>
                    </li>
                    #else
                    <li class="nav-item mr-3">
                        <a class="nav-link" href="month.html">$Extras.Translations[$Extras.language].month</a>
                    </li>
                    #end if
                #end if

                #if $Extras.Header.year_nav_link == "yes"
                    #if $active_nav == 'year'
                    <li class="nav-item active mr-3">
                        <a class="nav-link" href="year.html">$Extras.Translations[$Extras.language].year</a>
                    </li>
                    #else
                    <li class="nav-item mr-3">
                        <a class="nav-link" href="year.html">$Extras.Translations[$Extras.language].year</a>
                    </li>
                    #end if
                #end if

                #if $Extras.Header.archive_nav_link == "yes"
                    #if $active_nav == 'archive'
                    <li class="nav-item active mr-3">
                        <a class="nav-link" href="archive.html">$Extras.Translations[$Extras.language].archive</a>
                    </li>
                    #else
                    <li class="nav-item mr-3">
                        <a class="nav-link" href="archive.html">$Extras.Translations[$Extras.language].archive</a>
                    </li>
                    #end if
                #end if

                #if $Extras.Header.almanac_nav_link == "yes"
                    #if $active_nav == 'almanac'
                    <li class="nav-item active mr-3">
                        <a class="nav-link" href="almanac.html">$Extras.Translations[$Extras.language].almanac</a>
                    </li>
                    #else
                    <li class="nav-item mr-3">
                        <a class="nav-link" href="almanac.html">$Extras.Translations[$Extras.language].almanac</a>
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

        #if $active_nav != 'archive' and $Extras.Header.show_almanac == "yes"
            #if $almanac.hasExtras
            <ul class="navbar-nav nav-flex-icons" style="line-height: 1.75; padding-top: .5rem">
                <li class="nav-item mr-4 text-white">
                    <i class="wi wi-sunrise mr-1" style="opacity: .75"></i> $almanac.sun.rise <br>
                    <i class="wi wi-sunset mr-1" style="opacity: .75"></i> $almanac.sun.set
                </li>
                <li class="nav-item mr-4 text-white" style="line-height: 1.75">
                    <i class="wi wi-moonrise mr-1" style="opacity: .75"></i> $almanac.moon.rise <br>
                    <i class="wi wi-moonset mr-1" style="opacity: .75"></i> $almanac.moon.set
                </li>
                <li class="nav-item mr-4 text-white text-center" style="line-height: 1.75" >
                    <span title="$almanac.moon_phase" data-toggle="tooltip">
                        #if $almanac.moon_phase == $almanac.moon_phases[0]
                        <i class="wi wi-moon-new mr-1" style="opacity: .75"></i>
                        #else if $almanac.moon_phase == $almanac.moon_phases[1]
                        <i class="wi wi-moon-waxing-crescent-4 mr-1" style="opacity: .75"></i>
                        #else if $almanac.moon_phase == $almanac.moon_phases[2]
                        <i class="wi wi-moon-first-quarter mr-1" style="opacity: .75"></i>
                        #else if $almanac.moon_phase == $almanac.moon_phases[3]
                        <i class="wi wi-moon-waxing-gibbous-4 mr-1" style="opacity: .75"></i>
                        #else if $almanac.moon_phase == $almanac.moon_phases[4]
                        <i class="wi wi-moon-full mr-1" style="opacity: .75"></i>
                        #else if $almanac.moon_phase == $almanac.moon_phases[5]
                        <i class="wi wi-moon-waning-gibbous-4 mr-1" style="opacity: .75"></i>
                        #else if $almanac.moon_phase == $almanac.moon_phases[6]
                        <i class="wi wi-moon-third-quarter mr-1" style="opacity: .75"></i>
                        #else if $almanac.moon_phase == $almanac.moon_phases[7]
                        <i class="wi wi-moon-waning-crescent-4 mr-1" style="opacity: .75"></i>
                        #else
                        <i class="wi wi-moon-waning-crescent-3 mr-1" style="opacity: .75"></i>
                        #end if
                        $almanac.moon_fullness%
                    </span><br>
                    <a href="almanac.html" class="text-white" style="opacity: .6">
                        $Extras.Translations[$Extras.language].more
                    </a>
                </li>
            </ul>
            #else
            <ul class="navbar-nav nav-flex-icons" style="line-height: 1.75; padding-top: .5rem">
                <li class="nav-item mr-4 text-white">
                    <i class="wi wi-sunrise mr-1" style="opacity: .75"></i> $almanac.sunrise <br>
                    <i class="wi wi-sunset mr-1" style="opacity: .75"></i> $almanac.sunset
                </li>
            </ul>
            #end if
        #end if
    </nav>

</header>