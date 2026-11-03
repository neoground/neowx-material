#encoding UTF-8
#import datetime
#import time
## +-------------------------------------------------------------------------+
## |    header.inc                 Header section (nav) for all templates    |
## +-------------------------------------------------------------------------+

#if not $varExists('base_path')
    #set $base_path = ''
#end if
#if not $varExists('page')
    #set $page = ''
#end if
#if not $varExists('active_nav')
    #set $active_nav = ''
#end if
#if not $varExists('context_year')
    #set $context_year = $time.strftime('%Y')
#end if
#if not $varExists('context_month')
    #set $context_month = $time.strftime('%m')
#end if
#if not $varExists('context_month_name')
    #set $context_month_name = $time.strftime('%B')
#end if
#if not $varExists('context_week')
    #set $context_week = $time.strftime('%V')
#end if
#if not $varExists('context_day')
    #set $context_day = $time.strftime('%Y-%m-%d')
#end if
#if $varExists('alltime') and $alltime.start.raw is not None
    #set $weewx_first_date = $alltime.start.format('%Y-%m-%d')
#else
    #set $weewx_first_date = '1970-01-01'
#end if

#if not $varExists('archive_stop_date') or $archive_stop_date == ''
    #set $archive_stop_date = $time.strftime('%Y-%m-%d')
#end if
#if not $varExists('archive_start_date') or str($archive_start_date).strip().lower() in ('1970-01-01', '', 'all')
    #set $archive_start_date = $weewx_first_date
#else if str($archive_start_date).strip().lower() == 'current_year'
    #set $archive_start_date = $archive_stop_date[0:4] + '-01-01'
#else if str($archive_start_date).strip().lower() == 'current_month'
    #set $archive_start_date = $archive_stop_date[0:7] + '-01'
#else if str($archive_start_date).strip().lower() in ('current', 'today', 'none')
    #set $archive_start_date = $archive_stop_date
#end if
#if $weewx_first_date != '1970-01-01' and $weewx_first_date > $archive_start_date
    #set $archive_start_date = $weewx_first_date
#end if
#set $archive_start_year = int($archive_start_date.split('-')[0])
#set $archive_stop_year = int($archive_stop_date.split('-')[0])
#set $archive_years = [str($y) for $y in range($archive_start_year, $archive_stop_year + 1)]
#set $archive_years_reversed = [str($y) for $y in range($archive_stop_year, $archive_start_year - 1, -1)]
#if not $varExists('context_day_url')
    #if $context_day == $archive_stop_date or $context_day == $time.strftime('%Y-%m-%d')
        #set $context_day_url = $base_path + 'index.html'
    #else
        #set $context_day_url = $base_path + 'archive/' + $context_year + '/days/day-' + $context_day + '.html'
    #end if
#else
    #if $context_day_url.endswith('day-' + $archive_stop_date + '.html') or $context_day_url.endswith('day-' + $time.strftime('%Y-%m-%d') + '.html')
        #set $context_day_url = $base_path + 'index.html'
    #end if
#end if
#if not $varExists('context_week_url')
    #set $context_week_url = $base_path + 'archive/' + $context_year + '/weeks/week-' + $context_year + '-' + $context_week + '.html'
#end if
#if not $varExists('context_month_url')
    #set $context_month_url = $base_path + 'archive/' + $context_year + '/months/month-' + $context_year + '-' + $context_month + '.html'
#end if
#if not $varExists('context_year_url')
    #set $context_year_url = $base_path + 'archive/' + $context_year + '/year-' + $context_year + '.html'
#end if
#if not $varExists('current_day_url')
    #set $current_day_url = $base_path + 'index.html'
#end if
#if not $varExists('current_week_url')
    #set $current_week_url = $context_week_url
#end if
#if not $varExists('current_month_url')
    #set $current_month_url = $context_month_url
#end if
#if not $varExists('current_year_url')
    #set $current_year_url = $context_year_url
#end if
#if not $varExists('has_prev_day')
    #set $has_prev_day = True
#end if
#if not $varExists('has_next_day')
    #set $has_next_day = False
#end if
#if not $varExists('prev_day_url')
    #set $yest_dt = datetime.datetime.fromtimestamp($time.time()) - datetime.timedelta(days=1)
    #set $prev_day_url = $base_path + 'archive/' + $yest_dt.strftime('%Y') + '/days/day-' + $yest_dt.strftime('%Y-%m-%d') + '.html'
#end if
#if not $varExists('next_day_url')
    #set $next_day_url = '#'
#else
    #if $next_day_url.endswith('day-' + $archive_stop_date + '.html') or $next_day_url.endswith('day-' + $time.strftime('%Y-%m-%d') + '.html')
        #set $next_day_url = $base_path + 'index.html'
    #end if
#end if
#if not $varExists('has_prev_week')
    #set $has_prev_week = True
#end if
#if not $varExists('has_next_week')
    #set $has_next_week = False
#end if
#if not $varExists('prev_week_url')
    #set $prev_week_url = '#'
#end if
#if not $varExists('next_week_url')
    #set $next_week_url = '#'
#end if
#if not $varExists('has_prev_month')
    #set $has_prev_month = True
#end if
#if not $varExists('has_next_month')
    #set $has_next_month = False
#end if
#if not $varExists('prev_month_url')
    #set $prev_month_url = '#'
#end if
#if not $varExists('next_month_url')
    #set $next_month_url = '#'
#end if
#if not $varExists('has_prev_year')
    #set $has_prev_year = True
#end if
#if not $varExists('has_next_year')
    #set $has_next_year = False
#end if
#if not $varExists('prev_year_url')
    #set $prev_year_url = '#'
#end if
#if not $varExists('next_year_url')
    #set $next_year_url = '#'
#end if
#if $context_day <= $archive_start_date
    #set $has_prev_day = False
#end if
#if ($context_year + '-' + $context_month) <= $archive_start_date[0:7]
    #set $has_prev_month = False
#end if
#if int($context_year) <= $archive_start_year
    #set $has_prev_year = False
#end if
#if not $varExists('context_noaa_month_url')
    #set $context_noaa_month_url = $base_path + 'archive/NOAA/NOAA-' + $context_year + '-' + $context_month + '.txt'
#end if
#if not $varExists('context_noaa_year_url')
    #set $context_noaa_year_url = $base_path + 'archive/NOAA/NOAA-' + $context_year + '.txt'
#end if

## Add google tag manager if enabled
#if $Extras.Header.google_tagmanager_enable == "yes"
    <!-- Google Tag Manager (noscript) -->
    <noscript><iframe src="https://www.googletagmanager.com/ns.html?id=$Extras.Header.google_tagmanager_id"
        height="0" width="0" style="display:none;visibility:hidden"></iframe></noscript>
    <!-- End Google Tag Manager (noscript) -->
#end if

#if $active_nav != 'current'
    ## Anti-flash guard (non-current pages): hide the fields that neowx-live.js
    ## replaces from live.json, so this page's stale build-time values (e.g. an
    ## old date / sun-moon times on a day-old archive page) don't flash before
    ## the fetch resolves. JS-gated via the nx-live-pending class, so no-JS
    ## visitors still see the server-rendered fallback. visibility:hidden keeps
    ## the layout from shifting. The inline setTimeout is a safety net: if
    ## neowx-live.js never runs (e.g. fails to load), the fields still reveal.
    <script>
        document.documentElement.classList.add('nx-live-pending');
        setTimeout(function () {
            document.documentElement.classList.remove('nx-live-pending');
        }, 5000);
    </script>
    <style>
        .nx-live-pending #header-date,
        .nx-live-pending #alm-sun-rise, .nx-live-pending #alm-sun-set,
        .nx-live-pending #alm-sunrise, .nx-live-pending #alm-sunset,
        .nx-live-pending #alm-moon-rise, .nx-live-pending #alm-moon-set,
        .nx-live-pending #alm-moon-icon, .nx-live-pending #alm-moon-fullness,
        .nx-live-pending #footer-year,
        .nx-live-pending #footer-weewx-version, .nx-live-pending #footer-skin-version,
        .nx-live-pending #footer-station-uptime, .nx-live-pending #footer-server-uptime {
            visibility: hidden;
        }
    </style>
#end if



<header style="position: relative; z-index: 1030;">

    <nav class="navbar navbar-expand-lg navbar-dark $Extras.color" style="position: relative; z-index: 1030; padding: 0.4rem 1rem; min-height: 5.2rem;">

        <!-- ===== COLUMN 1: Brand / Station Info ===== -->
        <!-- Fixed height = guarantees logo + station name never move between pages -->
        <div class="d-flex align-items-center flex-shrink-0 mr-3" style="height: 4.5rem;">

            <!-- Sub-col A: Weather icon / logo (vertically centered within fixed height) -->
            #if $Extras.Header.show_logo == "yes"
                <a class="mr-3" href="${base_path}index.html" style="display:flex; align-items:center; height:100%; flex-shrink:0;">
                    #if $Extras.Header.custom_logo_url != ""
                        <img src="$Extras.Header.custom_logo_url" style="width: 4rem">
                    #else
                        <img src="${base_path}img/icon-alpha-1x.png" srcset="${base_path}img/icon-alpha-1x.png 1x, ${base_path}img/icon-alpha-2x.png 2x" style="width: 4rem">
                    #end if
                </a>
            #end if

            <!-- Sub-col B: flex-column, centered in fixed height so every row is always at the same Y -->
            <div style="display:flex; flex-direction:column; justify-content:center; height:100%; line-height:1.2;">

                <!-- Row 1: Station name -->
                <div style="font-size:20px; font-weight:700; color:#fff; white-space:nowrap; margin-bottom:0.3rem;">$station.location</div>

                <!-- Row 2: Date + icons (icons column always rendered, empty on non-current pages) -->
                <div style="display:flex; align-items:center; gap:0.4rem;">
                    #set $date_format = $Extras.Formatting.get('date', '%x')
                    #set $always_show_status = $Extras.OnlineCheck.get('always_show_status', False)
                    #if $active_nav == 'current'
                        <div id="current-date" style="white-space:nowrap; font-size:0.9rem; color:#fff;">$current.dateTime.format($date_format)</div>
                    #else
                        <!-- Server value is the page's build date (fallback); neowx-live.js overwrites it with today's date from live.json -->
                        <div id="header-date" style="white-space:nowrap; font-size:0.9rem; color:#fff;">$time.strftime($date_format)</div>
                    #end if
                    <!-- Icon container: position:relative so OFFLINE badge can float below without affecting row height -->
                    <div style="position:relative; display:flex; flex-direction:column; gap:0.1rem;">
                        <!-- Icons row -->
                        <div style="display:flex; align-items:center; gap:0.25rem; font-size:1.1rem;">
                            #if $active_nav == 'current'
                                #if str($Extras.MQTT.enabled).lower() == 'true'
                                    <div id="mqtt-status" class="text-white">
                                        <span id="mqtt-indicator" class="mdi mdi-resistor-nodes"
                                              data-toggle="tooltip" data-placement="bottom"
                                              style="vertical-align:middle;" title=""></span>
                                    </div>
                                #end if
                                #if $varExists('onlinecheck')
                                    #set $online_check = $onlinecheck.get_status()
                                #else
                                    #set $online_check = {'offline': False}
                                #end if
                                #if $online_check.offline
                                    <div id="online-status" class="text-white">
                                        <span class="mdi mdi-wifi-strength-alert-outline"
                                              data-toggle="tooltip" data-placement="bottom"
                                              style="vertical-align:middle;" title="$gettext('System is offline')"></span>
                                    </div>
                                #elif not $online_check.offline and str($always_show_status).lower() == 'true'
                                    <div id="online-status" class="text-white">
                                        <span class="mdi mdi-wifi"
                                              data-toggle="tooltip" data-placement="bottom"
                                              style="vertical-align:middle;" title="$gettext('System is online')"></span>
                                    </div>
                                #end if
                            #else
                                <!-- Calendar icon: hidden by default; neowx-live.js reveals it when this
                                     page's build date (data-generated-date) differs from today. The title
                                     shows when the page content was generated. -->
                                <div id="generated-status" class="text-white" style="display:none;"
                                     data-generated-ts="$time.time()"
                                     data-old-threshold="$Extras.Header.get('old_page_threshold_minutes', '1440')">
                                    <span class="mdi mdi-calendar-clock"
                                          data-toggle="tooltip" data-placement="bottom"
                                          style="vertical-align:middle;"
                                          title="$time.strftime($Extras.Formatting.get('datetime_generated', '%d.%m.%Y %H:%M'))"></span>
                                </div>
                                <!-- Transparent spacer — keeps icon-column height identical to current page -->
                                <span style="font-size:1.1rem; color:transparent;" aria-hidden="true">&#9632;</span>
                            #end if
                        </div>
                        <!-- OFFLINE badge: absolutely positioned below icons, left-aligned with MQTT icon.
                             position:absolute means it does NOT affect the flex-row height — no layout shift. -->
                        <span id="status-label"
                              style="display:none; position:absolute; top:100%; left:0; margin-top:3px;
                                     font-size:0.65rem; font-weight:700; line-height:1.4;
                                     color:#ffffff; background-color:#f44336;
                                     padding:1px 4px; border-radius:2px; white-space:nowrap;"></span>
                    </div>
                </div>

                <!-- Row 3: Time only -->
                #if $active_nav == 'current'
                    <div id="current-time" style="white-space:nowrap; font-size:0.9rem; color:#fff;">$current.dateTime.format($Extras.Formatting.time)</div>
                    <span id="current-datetime" data-timestamp="$current.dateTime.raw" style="display:none;"></span>
                #else
                    <div style="white-space:nowrap; font-size:0.9rem; color:transparent;" aria-hidden="true">00:00:00</div>
                #end if
            </div>

        </div><!-- end column 1 -->

        <!-- Mobile toggle button -->
        <button class="navbar-toggler" type="button" data-toggle="collapse"
                data-target="#navbarSupportedContent" aria-controls="navbarSupportedContent"
                aria-expanded="false" aria-label="Toggle navigation">
            <span class="navbar-toggler-icon"></span>
        </button>

        <!-- ===== COLUMN 2: Navigation Hub (Option 2: Stepper + Scale Menu) ===== -->
        <div class="collapse navbar-collapse justify-content-center" id="navbarSupportedContent" style="flex: 1 1 auto; position: relative; z-index: 1031;">
            <div class="nwm-nav-hub my-1" id="nwm-nav-hub" style="position: relative; z-index: 1032;"
                 data-base-path="${base_path}"
                 data-page="${page}"
                 data-active-nav="${active_nav}"
                 data-context-year="${context_year}"
                 data-context-month="${context_month}"
                 data-context-week="${context_week}"
                 data-context-day="${context_day}"
                 data-start-date="${archive_start_date}"
                 data-stop-date="${archive_stop_date}"
                 data-start-year="${archive_start_year}"
                 data-stop-year="${archive_stop_year}">

                <!-- Stepper Controls (Adaptive per page scale) -->
                <div class="nwm-stepper-group">

                    #if $page in ('day', 'today') or $active_nav in ('current', 'live', 'day')
                        #if $has_prev_day
                            <a href="$prev_day_url" class="nwm-nav-btn" data-toggle="tooltip" data-placement="bottom" title="$gettext('previous')" aria-label="$gettext('previous')">
                                <i class="mdi mdi-chevron-left"></i>
                            </a>
                        #else
                            <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('no_earlier_data')" aria-label="$gettext('no_earlier_data')">
                                <i class="mdi mdi-chevron-left"></i>
                            </button>
                        #end if

                        <div class="nwm-header-center dropdown" id="nx-picker-container">
                            <button class="nwm-header-picker-btn" id="nx-picker-btn" type="button"
                                    data-toggle="dropdown" aria-haspopup="true" aria-expanded="false"
                                    title="$gettext('select_date')">
                                <i class="mdi mdi-calendar-today mr-1" style="font-size:0.9rem; opacity:0.85;"></i>
                                <span class="nwm-picker-text" id="nx-picker-label">$context_day</span>
                                <i class="mdi mdi-menu-down nwm-picker-arrow"></i>
                            </button>
                            <div class="dropdown-menu dropdown-menu-center nwm-picker-dropdown nwm-picker-calendar" aria-labelledby="nx-picker-btn" id="nx-picker-dropdown">
                                <!-- Populated dynamically by neowx-date-nav.js -->
                            </div>
                        </div>

                        #if ($active_nav not in ('current', 'live') and $page != 'today') and $has_next_day
                            <a href="$next_day_url" class="nwm-nav-btn" data-toggle="tooltip" data-placement="bottom" title="$gettext('next')" aria-label="$gettext('next')">
                                <i class="mdi mdi-chevron-right"></i>
                            </a>
                        #else
                            <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('no_later_data')" aria-label="$gettext('no_later_data')">
                                <i class="mdi mdi-chevron-right"></i>
                            </button>
                        #end if

                    #else if $page == 'week' or $active_nav == 'week'
                        #if $has_prev_week
                            <a href="$prev_week_url" class="nwm-nav-btn" data-toggle="tooltip" data-placement="bottom" title="$gettext('previous')" aria-label="$gettext('previous')">
                                <i class="mdi mdi-chevron-left"></i>
                            </a>
                        #else
                            <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('no_earlier_data')" aria-label="$gettext('no_earlier_data')">
                                <i class="mdi mdi-chevron-left"></i>
                            </button>
                        #end if

                        <div class="nwm-header-center dropdown" id="nx-picker-container">
                            <button class="nwm-header-picker-btn" id="nx-picker-btn" type="button"
                                    data-toggle="dropdown" aria-haspopup="true" aria-expanded="false"
                                    title="$gettext('select_week')">
                                <span class="nwm-picker-text" id="nx-picker-label">${context_year} - $gettext("CW") ${context_week}</span>
                                <i class="mdi mdi-menu-down nwm-picker-arrow"></i>
                            </button>
                            <div class="dropdown-menu dropdown-menu-center nwm-picker-dropdown nwm-picker-week" aria-labelledby="nx-picker-btn" id="nx-picker-dropdown">
                                <!-- Populated dynamically by neowx-date-nav.js -->
                            </div>
                        </div>

                        #if $has_next_week
                            <a href="$next_week_url" class="nwm-nav-btn" data-toggle="tooltip" data-placement="bottom" title="$gettext('next')" aria-label="$gettext('next')">
                                <i class="mdi mdi-chevron-right"></i>
                            </a>
                        #else
                            <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('no_later_data')" aria-label="$gettext('no_later_data')">
                                <i class="mdi mdi-chevron-right"></i>
                            </button>
                        #end if

                    #else if $page == 'month' or $active_nav == 'month'
                        #if $has_prev_month
                            <a href="$prev_month_url" class="nwm-nav-btn" data-toggle="tooltip" data-placement="bottom" title="$gettext('previous')" aria-label="$gettext('previous')">
                                <i class="mdi mdi-chevron-left"></i>
                            </a>
                        #else
                            <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('no_earlier_data')" aria-label="$gettext('no_earlier_data')">
                                <i class="mdi mdi-chevron-left"></i>
                            </button>
                        #end if

                        <div class="nwm-header-center dropdown" id="nx-picker-container">
                            <button class="nwm-header-picker-btn" id="nx-picker-btn" type="button"
                                    data-toggle="dropdown" aria-haspopup="true" aria-expanded="false"
                                    title="$gettext('select_month')">
                                <span class="nwm-picker-text" id="nx-picker-label">${context_year}-${context_month}</span>
                                <i class="mdi mdi-menu-down nwm-picker-arrow"></i>
                            </button>
                            <div class="dropdown-menu dropdown-menu-center nwm-picker-dropdown nwm-picker-month" aria-labelledby="nx-picker-btn" id="nx-picker-dropdown">
                                <!-- Populated dynamically by neowx-date-nav.js -->
                            </div>
                        </div>

                        #if $has_next_month
                            <a href="$next_month_url" class="nwm-nav-btn" data-toggle="tooltip" data-placement="bottom" title="$gettext('next')" aria-label="$gettext('next')">
                                <i class="mdi mdi-chevron-right"></i>
                            </a>
                        #else
                            <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('no_later_data')" aria-label="$gettext('no_later_data')">
                                <i class="mdi mdi-chevron-right"></i>
                            </button>
                        #end if

                    #else if $page == 'year' or $active_nav == 'year'
                        #if $has_prev_year
                            <a href="$prev_year_url" class="nwm-nav-btn" data-toggle="tooltip" data-placement="bottom" title="$gettext('previous')" aria-label="$gettext('previous')">
                                <i class="mdi mdi-chevron-left"></i>
                            </a>
                        #else
                            <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('no_earlier_data')" aria-label="$gettext('no_earlier_data')">
                                <i class="mdi mdi-chevron-left"></i>
                            </button>
                        #end if

                        <div class="nwm-header-center dropdown" id="nx-picker-container">
                            <button class="nwm-header-picker-btn" id="nx-picker-btn" type="button"
                                    data-toggle="dropdown" aria-haspopup="true" aria-expanded="false"
                                    title="$gettext('select_year')">
                                <span class="nwm-picker-text" id="nx-picker-label">${context_year}</span>
                                <i class="mdi mdi-menu-down nwm-picker-arrow"></i>
                            </button>
                            <div class="dropdown-menu dropdown-menu-center nwm-picker-dropdown nwm-picker-year" aria-labelledby="nx-picker-btn" id="nx-picker-dropdown">
                                <div class="nwm-picker-header">
                                    <span class="nwm-picker-title">$gettext('year')</span>
                                </div>
                                <div class="nwm-picker-grid-years">
                                    #for $yr in $archive_years_reversed
                                    <a class="nwm-picker-cell#if $yr == $context_year# active#end if#"
                                       href="${base_path}archive/${yr}/year-${yr}.html">${yr}</a>
                                    #end for
                                </div>
                            </div>
                        </div>

                        #if $has_next_year
                            <a href="$next_year_url" class="nwm-nav-btn" data-toggle="tooltip" data-placement="bottom" title="$gettext('next')" aria-label="$gettext('next')">
                                <i class="mdi mdi-chevron-right"></i>
                            </a>
                        #else
                            <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('no_later_data')" aria-label="$gettext('no_later_data')">
                                <i class="mdi mdi-chevron-right"></i>
                            </button>
                        #end if

                    #else if $page == 'history' or $active_nav == 'history'
                        <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('previous')" aria-label="$gettext('previous')">
                            <i class="mdi mdi-chevron-left"></i>
                        </button>

                        <div class="nwm-header-center">
                            <span class="nwm-header-label">$gettext("history")</span>
                        </div>

                        <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('next')" aria-label="$gettext('next')">
                            <i class="mdi mdi-chevron-right"></i>
                        </button>

                    #else if $page == 'almanac' or $active_nav == 'almanac'
                        <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('previous')" aria-label="$gettext('previous')">
                            <i class="mdi mdi-chevron-left"></i>
                        </button>

                        <div class="nwm-header-center">
                            <span class="nwm-header-label">$gettext("almanac")</span>
                        </div>

                        <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('next')" aria-label="$gettext('next')">
                            <i class="mdi mdi-chevron-right"></i>
                        </button>

                    #else
                        <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('previous')" aria-label="$gettext('previous')">
                            <i class="mdi mdi-chevron-left"></i>
                        </button>

                        <div class="nwm-header-center">
                            <span class="nwm-header-label">#if $varExists('page') and $page#$gettext($page)#else#$station.location#end if#</span>
                        </div>

                        <button class="nwm-nav-btn disabled" disabled data-toggle="tooltip" data-placement="bottom" title="$gettext('next')" aria-label="$gettext('next')">
                            <i class="mdi mdi-chevron-right"></i>
                        </button>

                    #end if

                </div><!-- end nwm-stepper-group -->

                <div class="nwm-hub-divider d-none d-lg-block"></div>

                <!-- NOAA Report Icon (Fixed position: datepicker | NOAA | navigation) -->
                <div class="nwm-noaa-slot">
                    #if $active_nav == 'month' or $page == 'month'
                        <a href="$context_noaa_month_url" target="_blank" rel="noopener" class="nwm-nav-btn"
                           data-toggle="tooltip" data-placement="bottom"
                           title="$gettext('show_noaa_data')" aria-label="$gettext('show_noaa_data')">
                            <i class="mdi mdi-file-document-outline"></i>
                        </a>
                    #else if $active_nav == 'year' or $page == 'year'
                        <a href="$context_noaa_year_url" target="_blank" rel="noopener" class="nwm-nav-btn"
                           data-toggle="tooltip" data-placement="bottom"
                           title="$gettext('show_noaa_data')" aria-label="$gettext('show_noaa_data')">
                            <i class="mdi mdi-file-document-outline"></i>
                        </a>
                    #else
                        <button class="nwm-nav-btn disabled" disabled
                                data-toggle="tooltip" data-placement="bottom"
                                title="$gettext('noaa_not_available')" aria-label="$gettext('noaa_not_available')">
                            <i class="mdi mdi-file-document-outline"></i>
                        </button>
                    #end if
                </div>

                <div class="nwm-hub-divider d-none d-lg-block"></div>

                <!-- Navigation Links -->
                <ul class="navbar-nav flex-row flex-wrap align-items-center justify-content-center mb-0">

                    #if $Extras.Header.get('live_nav_link', 'yes') == "yes" or $Extras.Header.get('current_nav_link', 'yes') == "yes"
                        <li class="nav-item#if $active_nav in ('current', 'live')# active#end if# px-1">
                            <a class="nav-link" id="nav-current" href="${base_path}index.html">$gettext("current")</a>
                        </li>
                    #end if

                    #if $Extras.Header.get('day_nav_link', 'no') == "yes"
                        <li class="nav-item#if $active_nav == 'day'# active#end if# px-1">
                            <a class="nav-link" id="nav-day" href="$context_day_url">$gettext("Day")</a>
                        </li>
                    #end if

                    #if $Extras.Header.week_nav_link == "yes"
                        <li class="nav-item#if $active_nav == 'week'# active#end if# px-1">
                            <a class="nav-link" id="nav-week" href="$context_week_url">$gettext("week")</a>
                        </li>
                    #end if

                    #if $Extras.Header.month_nav_link == "yes"
                        <li class="nav-item#if $active_nav == 'month'# active#end if# px-1">
                            <a class="nav-link" id="nav-month" href="$context_month_url">$gettext("month")</a>
                        </li>
                    #end if

                    #if $Extras.Header.year_nav_link == "yes"
                        <li class="nav-item#if $active_nav == 'year'# active#end if# px-1">
                            <a class="nav-link" id="nav-year" href="$context_year_url">$gettext("year")</a>
                        </li>
                    #end if

                    <li class="nav-item#if $active_nav == 'history'# active#end if# px-1">
                        <a class="nav-link" href="${base_path}history.html">$gettext("history")</a>
                    </li>

                    #if $Extras.Header.almanac_nav_link == "yes"
                        <li class="nav-item#if $active_nav == 'almanac'# active#end if# px-1">
                            <a class="nav-link" href="${base_path}almanac.html">$gettext("almanac")</a>
                        </li>
                    #end if

                    #if $Extras.Header.custom1_url != ""
                        <li class="nav-item px-1">
                            <a class="nav-link" href="$Extras.Header.custom1_url">$Extras.Header.custom1_label</a>
                        </li>
                    #end if

                    #if $Extras.Header.custom2_url != ""
                        <li class="nav-item px-1">
                            <a class="nav-link" href="$Extras.Header.custom2_url">$Extras.Header.custom2_label</a>
                        </li>
                    #end if

                </ul>

            </div><!-- end nwm-nav-hub -->
        </div><!-- end column 2 -->

        <!-- ===== COLUMN 3: Almanac ===== -->
        #if $Extras.Header.show_almanac == "yes"
            #if $almanac.hasExtras
                <ul class="navbar-nav nav-flex-icons flex-shrink-0 ml-3" style="line-height: 1.5; padding-top: .25rem; font-size: 0.9rem;">
                    <li class="nav-item mr-4 text-white" style="font-size: 0.9rem;">
                        <i class="wi wi-sunrise mr-1" style="opacity: .75"></i> <span id="alm-sun-rise">$almanac.sun.rise</span> <br>
                        <i class="wi wi-sunset mr-1" style="opacity: .75"></i> <span id="alm-sun-set">$almanac.sun.set</span>
                    </li>
                    <li class="nav-item mr-4 text-white" style="line-height: 1.5; font-size: 0.9rem;">
                        <i class="wi wi-moonrise mr-1" style="opacity: .75"></i> <span id="alm-moon-rise">$almanac.moon.rise</span> <br>
                        <i class="wi wi-moonset mr-1" style="opacity: .75"></i> <span id="alm-moon-set">$almanac.moon.set</span>
                    </li>
                    <li class="nav-item mr-2 text-white text-center" style="line-height: 1.5; font-size: 0.9rem;">
                        <span id="alm-moon-phase" title="$almanac.moon_phase" data-toggle="tooltip" data-html="true">
                            <span id="alm-moon-icon">
                            #set $moon_icons = {0: 'wi-moon-new', 1: 'wi-moon-waxing-crescent-4', 2: 'wi-moon-first-quarter', 3: 'wi-moon-waxing-gibbous-4', 4: 'wi-moon-full', 5: 'wi-moon-waning-gibbous-4', 6: 'wi-moon-third-quarter', 7: 'wi-moon-waning-crescent-4'}
                            #set $moon_icon_class = $moon_icons.get($almanac.moon_index, 'wi-moon-waning-crescent-3')
                            <i class="wi $moon_icon_class mr-1" style="opacity: .75"></i>
                            </span>
                            <span id="alm-moon-fullness">$almanac.moon_fullness</span>%
                        </span><br>
                        <a href="${base_path}almanac.html" class="text-white" style="opacity: .6; font-size: 0.85rem;">
                            $gettext("more")
                        </a>
                    </li>
                </ul>
            #else
                <ul class="navbar-nav nav-flex-icons flex-shrink-0 ml-3" style="line-height: 1.5; padding-top: .25rem; font-size: 0.9rem;">
                    <li class="nav-item mr-4 text-white" style="font-size: 0.9rem;">
                        <i class="wi wi-sunrise mr-1" style="opacity: .75"></i> <span id="alm-sunrise">$almanac.sunrise</span> <br>
                        <i class="wi wi-sunset mr-1" style="opacity: .75"></i> <span id="alm-sunset">$almanac.sunset</span>
                    </li>
                </ul>
            #end if
        #end if
        ## end column 3

    </nav>

    <!-- 404 Protection Modal -->
    <div class="modal fade" id="nwm-date-missing-modal" tabindex="-1" role="dialog" aria-labelledby="nwm-date-missing-title" aria-hidden="true" style="z-index: 1075;">
        <div class="modal-dialog modal-dialog-centered" role="document" style="max-width: 440px;">
            <div class="modal-content text-left shadow-lg">
                <div class="modal-header border-bottom-0 pb-0">
                    <h5 class="modal-title font-weight-bold d-flex align-items-center" id="nwm-date-missing-title" style="font-size: 1.1rem; color: #f44336;">
                        <i class="mdi mdi-alert-circle-outline mr-2" style="font-size: 1.4rem;"></i>
                        <span>$gettext('no_data_modal_title')</span>
                    </h5>
                    <button type="button" class="close" data-dismiss="modal" aria-label="$gettext('close')">
                        <span aria-hidden="true">&times;</span>
                    </button>
                </div>
                <div class="modal-body py-3">
                    <p class="mb-2" style="font-size: 0.95rem; opacity: 0.9;">
                        $gettext('no_data_modal_text')
                    </p>
                    <p class="text-muted small mb-0">
                        $gettext('redirecting_to_live') <span id="nwm-redirect-countdown" class="badge badge-secondary ml-1">3s</span>
                    </p>
                </div>
                <div class="modal-footer border-top-0 pt-0">
                    <button type="button" class="btn btn-outline-secondary btn-sm" data-dismiss="modal">
                        $gettext('stay_here')
                    </button>
                    <a href="${base_path}index.html" class="btn btn-primary btn-sm ml-2" id="nwm-redirect-now-btn">
                        $gettext('go_to_live')
                    </a>
                </div>
            </div>
        </div>
    </div>

    <script>
    function nxJumpDate(type, val) {
        if (!val) return;
        var base = '${base_path}';
        var targetUrl = '';
        if (type === 'day') {
            var parts = val.split('-');
            if (parts.length === 3) {
                if (val === '${archive_stop_date}' || val === new Date().toISOString().slice(0, 10)) {
                    targetUrl = base + 'index.html';
                } else {
                    targetUrl = base + 'archive/' + parts[0] + '/days/day-' + val + '.html';
                }
            }
        } else if (type === 'week') {
            var parts = val.split('-W');
            if (parts.length === 2) {
                targetUrl = base + 'archive/' + parts[0] + '/weeks/week-' + parts[0] + '-' + parts[1] + '.html';
            }
        } else if (type === 'month') {
            var parts = val.split('-');
            if (parts.length === 2) {
                targetUrl = base + 'archive/' + parts[0] + '/months/month-' + val + '.html';
            }
        } else if (type === 'year') {
            targetUrl = base + 'archive/' + val + '/year-' + val + '.html';
        }

        if (targetUrl) {
            if (window.nxNavigateWithCheck) {
                window.nxNavigateWithCheck(targetUrl);
            } else {
                window.location.href = targetUrl;
            }
        }
    }
    </script>

</header>