#encoding UTF-8
## +-------------------------------------------------------------------------+
## |    head.inc                      HTML head section for all templates    |
## +-------------------------------------------------------------------------+
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, shrink-to-fit=no">
<meta http-equiv="x-ua-compatible" content="ie=edge">

<meta name="description" content="$Extras.Header.description">
<meta name="keywords" content="$Extras.Header.keywords">

## Web App
<meta name="mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-title" content="$Extras.Translations[$Extras.language].weather">
<meta name="apple-mobile-web-app-capable" content="yes">
<meta name="apple-mobile-web-app-status-bar-style" content="black">
<meta name="application-name" content="$Extras.Translations[$Extras.language].weather | $station.location">

## Pinned Sites
<meta name="application-name" content="$Extras.Translations[$Extras.language].weather | $station.location">
<meta name="msapplication-tooltip" content="$station.location">
<meta name="msapplication-starturl" content="/">

## Tap highlighting
<meta name="msapplication-tap-highlight" content="no">

## Favicons / mobile icons
<link href="img/icon-16.ico" rel="shortcut icon" type="image/x-icon">
<link href="img/icon-16.png" rel="icon" type="image/png" sizes="16x16">
<link href="img/icon-32.png" rel="icon" type="image/png" sizes="32x32">
<link href="img/icon-48.png" rel="icon" type="image/png" sizes="48x48">

<link href="img/icon-180.png" rel="apple-touch-icon">
<link href="img/icon-76.png" rel="apple-touch-icon" sizes="76x76">
<link href="img/icon-120.png" rel="apple-touch-icon" sizes="120x120">
<link href="img/icon-152.png" rel="apple-touch-icon" sizes="152x152">

<link href="img/icon-192.png" rel="icon" sizes="192x192">
<link href="img/icon-128.png" rel="icon" sizes="128x128">

<link href="manifest.json" rel="manifest">

## Bootstrap core CSS
<link rel="stylesheet" href="css/bootstrap.min.css">

## MDB + skin style
<link rel="stylesheet" href="css/style.min.css">

## Weather Icons
<link rel="stylesheet" href="weather-icons/css/weather-icons.min.css">
<link rel="stylesheet" href="weather-icons/css/weather-icons-wind.min.css">

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

## Startup images (splash screens) for ios web app

<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 320px) and (device-height: 568px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
        href="img/splash/splash_1136x640.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 375px) and (device-height: 812px) and (-webkit-device-pixel-ratio: 3) and (orientation: landscape)"
        href="img/splash/splash_2436x1125.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 414px) and (device-height: 896px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
        href="img/splash/splash_1792x828.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 414px) and (device-height: 896px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
        href="img/splash/splash_828x1792.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 375px) and (device-height: 667px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
        href="img/splash/splash_1334x750.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 414px) and (device-height: 896px) and (-webkit-device-pixel-ratio: 3) and (orientation: portrait)"
        href="img/splash/splash_1242x2688.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 414px) and (device-height: 736px) and (-webkit-device-pixel-ratio: 3) and (orientation: landscape)"
        href="img/splash/splash_2208x1242.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 375px) and (device-height: 812px) and (-webkit-device-pixel-ratio: 3) and (orientation: portrait)"
        href="img/splash/splash_1125x2436.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 414px) and (device-height: 736px) and (-webkit-device-pixel-ratio: 3) and (orientation: portrait)"
        href="img/splash/splash_1242x2208.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 1024px) and (device-height: 1366px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
        href="img/splash/splash_2732x2048.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 414px) and (device-height: 896px) and (-webkit-device-pixel-ratio: 3) and (orientation: landscape)"
        href="img/splash/splash_2688x1242.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 834px) and (device-height: 1112px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
        href="img/splash/splash_2224x1668.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 375px) and (device-height: 667px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
        href="img/splash/splash_750x1334.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 1024px) and (device-height: 1366px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
        href="img/splash/splash_2048x2732.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 834px) and (device-height: 1194px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
        href="img/splash/splash_2388x1668.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 834px) and (device-height: 1112px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
        href="img/splash/splash_1668x2224.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 320px) and (device-height: 568px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
        href="img/splash/splash_640x1136.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 834px) and (device-height: 1194px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
        href="img/splash/splash_1668x2388.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 768px) and (device-height: 1024px) and (-webkit-device-pixel-ratio: 2) and (orientation: landscape)"
        href="img/splash/splash_2048x1536.png"
/>
<link
        rel="apple-touch-startup-image"
        media="screen and (device-width: 768px) and (device-height: 1024px) and (-webkit-device-pixel-ratio: 2) and (orientation: portrait)"
        href="img/splash/splash_1536x2048.png"
/>
