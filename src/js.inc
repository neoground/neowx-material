#errorCatcher Echo
#encoding UTF-8

## jQuery
<script type="text/javascript" src="js/jquery.min.js"></script>
## Bootstrap tooltips / JS
<script type="text/javascript" src="js/popper.min.js"></script>
<script type="text/javascript" src="js/bootstrap.min.js"></script>
## MDB core JavaScript
<script type="text/javascript" src="js/mdb.min.js"></script>
## Vendor JS
#if $Extras.language == "en"
<script src="js/vendor/moment.min.js"></script>
#else
<script src="js/vendor/moment-with-locales.min.js"></script>
<script type="text/javascript">moment.locale('$Extras.language')</script>
#end if
<script src="js/vendor/apexcharts/apexcharts.min.js"></script>
## NeoWX Material App
<script type="text/javascript" src="js/app.js"></script>