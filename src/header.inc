<header>

    <nav class="navbar navbar-expand-lg navbar-dark indigo">
    <span class="navbar-brand" ><strong>$station.location</strong><br>
      <span class="font-small">$current.dateTime</span>
    </span>
        <button class="navbar-toggler" type="button" data-toggle="collapse" data-target="#navbarSupportedContent"
                aria-controls="navbarSupportedContent" aria-expanded="false" aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
        </button>
        <div class="ml-3 collapse navbar-collapse" id="navbarSupportedContent">
            <ul class="navbar-nav ml-auto mr-auto">

                <li class="nav-item mr-3">
                    <a class="nav-link" href="index.html">$Extras.Translations[$Extras.language].current</a>
                </li>

                <li class="nav-item mr-3">
                    <a class="nav-link" href="yesterday.html">$Extras.Translations[$Extras.language].yesterday</a>
                </li>

                <li class="nav-item mr-3">
                    <a class="nav-link" href="week.html">$Extras.Translations[$Extras.language].week</a>
                </li>

                <li class="nav-item mr-3">
                    <a class="nav-link" href="month.html">$Extras.Translations[$Extras.language].month</a>
                </li>

                <li class="nav-item mr-3">
                    <a class="nav-link" href="year.html">$Extras.Translations[$Extras.language].year</a>
                </li>

                <li class="nav-item mr-3">
                    <a class="nav-link" href="archive.html">$Extras.Translations[$Extras.language].archive</a>
                </li>

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
            <li class="nav-item mr-4 text-white" style="line-height: 1.75">
                #if $almanac.moon_phase == "New"
                <i class="wi wi-moon-new mr-1" style="opacity: .75"></i>
                #else if $almanac.moon_phase == "Waxing crescent"
                <i class="wi wi-moon-waxing-crescent-4 mr-1" style="opacity: .75"></i>
                #else if $almanac.moon_phase == "First quarter"
                <i class="wi wi-moon-first-quarter mr-1" style="opacity: .75"></i>
                #else if $almanac.moon_phase == "Waxing gibbous"
                <i class="wi wi-moon-waxing-gibbous-4 mr-1" style="opacity: .75"></i>
                #else if $almanac.moon_phase == "Full"
                <i class="wi wi-moon-full mr-1" style="opacity: .75"></i>
                #else if $almanac.moon_phase == "Waning gibbous"
                <i class="wi wi-moon-waning-gibbous-4 mr-1" style="opacity: .75"></i>
                #else if $almanac.moon_phase == "Last quarter"
                <i class="wi wi-moon-third-quarter mr-1" style="opacity: .75"></i>
                #else if $almanac.moon_phase == "Waning crescent"
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