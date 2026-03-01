#encoding UTF-8
## +-------------------------------------------------------------------------+
## |    footer.inc                       Footer section for all templates    |
## +-------------------------------------------------------------------------+

<footer class="page-footer font-small $Extras.color pt-4">

    ## Footer Links
    <div class="container-fluid text-center text-md-left mb-4">

        ## Grid row
        <div class="row">

            ## Grid column
            <div class="col-md-5 mt-md-0 mt-3">

                ## Content
                <h5 class="text-uppercase">$gettext("about")</h5>

                <table class="d-inline">
                    <tr>
                        <td>$gettext("hardware"):</td>
                        <td>$station.hardware</td>
                    </tr>
                    <tr>
                        <td>$gettext("altitude"):</td>
                        <td>$station.altitude</td>
                    </tr>
                    #if $Extras.Footer.show_uptime == "yes"
                        <tr>
                            <td>$gettext("station_uptime"):</td>
                            <td>$station.uptime.long_form</td>
                        </tr>
                        <tr>
                            <td>$gettext("server_uptime"):</td>
                            <td>$station.os_uptime.long_form</td>
                        </tr>
                    #end if
                    #if $Extras.Footer.show_telemetry == "yes"
                        <tr>
                            <td>
                                <a href="telemetry.html">
                                    $gettext("telemetry")
                                </a>
                            </td>
                            <td></td>
                        </tr>
                    #end if
                </table>

                #if $Extras.Footer.about != ""
                    <p>$Extras.Footer.about</p>
                #end if

            </div>
            ## Grid column

            <hr class="clearfix w-100 d-md-none pb-3">

            ## Grid column
            <div class="col-md-4 mb-md-0 mb-3">

                #if $Extras.Footer.box_title != ""
                    <h5 class="text-uppercase">$Extras.Footer.box_title</h5>
                #end if

                #if $Extras.Footer.box_content != ""
                    <p>$Extras.Footer.box_content</p>
                #end if
            </div>
            ## Grid column

            ## Grid column
            <div class="col-md-3 mb-md-0 mb-3">

                ## Links
                <h5 class="text-uppercase">$gettext("links")</h5>

                <ul class="list-unstyled">
                    #if $Extras.Footer.link1_url != ""
                        <li>
                            <a href="$Extras.Footer.link1_url" target="_blank" rel="noopener">$Extras.Footer.link1_label</a>
                        </li>
                    #end if
                    #if $Extras.Footer.link2_url != ""
                        <li>
                            <a href="$Extras.Footer.link2_url" target="_blank" rel="noopener">$Extras.Footer.link2_label</a>
                        </li>
                    #end if
                    #if $Extras.Footer.link3_url != ""
                        <li>
                            <a href="$Extras.Footer.link3_url" target="_blank" rel="noopener">$Extras.Footer.link3_label</a>
                        </li>
                    #end if
                    #if $Extras.Footer.link4_url != ""
                        <li>
                            <a href="$Extras.Footer.link4_url" target="_blank" rel="noopener">$Extras.Footer.link4_label</a>
                        </li>
                    #end if
                </ul>

            </div>
            ## Grid column

        </div>
        ## Grid row

    </div>
    ## Footer Links

    ## Copyright
    <div class="footer-copyright text-center py-3">
        &copy; $current.dateTime.format("%Y") $Extras.Footer.name

        #if $Extras.Footer.support_weewx == "yes"
            <span class="mx-2">&bull;</span>
            Powered by <a href="https://weewx.com" target="_blank">weewx</a>
            #if $Extras.Footer.show_version == "yes"
                v$station.version
            #end if
        #end if

        #if $Extras.Footer.support_skin == "yes"
            <span class="mx-2">&bull;</span>
            <a href="https://github.com/seehase/neowx-material" target="_blank">Fork of NeoWX Material</a> skin

            #if $Extras.Footer.show_version == "yes"
                v$Extras.version
            #end if
        #end if

        #if $Extras.Footer.imprint_url != ""
            <span class="mx-2">&bull;</span>
            <a href="$Extras.Footer.imprint_url" target="_blank">$Extras.Footer.imprint_label</a>
        #end if
        #if $Extras.Footer.privacy_url != ""
            <span class="mx-2">&bull;</span>
            <a href="$Extras.Footer.privacy_url" target="_blank">$Extras.Footer.privacy_label</a>
        #end if
        #if "forecast" in $Extras.Appearance.values_order:
            <div class="text-center px-2">
                Weather forecast data provided by&nbsp;<a href="https://open-meteo.com/">Open-Meteo.com</a> under (<a href="https://creativecommons.org/licenses/by/4.0/">CC BY 4.0</a>) license. Data may be modified (e.g., unit conversion, aggregation).
            </div>
        #end if

        ## Update notification
        #if $update_check and $update_check.available
            <div class="alert alert-info alert-dismissible fade show mx-3 my-2" role="alert">
                <i class="fas fa-info-circle"></i>
                $gettext("update_available")
                <strong>v$update_check.latest_version</strong>
                ($gettext("current"): v$update_check.current_version)
                <a href="$update_check.github_url" target="_blank" class="alert-link">$gettext("view_on_github")</a>
            </div>
        #end if
    </div>
    ## Copyright

</footer>
