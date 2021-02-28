<header>

    <nav class="navbar navbar-expand-lg navbar-dark $Extras.color">
        <span class="navbar-brand d-flex">
            #if $Extras.show_logo == "yes"
            <span class="mr-2">
                <img src="img/icon-alpha-1x.png" srcset="img/icon-alpha-1x.png 1x, img/icon-alpha-2x.png 2x" style="width: 4rem">
            </span>
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

                #if $active_nav is 'current'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="index.html">$Extras.Translations[$Extras.language].current</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="index.html">$Extras.Translations[$Extras.language].current</a>
                </li>
                #end if

                #if $active_nav is 'yesterday'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="yesterday.html">$Extras.Translations[$Extras.language].yesterday</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="yesterday.html">$Extras.Translations[$Extras.language].yesterday</a>
                </li>
                #end if

                #if $active_nav is 'week'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="week.html">$Extras.Translations[$Extras.language].week</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="week.html">$Extras.Translations[$Extras.language].week</a>
                </li>
                #end if

                #if $active_nav is 'month'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="month.html">$Extras.Translations[$Extras.language].month</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="month.html">$Extras.Translations[$Extras.language].month</a>
                </li>
                #end if

                #if $active_nav is 'year'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="year.html">$Extras.Translations[$Extras.language].year</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="year.html">$Extras.Translations[$Extras.language].year</a>
                </li>
                #end if

                #if $active_nav is 'archive'
                <li class="nav-item active mr-3">
                    <a class="nav-link" href="archive.html">$Extras.Translations[$Extras.language].archive</a>
                </li>
                #else
                <li class="nav-item mr-3">
                    <a class="nav-link" href="archive.html">$Extras.Translations[$Extras.language].archive</a>
                </li>
                #end if

            </ul>
        </div>
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
            <li class="nav-item mr-4 text-white" style="line-height: 1.75" title="$almanac.moon_phase" data-toggle="tooltip">
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
                $almanac.moon_fullness% <br>
                <a href="almanac.html" class="text-white" style="opacity: .6">$Extras.Translations[$Extras.language].more</a>
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
    </nav>

</header>