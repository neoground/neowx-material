$(document).ready(function() {
    // Archive sub menu handling especially for mobile
    $('.dropdown-submenu > a').on("click", function(e) {
        var submenu = $(this);
        $('.dropdown-submenu .dropdown-menu').removeClass('show');
        submenu.next('.dropdown-menu').addClass('show');
        e.stopPropagation();
    });

    $(function () {
        $('[data-toggle="tooltip"]').tooltip()
    });
});