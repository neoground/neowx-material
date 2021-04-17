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
            <div class="col-md-6 mt-md-0 mt-3">

                ## Content
                <h5 class="text-uppercase">$Extras.Translations[$Extras.language].about</h5>

                <table class="d-inline">
                    <tr>
                        <td>$Extras.Translations[$Extras.language].hardware:</td>
                        <td>$station.hardware</td>
                    </tr>
                    <tr>
                        <td>$Extras.Translations[$Extras.language].altitude:</td>
                        <td>$station.altitude</td>
                    </tr>
                    #if $Extras.Footer.show_telemetry == "yes"
                    <tr>
                        <td>
                            <a href="telemetry.html">
                                $Extras.Translations[$Extras.language].telemetry
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
            <div class="col-md-3 mb-md-0 mb-3">

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
                <h5 class="text-uppercase">$Extras.Translations[$Extras.language].links</h5>

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
          <a href="https://neoground.com/projects/neowx-material" target="_blank">NeoWX Material</a> skin
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
    </div>
    ## Copyright

</footer>
