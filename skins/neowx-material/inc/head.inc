#encoding UTF-8
## +-------------------------------------------------------------------------+
## |    head.inc                      HTML head section for all templates    |
## +-------------------------------------------------------------------------+
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
<meta http-equiv="x-ua-compatible" content="ie=edge">
<!-- NeoWX Material skin v$Extras.version -->
<meta name="generator" content="neowx-material $Extras.version">
<meta name="neowx-version" content="$Extras.version">

#if not $varExists('base_path')
    #set $base_path = ''
#end if
<script>
    window.NX_BASE_PATH = '$base_path';
    window.NX_LIVE_DATA = (function () {
        try {
            return fetch((window.NX_BASE_PATH || '') + 'live.json?_=' + Date.now(), { cache: 'no-store' })
                .then(function (r) {
                    if (!r.ok) { throw new Error('live.json HTTP ' + r.status); }
                    return r.json();
                });
        } catch (e) { return Promise.reject(e); }
    })();
    // No-op handler so an unused/failed pre-warm (e.g. current page with uptime
    // off) doesn't surface as an unhandled rejection. neowx-live.js attaches its
    // own then/catch to the same promise independently.
    window.NX_LIVE_DATA.catch(function () {});
</script>

<meta name="description" content="$Extras.Header.description">
<meta name="keywords" content="$Extras.Header.keywords">

## Web App
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-title" content="$gettext("weather")">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black">
<meta name="application-name" content="$gettext("weather") | $station.location">

## Pinned Sites
<meta name="application-name" content="$gettext("weather") | $station.location">
<meta name="msapplication-tooltip" content="$station.location">
<meta name="msapplication-starturl" content="/">

## Tap highlighting
<meta name="msapplication-tap-highlight" content="no">

## Auto refresh
#if $Extras.Header.auto_refresh_enable == "yes"
    <meta http-equiv="refresh" content="$Extras.Header.auto_refresh_seconds">
#end if

## Google Tag Manager or Google Analytics but not both at the same time

#if $Extras.Header.google_tagmanager_enable == "yes"
    <!-- Google Tag Manager -->
    <script>(function(w,d,s,l,i){w[l]=w[l]||[];w[l].push({'gtm.start':
        new Date().getTime(),event:'gtm.js'});var f=d.getElementsByTagName(s)[0],
        j=d.createElement(s),dl=l!='dataLayer'?'&l='+l:'';j.async=true;j.src=
        'https://www.googletagmanager.com/gtm.js?id='+i+dl;f.parentNode.insertBefore(j,f);
    })(window,document,'script','dataLayer','$Extras.Header.google_tagmanager_id');
</script>
<!-- End Google Tag Manager -->
#else if $Extras.Header.google_analytics_enable == "yes"
<!-- Google analytics tag (gtag.js) -->
<script async src="https://www.googletagmanager.com/gtag/js?id=$Extras.Header.google_analytics_id"></script>
<script>
    window.dataLayer = window.dataLayer || [];
    function gtag(){dataLayer.push(arguments);}
    gtag('js', new Date());

    gtag('config', '$Extras.Header.google_analytics_id');
</script>
<!-- End Google analytics tag -->
#end if

## Favicons / mobile icons
<link href="${base_path}img/icon-16.ico" rel="shortcut icon" type="image/x-icon">
<link href="${base_path}img/icon-16.png" rel="icon" type="image/png" sizes="16x16">
<link href="${base_path}img/icon-32.png" rel="icon" type="image/png" sizes="32x32">
<link href="${base_path}img/icon-48.png" rel="icon" type="image/png" sizes="48x48">

<link href="${base_path}img/icon-180.png" rel="apple-touch-icon">
<link href="${base_path}img/icon-76.png" rel="apple-touch-icon" sizes="76x76">
<link href="${base_path}img/icon-120.png" rel="apple-touch-icon" sizes="120x120">
<link href="${base_path}img/icon-152.png" rel="apple-touch-icon" sizes="152x152">

<link href="${base_path}img/icon-192.png" rel="icon" sizes="192x192">
<link href="${base_path}img/icon-128.png" rel="icon" sizes="128x128">

<link href="${base_path}manifest.json" rel="manifest">

## Bootstrap core CSS
<link rel="stylesheet" href="${base_path}css/bootstrap.min.css">

## MDB + skin style
<link rel="stylesheet" href="${base_path}css/style.min.css?v=$Extras.version">

## Weather Icons
<link rel="stylesheet" href="${base_path}weather-icons/css/weather-icons.min.css">
<link rel="stylesheet" href="${base_path}weather-icons/css/weather-icons-wind.min.css">

## Material Design Icons
<link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/@mdi/font@7.4.47/css/materialdesignicons.min.css">

## ApexChart stylesheet
<link rel="stylesheet" href="${base_path}js/vendor/apexcharts/apexcharts.css">

## Panel state restore.  Must be INLINE and must run before the body parses:
## the panels are painted from the server-rendered default, so anything that
## waits for DOMContentLoaded shows the wrong state first and corrects it
## visibly.  On a page that auto-refreshes every few minutes that flash would
## be relentless.
##
## The override needs no knowledge of what the default IS.  Each stored state
## maps to a rule that is either corrective or a no-op, so we can emit both
## directions blind.
#if $varExists('Extras.Appearance.rememberPanelState') and str($Extras.Appearance.rememberPanelState).strip().lower() not in ('false', 'no', '0')
<script>
(function () {
    // Tells js/app.js the setting is on, so its own DOMContentLoaded handler
    // knows whether to correct the DOM and bind the writers below - it has no
    // template access of its own to check Extras.Appearance directly. Set
    // before the try so it still lands even if storage is unreadable or the
    // stored blob is malformed and one of the early returns below fires.
    window.NWM_REMEMBER_PANELS = true;
    var stored;
    try { stored = JSON.parse(sessionStorage.getItem('nwm-panels') || '{}'); }
    catch (e) { return; }
    if (!stored || typeof stored !== 'object' || Array.isArray(stored)) { return; }
    var css = '';
    for (var slug in stored) {
        if (!Object.prototype.hasOwnProperty.call(stored, slug)) { continue; }
        if (!/^[A-Za-z0-9_-]+$/.test(slug)) { continue; }
        var p = '.nwm-expansion-panel[data-section="' + slug + '"]';
        // Static panels (collapsed = none) have no toggle, so a stale stored
        // false would hide one with no way to reopen it.  The header is the
        // body's immediate previous sibling, so + reaches exactly the right
        // element without relying on the CSS :has selector.
        var body = p + ' > .nwm-panel-header:not(.nwm-panel-static) + .collapse';
        // The chevron rotates off [aria-expanded] on the header, which this
        // override does not change - so without a matching transform the
        // panel paints with its chevron pointing the wrong way.  This
        // selector is (0,5,0) against the base rule's (0,3,0); a shorter one
        // ties and is then decided by source order.
        var icon = p + ' > .nwm-panel-header[aria-expanded] .nwm-panel-icon';
        css += stored[slug]
            ? body + '{display:block}' + icon + '{transform:rotate(180deg)}'
            : body + '.show{display:none}' + icon + '{transform:none}';
    }
    if (css) {
        var st = document.createElement('style');
        st.id = 'nwm-panel-restore';
        st.textContent = css;
        document.head.appendChild(st);
    }
})();
</script>
#end if

## Appearance settings

#if $Extras.Appearance.lo_value_color != ""
    <style>
        .lo-text {
            color: $Extras.Appearance.lo_value_color !important;
        }
    </style>
#end if

#if $Extras.Appearance.hi_value_color != ""
    <style>
        .hi-text {
            color: $Extras.Appearance.hi_value_color !important;
        }
    </style>
#end if

#set $_panel_color = str($Extras.Appearance.get('panelColor', '')).strip().strip('"').strip("'")
#set $_panel_color_dark = str($Extras.Appearance.get('panelColorDark', '')).strip().strip('"').strip("'")
#set $_panel_title_color = str($Extras.Appearance.get('panelTitleColor', '')).strip().strip('"').strip("'")
#set $_panel_title_color_dark = str($Extras.Appearance.get('panelTitleColorDark', '')).strip().strip('"').strip("'")

#if $_panel_color != '' or $_panel_color_dark != '' or $_panel_title_color != '' or $_panel_title_color_dark != ''
<style>
    #if $_panel_color != ''
    /* Light mode panel background */
    .light-theme .nwm-panel-header,
    .auto-theme .nwm-panel-header {
        background-color: $_panel_color !important;
    }
    #end if
    #if $_panel_color_dark != ''
    /* Dark mode panel background */
    .dark-theme .nwm-panel-header {
        background-color: $_panel_color_dark !important;
    }
    @media (prefers-color-scheme: dark) {
        .auto-theme .nwm-panel-header {
            background-color: $_panel_color_dark !important;
        }
    }
    #end if
    #if $_panel_title_color != ''
    /* Light mode panel title */
    .light-theme .nwm-panel-title,
    .auto-theme .nwm-panel-title {
        color: $_panel_title_color !important;
    }
    #end if
    #if $_panel_title_color_dark != ''
    /* Dark mode panel title */
    .dark-theme .nwm-panel-title {
        color: $_panel_title_color_dark !important;
    }
    @media (prefers-color-scheme: dark) {
        .auto-theme .nwm-panel-title {
            color: $_panel_title_color_dark !important;
        }
    }
    #end if
</style>
#end if
## Startup images (splash screens) for ios web app

<link
rel="apple-touch-startup-image"
media="screen and (device-width: 320px) and (device-height: 568px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
href="${base_path}img/splash/splash_1136x640.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 375px) and (device-height: 812px) and (-webkit-device-pixel-ratio: 3) and (orientation: landscape)"
href="${base_path}img/splash/splash_2436x1125.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 414px) and (device-height: 896px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
href="${base_path}img/splash/splash_1792x828.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 414px) and (device-height: 896px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
href="${base_path}img/splash/splash_828x1792.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 375px) and (device-height: 667px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
href="${base_path}img/splash/splash_1334x750.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 414px) and (device-height: 896px) and (-webkit-device-pixel-ratio: 3) and (orientation: portrait)"
href="${base_path}img/splash/splash_1242x2688.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 414px) and (device-height: 736px) and (-webkit-device-pixel-ratio: 3) and (orientation: landscape)"
href="${base_path}img/splash/splash_2208x1242.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 375px) and (device-height: 812px) and (-webkit-device-pixel-ratio: 3) and (orientation: portrait)"
href="${base_path}img/splash/splash_1125x2436.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 414px) and (device-height: 736px) and (-webkit-device-pixel-ratio: 3) and (orientation: portrait)"
href="${base_path}img/splash/splash_1242x2208.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 1024px) and (device-height: 1366px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
href="${base_path}img/splash/splash_2732x2048.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 414px) and (device-height: 896px) and (-webkit-device-pixel-ratio: 3) and (orientation: landscape)"
href="${base_path}img/splash/splash_2688x1242.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 834px) and (device-height: 1112px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
href="${base_path}img/splash/splash_2224x1668.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 375px) and (device-height: 667px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
href="${base_path}img/splash/splash_750x1334.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 1024px) and (device-height: 1366px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
href="${base_path}img/splash/splash_2048x2732.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 834px) and (device-height: 1194px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
href="${base_path}img/splash/splash_2388x1668.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 834px) and (device-height: 1112px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
href="${base_path}img/splash/splash_1668x2224.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 320px) and (device-height: 568px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
href="${base_path}img/splash/splash_640x1136.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 834px) and (device-height: 1194px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
href="${base_path}img/splash/splash_1668x2388.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 768px) and (device-height: 1024px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
href="${base_path}img/splash/splash_2048x1536.png"
/>
<link
rel="apple-touch-startup-image"
media="screen and (device-width: 768px) and (device-height: 1024px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
href="${base_path}img/splash/splash_1536x2048.png"
/>

<style>
    /* Applies hand cursor and smooth transitions to both Value Cards and Graph Cards */
    .card-value,
    .card-chart {
        cursor: pointer;
        transition: all 0.2s ease-in-out;
    }

    #set $enable_hover = $Extras.Appearance.get('enable_hover_effect', 'true')
    #if $enable_hover == 'true' or $enable_hover == True
        /* Hover effect for Value Cards and Graph Cards (Lift & Shadow) */
        main .card:not(.card-forecast):hover,
        .temprow .card:not(.card-forecast):hover,
        .graphrow .card:not(.card-forecast):hover {
            box-shadow: 0 8px 15px rgba(0,0,0,0.3) !important;
            transform: translateY(-3px);
        }
    #end if

    /* Prevents "Touch to Search" popups and blue text selection on mobile/desktop */
    main .card,
    main .card h3,
    main .card .card-title,
    .temprow .card,
    .graphrow .card,
    .graphrow .card h3,
    .graphrow .card .card-title,
    .seasons-vector-img {
        -webkit-user-select: none; /* Safari */
        -moz-user-select: none;    /* Firefox */
        -ms-user-select: none;     /* IE/Edge */
        user-select: none;         /* Standard */
        -webkit-tap-highlight-color: transparent; /* Removes the gray flash on mobile tap */
    }

    .nwm-panel-static {
        cursor: default;
    }

    /* A static panel (collapsed = none) has no toggle, so it must not
       hover-highlight as if it were clickable. css/style.css applies
       .nwm-panel-header:hover plus two dark-mode !important variants; the
       rules below are more specific and come later, so they win.

       "Cancel the hover" means repaint whatever the header shows when it is
       NOT hovered, which is not always transparent. In light mode the
       configured panelColor rule is !important and this one is not, so the
       configured colour already wins and transparent only applies when
       nothing is configured. In dark mode both carry !important, so this
       rule wins on specificity and has to repaint panelColorDark itself -
       otherwise hovering a static header erases the configured fill. */
    #set $_static_dark = $_panel_color_dark if $_panel_color_dark != '' else 'transparent'
    .nwm-panel-header.nwm-panel-static:hover {
        background-color: transparent;
    }
    .dark-theme .nwm-panel-header.nwm-panel-static:hover {
        background-color: $_static_dark !important;
    }
    @media (prefers-color-scheme: dark) {
        .auto-theme .nwm-panel-header.nwm-panel-static:hover {
            background-color: $_static_dark !important;
        }
    }
</style>

## +-------------------------------------------------------------------------+
## | Card heading alignment                 Appearance/items_title_align      |
## +-------------------------------------------------------------------------+
## Only the heading moves. The rule targets it directly rather than the
## .card-body's text-center, so a value card's reading, a chart and a forecast
## row stay centred whatever the heading does.
##
## "center" is what the skin has always done, so a site that sets neither this
## nor a per-card override emits no CSS here at all.
##
## How much room to leave for the card's expand button or toolbar is a runtime
## question - it depends which control that card ended up with, and the chart
## ones are built in JavaScript - so js/app.js measures it and sets
## --nwm-item-title-reserve on the card. The fallback of 0px is what a card with no
## control gets, which makes center-adjusted behave as center there.
##
## It sets --nwm-item-title-inset the same way: how far a left-aligned heading has
## to be pushed off the card's edge, on top of whatever the card body's own
## padding already contributes. That differs by card - the [[Embedded]] ones
## have none at all - so it cannot be a constant here.
#set $_ALIGNS = ('center', 'left', 'center-adjusted')

#set $_items_title_align = str($Extras.Appearance.get('items_title_align', 'center')).strip().lower()
#if $_items_title_align not in $_ALIGNS
    #set $_items_title_align = 'center'
#end if

## Per-card overrides, keyed by the name the card carries as data-name:
##   - [[[ItemsTitleAlign]]] maps names to values, for value cards and built-in
##     charts, which have no section of their own
##   - an items_title_align key inside an [[Embedded]] card's or a
##     [[[customChart...]]] section, which is where those two are configured
##     anyway
##
## Applied in that order, so a card named in both places ends up following its
## own section - the more specific place to have said it. tests/ has a check
## that pins this; do not reorder the loops without moving it.
#set $_title_overrides = {}
#if $varExists('Extras.Appearance.ItemsTitleAlign')
    #for $_key in $Extras.Appearance.ItemsTitleAlign.keys()
        #set $_a = str($Extras.Appearance.ItemsTitleAlign[$_key]).strip().lower()
        #if $_a in $_ALIGNS
            #silent $_title_overrides.update({str($_key): $_a})
        #end if
    #end for
#end if
#for $_key in $Extras.Appearance.keys()
    #set $_sub = $Extras.Appearance[$_key]
    #if hasattr($_sub, 'get')
        #set $_a = str($_sub.get('items_title_align', '')).strip().lower()
        #if $_a in $_ALIGNS
            #silent $_title_overrides.update({str($_key): $_a})
        #end if
    #end if
#end for
#if $varExists('Extras.Embedded')
    #for $_key in $Extras.Embedded.keys()
        #set $_sub = $Extras.Embedded[$_key]
        #if hasattr($_sub, 'get')
            #set $_a = str($_sub.get('items_title_align', '')).strip().lower()
            #if $_a in $_ALIGNS
                #silent $_title_overrides.update({str($_key): $_a})
            #end if
        #end if
    #end for
#end if

## A [[[sections]]] panel can set items_title_align for everything it lists,
## which is the setting most people want: a whole panel of charts left-aligned
## without naming each one. It reaches the cards the panel holds, never the
## panel's own title.
##
## Qualified by the kind of card the panel holds, because a value card and a
## chart card for the same observation carry the SAME data-name - "barometer"
## is usually in both a cards list and a charts list, and a rule keyed on the
## name alone would move both. The panel's content setting says which kind it
## holds, and the templates give each kind its own class.
##
## Every selector these produce has the same specificity as a per-card one, so
## which wins is decided by the order they are emitted in below: section rules
## first, per-card rules after.
#set $_CONTENT_CARD_KINDS = {
    'chart': ('.card-chart', '.card-embedded'),
    'telemetry_chart': ('.card-chart', '.card-embedded'),
    'telemetry': ('.card-value', '.card-embedded'),
}
#set $_section_overrides = []
#if $varExists('Extras.Appearance.sections')
    #for $_sid in $Extras.Appearance.sections.keys()
        #set $_section = $Extras.Appearance.sections[$_sid]
        #if not hasattr($_section, 'get')
            #continue
        #end if
        #set $_a = str($_section.get('items_title_align', '')).strip().lower()
        #if $_a not in $_ALIGNS
            #continue
        #end if
        ## A default (or unrecognised) content is the cards panel, which holds
        ## value cards and can hold the forecast card too.
        #set $_content = str($_section.get('content', '')).strip().lower()
        #set $_kinds = $_CONTENT_CARD_KINDS.get($_content, ('.card-value', '.card-forecast', '.card-embedded'))
        ## configobj gives a list for "a, b, c" and a string for a single name.
        #set $_items = $_section.get('items', [])
        #if isinstance($_items, str)
            #set $_items = $_items.split(',')
        #end if
        ## Delegate to panelorder.py's parser rather than re-deriving the slug
        ## sanitize rule here: it also handles the "-2" collision suffix and
        ## the "section-<n>" fallback for a name with no usable characters.
        #set $_sslug = $panelSectionSlug($_sid)
        #for $_item in $_items
            #set $_item = str($_item).strip()
            #if $_item != '' and all(c.isalnum() or c in '_.:-' for c in $_item)
                #silent $_section_overrides.append(($_kinds, $_item, $_a, $_sslug))
            #end if
        #end for
    #end for
#end if

#if $_items_title_align != 'center' or len($_title_overrides) > 0 or len($_section_overrides) > 0
<style>
    ## The panel-wide rules first: same specificity as the per-card ones below,
    ## so anything named individually there overrides what its panel asked for.
    #for $_kinds, $_item, $_align, $_sslug in $_section_overrides
        ## :where() contributes zero specificity, which is load bearing here.
        ## The comment above _CONTENT_CARD_KINDS records that section rules and
        ## per-card rules must stay at EQUAL specificity, with emission order
        ## deciding which wins. A bare second attribute selector would raise
        ## these above the per-card rules below and silently invert that.
        #set $_sel = ', '.join([$_k + ':where([data-section="' + $_sslug + '"])[data-name="' + $_item + '"] > .card-body > .h5-responsive' for $_k in $_kinds])
    $_sel {
        #if $_align == 'left'
        text-align: left;
        padding-left: var(--nwm-item-title-inset, 0px);
        padding-right: var(--nwm-item-title-reserve, 0px);
        #else if $_align == 'center-adjusted'
        text-align: center;
        padding-left: 0;
        padding-right: var(--nwm-item-title-reserve, 0px);
        #else
        text-align: center;
        padding-left: 0;
        padding-right: 0;
        #end if
    }
        #for $_k in $_kinds
            ## Same :where() reasoning as $_sel above: js/app.js reads this
            ## custom property via getComputedStyle(card), and the per-card
            ## counterpart below (.card[data-name="X"]) is also (0,2,0) - a
            ## bare second attribute here would raise this rule to (0,3,0)
            ## and make it always win regardless of emission order.
    ${_k}:where([data-section="${_sslug}"])[data-name="${_item}"] {
        --nwm-item-title-align: $_align;
    }
        #end for
    #end for
    #for $_name, $_align in [(None, $_items_title_align)] + sorted($_title_overrides.items())
        #if $_name is None
            #set $_card = '.card'
        #else
            ## Anything that could close the attribute selector early would take
            ## every rule after it in this block down with it, so a name that is
            ## not plain gets no rule rather than a broken one.
            #if not all(c.isalnum() or c in '_.:-' for c in $_name)
                #continue
            #end if
            #set $_card = '.card[data-name="' + $_name + '"]'
        #end if
    $_card {
        /* Read by js/app.js when a pop-up inherits the card's alignment. */
        --nwm-item-title-align: $_align;
    }
    $_card > .card-body > .h5-responsive {
        #if $_align == 'left'
        text-align: left;
        padding-left: var(--nwm-item-title-inset, 0px);
        padding-right: var(--nwm-item-title-reserve, 0px);
        #else if $_align == 'center-adjusted'
        text-align: center;
        /* Explicit, not omitted: an override under a "left" global would
           otherwise inherit that rule's padding-left and centre the heading in
           a box padded on one side only. */
        padding-left: 0;
        padding-right: var(--nwm-item-title-reserve, 0px);
        #else
        text-align: center;
        padding-left: 0;
        padding-right: 0;
        #end if
    }
    #end for
</style>
<script type="text/javascript">
    // js/app.js measures the room each card's control needs, which is work
    // worth doing only because the rules above are here to use it.
    window.NEOWX_ITEM_TITLE_ALIGN_SET = true;
</script>
#end if
