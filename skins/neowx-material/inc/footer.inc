#encoding UTF-8
## +-------------------------------------------------------------------------+
## |    footer.inc                       Footer section for all templates    |
## +-------------------------------------------------------------------------+

#if not $varExists('base_path')
    #set $base_path = ''
#end if
#if not $varExists('page')
    #set $page = ''
#end if
#if not $varExists('subpage')
    #set $subpage = ''
#end if

<footer class="page-footer font-small $Extras.color nwm-collapsible-footer">

    <!-- Collapsible Drawer (About, System Health, Links) -->
    <div class="collapse nwm-footer-drawer" id="nwm-footer-drawer">
        <div class="container-fluid py-4 px-3 px-md-5">

            #set $show_uptime_col = ($Extras.Footer.show_uptime == "yes" or $Extras.Footer.box_title != "" or $Extras.Footer.box_content != "")
            #set $show_links_col = ($Extras.Footer.link1_url != "" or $Extras.Footer.link2_url != "" or $Extras.Footer.link3_url != "" or $Extras.Footer.link4_url != "")
            #set $col_count = 1
            #if $show_uptime_col
                #set $col_count = $col_count + 1
            #end if
            #if $show_links_col
                #set $col_count = $col_count + 1
            #end if

            #if $col_count == 3
                #set $col_class = "col-12 col-md-4 mb-3 mb-md-0"
            #else if $col_count == 2
                #set $col_class = "col-12 col-md-6 mb-3 mb-md-0"
            #else
                #set $col_class = "col-12 mb-3 mb-md-0"
            #end if

            <div class="row justify-content-center">

                <!-- Col 1: About / Hardware -->
                <div class="$col_class nwm-footer-col">
                    <h6 class="nwm-footer-heading">
                        <i class="mdi mdi-information-outline mr-2"></i>$gettext("about")
                    </h6>
                    <div class="nwm-footer-card">
                        #if $Extras.Footer.get('show_about_data', 'yes') == "yes"
                            <div class="nwm-footer-meta-row">
                                <span class="nwm-footer-meta-label">$gettext("hardware")</span>
                                <span class="nwm-footer-meta-value">$station.hardware</span>
                            </div>
                            <div class="nwm-footer-meta-row">
                                <span class="nwm-footer-meta-label">$gettext("altitude")</span>
                                <span class="nwm-footer-meta-value">$station.altitude</span>
                            </div>
                        #end if
                        #if $Extras.Footer.show_telemetry == "yes"
                            <div class="nwm-footer-meta-row">
                                <span class="nwm-footer-meta-label">$gettext("telemetry")</span>
                                <span class="nwm-footer-meta-value">
                                    <a href="${base_path}telemetry.html" class="nwm-footer-btn-chip">
                                        <i class="mdi mdi-chart-timeline-variant mr-1"></i>$gettext("telemetry")
                                    </a>
                                </span>
                            </div>
                        #end if
                        <div class="nwm-footer-meta-row">
                            <span class="nwm-footer-meta-label">$gettext("panels")</span>
                            <span class="nwm-footer-meta-value">
                                <button type="button" class="nwm-footer-btn-chip" id="nwm-reset-panels-btn" style="border:none; cursor:pointer;" title="$gettext('reset_panels_tooltip')">
                                    <i class="mdi mdi-restore mr-1"></i>$gettext("reset_panels")
                                </button>
                            </span>
                        </div>
                        #if $Extras.Footer.about != ""
                            <p class="nwm-footer-about-text mt-2 mb-0">$Extras.Footer.about</p>
                        #end if
                    </div>
                </div>

                <!-- Col 2: System Status / Live Uptimes -->
                #if $show_uptime_col
                    <div class="$col_class nwm-footer-col">
                        <h6 class="nwm-footer-heading">
                            <i class="mdi mdi-server-network mr-2"></i>$gettext("station_uptime")
                        </h6>
                        <div class="nwm-footer-card">
                            #if $Extras.Footer.show_uptime == "yes"
                                <div class="nwm-footer-meta-row">
                                    <span class="nwm-footer-meta-label">$gettext("station_uptime")</span>
                                    <span class="nwm-footer-meta-value" id="footer-station-uptime">$station.uptime.long_form</span>
                                </div>
                                <div class="nwm-footer-meta-row">
                                    <span class="nwm-footer-meta-label">$gettext("server_uptime")</span>
                                    <span class="nwm-footer-meta-value" id="footer-server-uptime">$station.os_uptime.long_form</span>
                                </div>
                            #end if
                            #if $Extras.Footer.box_title != "" or $Extras.Footer.box_content != ""
                                <div class="mt-2 pt-2 border-top" style="border-top-color: rgba(255,255,255,0.1) !important;">
                                    #if $Extras.Footer.box_title != ""
                                        <div class="font-weight-bold small mb-1">$Extras.Footer.box_title</div>
                                    #end if
                                    #if $Extras.Footer.box_content != ""
                                        <div class="small opacity-80">$Extras.Footer.box_content</div>
                                    #end if
                                </div>
                            #end if
                        </div>
                    </div>
                #end if

                <!-- Col 3: Links -->
                #if $show_links_col
                    <div class="$col_class nwm-footer-col">
                        <h6 class="nwm-footer-heading">
                            <i class="mdi mdi-link-variant mr-2"></i>$gettext("links")
                        </h6>
                        <div class="nwm-footer-card">
                            <div class="nwm-footer-links-grid">
                                #if $Extras.Footer.link1_url != ""
                                    <a href="$Extras.Footer.link1_url" target="_blank" rel="noopener" class="nwm-footer-link-chip">
                                        <i class="mdi mdi-open-in-new mr-1"></i>$Extras.Footer.link1_label
                                    </a>
                                #end if
                                #if $Extras.Footer.link2_url != ""
                                    <a href="$Extras.Footer.link2_url" target="_blank" rel="noopener" class="nwm-footer-link-chip">
                                        <i class="mdi mdi-open-in-new mr-1"></i>$Extras.Footer.link2_label
                                    </a>
                                #end if
                                #if $Extras.Footer.link3_url != ""
                                    <a href="$Extras.Footer.link3_url" target="_blank" rel="noopener" class="nwm-footer-link-chip">
                                        <i class="mdi mdi-open-in-new mr-1"></i>$Extras.Footer.link3_label
                                    </a>
                                #end if
                                #if $Extras.Footer.link4_url != ""
                                    <a href="$Extras.Footer.link4_url" target="_blank" rel="noopener" class="nwm-footer-link-chip">
                                        <i class="mdi mdi-open-in-new mr-1"></i>$Extras.Footer.link4_label
                                    </a>
                                #end if
                            </div>
                        </div>
                    </div>
                #end if

            </div>
        </div>
    </div>

    <!-- Visible Compact Bottom Bar -->
    <div class="nwm-footer-bar py-3 px-3">
        <div class="container-fluid d-flex flex-column flex-md-row align-items-center justify-content-between">

            <!-- Left / Center: Copyright & Attribution -->
            <div class="nwm-footer-info text-center text-md-left mb-2 mb-md-0">
                <div class="nwm-footer-text">
                    <span class="nwm-footer-item">&copy; <span id="footer-year">$current.dateTime.format("%Y")</span> $Extras.Footer.name</span>
                    #if $Extras.Footer.support_weewx == "yes"
                        <span class="nwm-sep">&bull;</span>
                        <span class="nwm-footer-item">Powered by <a href="https://weewx.com" target="_blank" rel="noopener">weewx</a>
                            #if $Extras.Footer.show_version == "yes"
                                <span class="nwm-version">v<span id="footer-weewx-version">$station.version</span></span>
                            #end if
                        </span>
                    #end if
                    #if $Extras.Footer.support_skin == "yes"
                        <span class="nwm-sep">&bull;</span>
                        <span class="nwm-footer-item"><a href="https://github.com/seehase/neowx-material" target="_blank" rel="noopener">Fork of NeoWX Material</a>
                            #if $Extras.Footer.show_version == "yes"
                                <span class="nwm-version">v<span id="footer-skin-version">$Extras.version</span></span>
                            #end if
                        </span>
                    #end if
                    #if $Extras.Footer.imprint_url != ""
                        <span class="nwm-sep">&bull;</span>
                        <span class="nwm-footer-item"><a href="$Extras.Footer.imprint_url" target="_blank" rel="noopener">$Extras.Footer.imprint_label</a></span>
                    #end if
                    #if $Extras.Footer.privacy_url != ""
                        <span class="nwm-sep">&bull;</span>
                        <span class="nwm-footer-item"><a href="$Extras.Footer.privacy_url" target="_blank" rel="noopener">$Extras.Footer.privacy_label</a></span>
                    #end if
                    #set $show_forecast_attribution = False
                    #if $varExists('forecast')
                        #set $show_forecast_attribution = True
                    #else if $varExists('Extras') and 'Forecast' in $Extras
                        #set $show_forecast_attribution = True
                    #else if $varExists('panelItems') and "forecast" in $panelItems('card', page='today', subpage='today')
                        #set $show_forecast_attribution = True
                    #end if
                    #if $show_forecast_attribution
                        <span class="nwm-sep nwm-disclaimer-sep">&bull;</span>
                        <span class="nwm-disclaimer-break"></span>
                        <span class="nwm-footer-disclaimer">Weather forecast data provided by <a href="https://open-meteo.com/" target="_blank" rel="noopener">Open-Meteo.com</a> under (<a href="https://creativecommons.org/licenses/by/4.0/" target="_blank" rel="noopener">CC BY 4.0</a>) license. Data may be modified (e.g., unit conversion, aggregation).</span>
                    #end if
                </div>
            </div>

            <!-- Right: Drawer Toggle Button -->
            <div class="nwm-footer-toggle-wrap mt-2 mt-md-0 flex-shrink-0 ml-md-3">
                <button type="button" class="btn nwm-footer-toggle-btn"
                        data-toggle="collapse" data-target="#nwm-footer-drawer"
                        aria-expanded="false" aria-controls="nwm-footer-drawer"
                        id="nwm-footer-toggle"
                        title="$gettext('about') &amp; $gettext('links')">
                    <i class="mdi mdi-information-outline mr-1"></i>
                    <span class="nwm-footer-toggle-text">$gettext("about") &amp; $gettext("links")</span>
                    <i class="mdi mdi-chevron-up ml-1 nwm-footer-chevron"></i>
                </button>
            </div>

        </div>

        ## Update notification
        #if $update_check and $update_check.available
            <div class="alert alert-info alert-dismissible fade show mx-3 my-2 text-center" role="alert">
                <span class="mdi mdi-information-outline mr-1"></span>
                $gettext("update_available")
                <strong>v$update_check.latest_version</strong>
                ($gettext("current"): v$update_check.current_version)
                <a href="$update_check.github_url" target="_blank" rel="noopener" class="alert-link ml-1">$gettext("view_on_github")</a>
            </div>
        #end if
    </div>

</footer>

<script>
document.addEventListener('DOMContentLoaded', function () {
    var toggleBtn = document.getElementById('nwm-footer-toggle');
    var drawer = document.getElementById('nwm-footer-drawer');
    if (!toggleBtn || !drawer) return;

    if (localStorage.getItem('nwm_footer_open') === '1') {
        if (window.jQuery && window.jQuery(drawer).collapse) {
            window.jQuery(drawer).collapse('show');
        } else {
            drawer.classList.add('show');
        }
        toggleBtn.setAttribute('aria-expanded', 'true');
        toggleBtn.classList.add('expanded');
    }

    if (window.jQuery) {
        window.jQuery(drawer).on('show.bs.collapse', function () {
            toggleBtn.setAttribute('aria-expanded', 'true');
            toggleBtn.classList.add('expanded');
            localStorage.setItem('nwm_footer_open', '1');
        });
        window.jQuery(drawer).on('hide.bs.collapse', function () {
            toggleBtn.setAttribute('aria-expanded', 'false');
            toggleBtn.classList.remove('expanded');
            localStorage.setItem('nwm_footer_open', '0');
        });
    }

    var resetBtn = document.getElementById('nwm-reset-panels-btn');
    if (resetBtn) {
        resetBtn.addEventListener('click', function (e) {
            e.preventDefault();
            try {
                sessionStorage.removeItem('nwm-panels');
                localStorage.removeItem('nwm-panels');
            } catch (err) {}
            window.location.reload();
        });
    }
});
</script>
