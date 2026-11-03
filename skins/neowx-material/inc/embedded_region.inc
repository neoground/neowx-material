## Shared embedded-content region.
##
## Cheetah's #include does NOT share #defs across files, so the defs that
## render an embedded item live here, beside the code that calls them.  What
## this file needs from its caller it reads itself: $page and $subpage are
## #attr values, and those DO resolve inside an include (footer.inc has read
## $page this way for years).  So every caller is a bare one-line include with
## no #set global handoff to forget.
##
## Out of scope by design: almanac, history and archive render no panel
## regions.  See the [[[pages]]] comments in skin.conf.

#def notFoundCard($name)
    <div class="col-12 col-md-6 col-xl-4 mb-4">
        <div class="card">
            <div class="card-body text-center">
                Section definition not found for "$name"
            </div>
        </div>
    </div>
#end def

## +-------------------------------------------------------------------------+
## | Expand button for [[Embedded]] cards. Styled to match the ApexCharts    |
## | chart toolbars. The data-nwm-embed-popup attribute is the contract: a   |
## | delegated click handler in js/app.js keys on it, so renaming it here    |
## | silently stops the button working.                                      |
## |                                                                         |
## | The toolbar must stay a direct child of .card, next to .card-body -     |
## | it anchors against Bootstrap's .card { position: relative }, and the    |
## | handler walks up to .card to find the heading and the media.            |
## +-------------------------------------------------------------------------+

#def embedToolbar()
    ## Default-on, so normalise rather than exact-match - see the same handling
    ## of enableChartPopup in js.inc.
    #if str($Extras.Appearance.get('enableEmbeddedPopup', 'true')).strip().lower() not in ('false', 'no', '0')
        <div class="nwm-embed-toolbar">
            <button type="button" class="nwm-embed-toolbar-icon" data-nwm-embed-popup
                    title="$gettext('expand')" aria-label="$gettext('expand')">
                <svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M15 3h6v6"/><path d="M9 21H3v-6"/><path d="M21 3l-7 7"/><path d="M3 21l7-7"/></svg>
            </button>
        </div>
    #end if
#end def

#if not $varExists('base_path')
    #set $base_path = ''
#end if

#def iframeCard($name, $section = "")
    #set iframe_url = $getVar('Extras.Embedded.' + name + '.url', "")
    #if $iframe_url != "" and not ($iframe_url.startswith('http://') or $iframe_url.startswith('https://') or $iframe_url.startswith('/') or $iframe_url.startswith('//'))
        #set iframe_url = $base_path + $iframe_url
    #end if
    #set iframe_title = $getVar('Extras.Embedded.' + name + '.title', "")
    #set iframe_aspect = $getVar('Extras.Embedded.' + name + '.aspect_ratio', "4/3")
    #if iframe_url != ""
        <div class="col-12 col-md-6 col-xl-4 mb-4">
            <div class="card card-embedded overflow-hidden" data-name="$name" data-section="$section">
                $embedToolbar()
                <div class="card-body text-center d-flex flex-column px-0 ${'pb-0' if iframe_title != '' else 'py-0'} overflow-hidden">
                    #if iframe_title != ""
                        <h5 class="h5-responsive $Extras.color-text">
                            $iframe_title
                        </h5>
                    #end if
                    <iframe src="$iframe_url" frameborder="0" class="flex-grow-1 w-100" style="aspect-ratio: ${iframe_aspect};"></iframe>
                </div>
            </div>
        </div>
    #else
        $notFoundCard($name)
    #end if
#end def

#def imageCard($name, $section = "")
    #set image_url = $getVar('Extras.Embedded.' + name + '.url', "")
    #if $image_url != "" and not ($image_url.startswith('http://') or $image_url.startswith('https://') or $image_url.startswith('/') or $image_url.startswith('data:'))
        #set image_url = $base_path + $image_url
    #end if
    ## by adding a timestamp parameter we prevent browser caching of the image
    #set $sep = '&' if '?' in $image_url else '?'
    #set $timestamp = int($time.time() / 60)
    #set image_title = $getVar('Extras.Embedded.' + name + '.title', "")
    #set image_link = $getVar('Extras.Embedded.' + name + '.link', "")
    #if $image_link != "" and not ($image_link.startswith('http://') or $image_link.startswith('https://') or $image_link.startswith('/') or $image_link.startswith('#'))
        #set image_link = $base_path + $image_link
    #end if
    #set image_new_tab = $getVar('Extras.Embedded.' + name + '.new_tab', "no")
    #set image_padding = $getVar('Extras.Embedded.' + name + '.padding', "no")
    #set image_aspect = $getVar('Extras.Embedded.' + name + '.aspect_ratio', "4/3")
    #set image_alt = $getVar('Extras.Embedded.' + name + '.alt', "Image")

    #if image_padding != "yes"
        #set padding_x = "px-0"
        #if image_title != ""
            #set padding_y = "pb-0"
        #else
            #set padding_y = "py-0"
        #end if
    #else
        #set padding_x = "px-3"
        #if image_title != ""
            #set padding_y = "pb-3"
        #else
            #set padding_y = "py-3"
        #end if
    #end if

    #if image_url != ""
        <div class="col-12 col-md-6 col-xl-4 mb-4">
            <div class="card card-embedded overflow-hidden" data-name="$name" data-section="$section">
                $embedToolbar()
                <div class="card-body text-center d-flex flex-column ${padding_x} ${padding_y} overflow-hidden">
                    #if image_title != ""
                        <h5 class="h5-responsive $Extras.color-text">
                            $image_title
                        </h5>
                    #end if
                    #if image_link != ""
                        #if image_new_tab.lower() == "yes"
                            <a href="$image_link" target="_blank" rel="noopener noreferrer" class="flex-grow-1 d-flex flex-column w-100">
                            #else
                                <a href="$image_link" class="flex-grow-1 d-flex flex-column w-100">
                                #end if
                            #end if

                            <img src="${image_url}${sep}ts=${timestamp}" alt="${image_alt}" class="flex-grow-1 w-100" style="aspect-ratio: ${image_aspect};" />
                            #if image_link != ""
                            </a>
                        #end if
                    </div>
                </div>
            </div>
        #else
            $notFoundCard($name)
        #end if
    #end def

## +-------------------------------------------------------------------------+
## | Helper: render items inside a panel (or directly) for embedded content   |
## +-------------------------------------------------------------------------+

#def renderEmbItem($x, $section = "")
    #if $x.lower().startswith("iframe")
        $iframeCard($x, $section)
    #else if $x.lower().startswith("image")
        $imageCard($x, $section)
    #else if $varExists('Extras.Embedded.' + $x)
        #if $getVar('Extras.Embedded.' + $x + '.link', '') != '' or $getVar('Extras.Embedded.' + $x + '.alt', '') != '' or $getVar('Extras.Embedded.' + $x + '.padding', '') != '' or str($getVar('Extras.Embedded.' + $x + '.url', '')).lower().endswith(('.png', '.jpg', '.jpeg', '.gif', '.svg', '.webp'))
            $imageCard($x, $section)
        #else
            $iframeCard($x, $section)
        #end if
    #else
        $notFoundCard($x)
    #end if
#end def

#if $getVar('emb_item', None)
    $renderEmbItem($emb_item, $getVar('emb_section', ''))
#end if
