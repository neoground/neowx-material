// neowx-date-nav.js
// -----------------------------------------------------------------------------
// Interactive Material Date, Week, Month, and Year navigation pickers for
// NeoWX Material skin.
// Handles dropdown menus, strict date bounds [archive_start_date, archive_stop_date],
// and proactive 404 verification before navigation.
(function ($) {
    'use strict';

    if (!$) return;

    function pad2(n) {
        return (n < 10 ? '0' : '') + n;
    }

    function formatDateIso(d) {
        return d.getFullYear() + '-' + pad2(d.getMonth() + 1) + '-' + pad2(d.getDate());
    }

    function parseDateIso(str) {
        if (!str) return null;
        var parts = str.split('-');
        if (parts.length < 3) return null;
        return new Date(parseInt(parts[0], 10), parseInt(parts[1], 10) - 1, parseInt(parts[2], 10));
    }

    function getIsoWeekRange(year, week) {
        var simple = new Date(year, 0, 4);
        var dayOfWeek = simple.getDay() || 7;
        var monday = new Date(simple.getTime() + ((week - 1) * 7 + (1 - dayOfWeek)) * 86400000);
        var sunday = new Date(monday.getTime() + 6 * 86400000);
        return { start: monday, end: sunday };
    }

    function getIsoWeeksInYear(year) {
        var d = new Date(Date.UTC(year, 11, 28));
        var dayNum = d.getUTCDay() || 7;
        d.setUTCDate(d.getUTCDate() + 4 - dayNum);
        var yearStart = new Date(Date.UTC(d.getUTCFullYear(), 0, 1));
        return Math.ceil((((d - yearStart) / 86400000) + 1) / 7);
    }

    function getLocalizedMonthNames() {
        if (typeof moment !== 'undefined' && moment.monthsShort) {
            return moment.monthsShort();
        }
        return ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec'];
    }

    function getLocalizedWeekdayNames() {
        if (typeof moment !== 'undefined' && moment.weekdaysMin) {
            var minDays = moment.weekdaysMin(); // Sunday is 0
            // Reorder to Monday-first: Mon (1) to Sun (0)
            return [minDays[1], minDays[2], minDays[3], minDays[4], minDays[5], minDays[6], minDays[0]];
        }
        return ['Mo', 'Tu', 'We', 'Th', 'Fr', 'Sa', 'Su'];
    }

    function sanitizeRelativePath(path) {
        if (typeof path !== 'string') return '';
        var cleaned = path.trim();
        // Disallow external schemes, absolute URLs, or protocol-relative paths
        if (/^(?:[a-zA-Z][a-zA-Z0-9+.-]*:|\/\/)/.test(cleaned)) {
            return '';
        }
        // Allow only characters valid in safe relative path segments
        return cleaned.replace(/[^a-zA-Z0-9_\-\.\/]/g, '');
    }

    function DateNavController() {
        this.$hub = $('#nwm-nav-hub');
        this.$container = $('#nx-picker-container');
        this.$dropdown = $('#nx-picker-dropdown');
        if (!this.$container.length || !this.$dropdown.length) return;

        var rawBasePath = (typeof window.NX_BASE_PATH === 'string')
            ? window.NX_BASE_PATH
            : (this.$hub.attr('data-base-path') || '');
        this.basePath = sanitizeRelativePath(rawBasePath);

        this.page = (this.$hub.attr('data-page') || '').replace(/[^a-zA-Z0-9_\-]/g, '');
        this.activeNav = (this.$hub.attr('data-active-nav') || '').replace(/[^a-zA-Z0-9_\-]/g, '');

        this.contextYear = parseInt(this.$hub.attr('data-context-year') || new Date().getFullYear(), 10);
        this.contextMonth = parseInt(this.$hub.attr('data-context-month') || (new Date().getMonth() + 1), 10);
        this.contextWeek = parseInt(this.$hub.attr('data-context-week') || 1, 10);

        var rawDay = this.$hub.attr('data-context-day') || '';
        this.contextDay = /^\d{4}-\d{2}-\d{2}$/.test(rawDay) ? rawDay : formatDateIso(new Date());

        var rawStart = (typeof window.NX_START_DATE === 'string')
            ? window.NX_START_DATE
            : (this.$hub.attr('data-start-date') || '');
        this.startDateStr = /^\d{4}-\d{2}-\d{2}$/.test(rawStart) ? rawStart : '1970-01-01';

        var rawStop = (typeof window.NX_STOP_DATE === 'string')
            ? window.NX_STOP_DATE
            : (this.$hub.attr('data-stop-date') || '');
        this.stopDateStr = /^\d{4}-\d{2}-\d{2}$/.test(rawStop) ? rawStop : formatDateIso(new Date());

        this.startDate = parseDateIso(this.startDateStr);
        this.stopDate = parseDateIso(this.stopDateStr);

        var rawStartYear = parseInt(this.$hub.attr('data-start-year'), 10);
        this.startYear = this.startDate
            ? this.startDate.getFullYear()
            : ((!isNaN(rawStartYear) && rawStartYear > 1900) ? rawStartYear : 1970);

        var rawStopYear = parseInt(this.$hub.attr('data-stop-year'), 10);
        this.stopYear = this.stopDate
            ? this.stopDate.getFullYear()
            : ((!isNaN(rawStopYear) && rawStopYear > 1900) ? rawStopYear : new Date().getFullYear());

        this.redirectTimer = null;
        this.init();
    }

    DateNavController.prototype.init = function () {
        var self = this;

        // Prevent clicking inside picker controls from accidentally closing the Bootstrap dropdown
        this.$dropdown.on('click', function (e) {
            if ($(e.target).closest('a[href]').length) {
                return; // Let navigation links proceed to interceptor
            }
            e.stopPropagation();
        });

        // Intercept navigation clicks for proactive 404 checking
        $(document).on('click', '.nwm-stepper-group a[href], .nwm-picker-dropdown a[href]', function (e) {
            var $link = $(this);
            var href = $link.attr('href');
            var normalizedHref = (href || '').trim().toLowerCase();
            if (!href || href === '#' ||
                normalizedHref.indexOf('javascript:') === 0 ||
                normalizedHref.indexOf('data:') === 0 ||
                normalizedHref.indexOf('vbscript:') === 0) return;
            if ($link.hasClass('disabled') || $link.is(':disabled')) {
                e.preventDefault();
                return;
            }
            e.preventDefault();
            self.navigateWithCheck(href);
        });

        // Initialize picker type
        if (this.$dropdown.hasClass('nwm-picker-calendar')) {
            this.initDayPicker();
        } else if (this.$dropdown.hasClass('nwm-picker-week')) {
            this.initWeekPicker();
        } else if (this.$dropdown.hasClass('nwm-picker-month')) {
            this.initMonthPicker();
        }
        this.syncNextStepperButton();

        // Global export for nxJumpDate and external triggers
        window.nxNavigateWithCheck = function (url) {
            self.navigateWithCheck(url);
        };

        // Expose global updaters so live.json can refresh bounds dynamically
        window.nxUpdateStartDate = function (newStartDateStr) {
            if (!newStartDateStr || !/^\d{4}-\d{2}-\d{2}$/.test(newStartDateStr)) return;
            self.startDateStr = newStartDateStr;
            self.startDate = parseDateIso(newStartDateStr);
            if (self.startDate) {
                self.startYear = self.startDate.getFullYear();
            }
            self.reRenderActivePicker();
        };

        window.nxUpdateStopDate = function (newStopDateStr) {
            if (!newStopDateStr || !/^\d{4}-\d{2}-\d{2}$/.test(newStopDateStr)) return;
            self.stopDateStr = newStopDateStr;
            self.stopDate = parseDateIso(newStopDateStr);
            if (self.stopDate) {
                self.stopYear = self.stopDate.getFullYear();
            }
            self.reRenderActivePicker();
        };
    };

    DateNavController.prototype.reRenderActivePicker = function () {
        if (this.$dropdown.hasClass('nwm-picker-calendar') && this.viewCalendarYear) {
            this.renderCalendarPicker(this.viewCalendarYear, this.viewCalendarMonth);
        } else if (this.$dropdown.hasClass('nwm-picker-month') && this.viewMonthYear) {
            this.renderMonthPicker(this.viewMonthYear);
        } else if (this.$dropdown.hasClass('nwm-picker-week') && this.viewWeekYear) {
            this.renderWeekPicker(this.viewWeekYear);
        }
        this.syncNextStepperButton();
    };

    // Dynamically enables the Next (>) stepper button on frozen archive pages once
    // live.json reports a newer stopDateStr than the page's build-time stopDate.
    DateNavController.prototype.syncNextStepperButton = function () {
        if (!this.stopDate) return;
        if (this.activeNav === 'current' || this.activeNav === 'live' || this.page === 'today') return;

        var $nextSlot = this.$container.next('.nwm-nav-btn');
        if (!$nextSlot.length || !$nextSlot.is('button.disabled')) return;

        var nextUrl = '';
        if (this.$dropdown.hasClass('nwm-picker-calendar')) {
            var curDay = parseDateIso(this.contextDay);
            if (curDay) {
                var nextDay = new Date(curDay.getTime() + 86400000);
                var nextDayIso = formatDateIso(nextDay);
                if (nextDayIso <= this.stopDateStr) {
                    nextUrl = (nextDayIso === this.stopDateStr)
                        ? (this.basePath + 'index.html')
                        : (this.basePath + 'archive/' + nextDay.getFullYear() + '/days/day-' + nextDayIso + '.html');
                }
            }
        } else if (this.$dropdown.hasClass('nwm-picker-week')) {
            var curWkRange = getIsoWeekRange(this.contextYear, this.contextWeek);
            var nextWkMon = new Date(curWkRange.start.getTime() + 7 * 86400000);
            if (formatDateIso(nextWkMon) <= this.stopDateStr) {
                var d = new Date(Date.UTC(nextWkMon.getFullYear(), nextWkMon.getMonth(), nextWkMon.getDate()));
                var dayNum = d.getUTCDay() || 7;
                d.setUTCDate(d.getUTCDate() + 4 - dayNum);
                var isoYr = d.getUTCFullYear();
                var yearStart = new Date(Date.UTC(isoYr, 0, 1));
                var isoWk = Math.ceil((((d - yearStart) / 86400000) + 1) / 7);
                nextUrl = this.basePath + 'archive/' + isoYr + '/weeks/week-' + isoYr + '-' + pad2(isoWk) + '.html';
            }
        } else if (this.$dropdown.hasClass('nwm-picker-month')) {
            var nextMoYr = (this.contextMonth < 12) ? this.contextYear : (this.contextYear + 1);
            var nextMo = (this.contextMonth < 12) ? (this.contextMonth + 1) : 1;
            var nextMoFirstIso = nextMoYr + '-' + pad2(nextMo) + '-01';
            if (nextMoFirstIso <= this.stopDateStr) {
                nextUrl = this.basePath + 'archive/' + nextMoYr + '/months/month-' + nextMoYr + '-' + pad2(nextMo) + '.html';
            }
        } else if (this.$dropdown.hasClass('nwm-picker-year')) {
            var nextYr = this.contextYear + 1;
            if (nextYr <= this.stopYear) {
                nextUrl = this.basePath + 'archive/' + nextYr + '/year-' + nextYr + '.html';
            }
        }

        if (nextUrl) {
            var $link = $('<a>', {
                href: nextUrl,
                class: 'nwm-nav-btn',
                'data-toggle': 'tooltip',
                'data-placement': 'bottom',
                title: 'Next',
                'aria-label': 'Next'
            }).append($('<i>', { class: 'mdi mdi-chevron-right' }));
            $nextSlot.replaceWith($link);
            if ($.fn.tooltip) {
                $link.tooltip();
            }
        }
    };

    // -------------------------------------------------------------------------
    // PROACTIVE 404 AVAILABILITY CHECK & NAVIGATION
    // -------------------------------------------------------------------------
    DateNavController.prototype.navigateWithCheck = function (url) {
        var normalizedUrl = (url || '').trim().toLowerCase();
        if (!url || url === '#' ||
            normalizedUrl.indexOf('javascript:') === 0 ||
            normalizedUrl.indexOf('data:') === 0 ||
            normalizedUrl.indexOf('vbscript:') === 0) return;
        var self = this;

        // If navigating to the current date's day page, always route to index.html
        var todayIso = formatDateIso(new Date());
        if ((this.stopDateStr && url.indexOf('day-' + this.stopDateStr + '.html') !== -1) ||
            (todayIso && url.indexOf('day-' + todayIso + '.html') !== -1)) {
            url = this.basePath + 'index.html';
        }

        // Direct navigation if file: protocol (local testing) or already targeting index.html
        if (window.location.protocol === 'file:' || url === (this.basePath + 'index.html') || url === 'index.html') {
            window.location.href = url;
            return;
        }

        // Only probe local paths
        try {
            var resolved = new URL(url, window.location.href);
            if (resolved.origin !== window.location.origin) {
                window.location.href = url;
                return;
            }
        } catch (e) {
            window.location.href = url;
            return;
        }

        // 1200ms timeout probe with AbortController
        var controller = (typeof AbortController !== 'undefined') ? new AbortController() : null;
        var signal = controller ? controller.signal : undefined;
        var timer = setTimeout(function () {
            if (controller) controller.abort();
        }, 1200);

        fetch(url, { method: 'HEAD', signal: signal, cache: 'no-cache' })
            .then(function (res) {
                clearTimeout(timer);
                if (res.status === 404) {
                    self.showMissingPageModal(url);
                } else {
                    window.location.href = url;
                }
            })
            .catch(function () {
                clearTimeout(timer);
                // On abort, timeout, or network issue, proceed with standard navigation
                window.location.href = url;
            });
    };

    DateNavController.prototype.showMissingPageModal = function () {
        var $modal = $('#nwm-date-missing-modal');
        var basePath = this.basePath;

        if (!$modal.length || !$.fn.modal) {
            // Fallback if modal is not in DOM
            window.location.href = basePath + 'index.html';
            return;
        }

        var countdown = 3;
        var $countdownEl = $('#nwm-redirect-countdown');
        if ($countdownEl.length) {
            $countdownEl.text(countdown + 's');
        }

        if (this.redirectTimer) {
            clearInterval(this.redirectTimer);
            this.redirectTimer = null;
        }

        var self = this;
        this.redirectTimer = setInterval(function () {
            countdown--;
            if ($countdownEl.length) {
                $countdownEl.text(countdown + 's');
            }
            if (countdown <= 0) {
                clearInterval(self.redirectTimer);
                self.redirectTimer = null;
                window.location.href = basePath + 'index.html';
            }
        }, 1000);

        $modal.off('hidden.bs.modal.nx404').on('hidden.bs.modal.nx404', function () {
            if (self.redirectTimer) {
                clearInterval(self.redirectTimer);
                self.redirectTimer = null;
            }
        });

        $('#nwm-redirect-now-btn').attr('href', basePath + 'index.html');
        $modal.modal('show');
    };

    // -------------------------------------------------------------------------
    // MONTH PICKER
    // -------------------------------------------------------------------------
    DateNavController.prototype.initMonthPicker = function () {
        var self = this;
        this.viewMonthYear = this.contextYear;
        // Normalize picker button label to YYYY-MM even on previously generated archive pages
        $('#nx-picker-label').text(this.contextYear + '-' + pad2(this.contextMonth));
        this.renderMonthPicker(this.viewMonthYear);

        this.$dropdown.on('click', '[data-action="prev-year"]', function (e) {
            e.preventDefault();
            if (self.viewMonthYear > self.startYear) {
                self.viewMonthYear--;
                self.renderMonthPicker(self.viewMonthYear);
            }
        });

        this.$dropdown.on('click', '[data-action="next-year"]', function (e) {
            e.preventDefault();
            if (self.viewMonthYear < self.stopYear) {
                self.viewMonthYear++;
                self.renderMonthPicker(self.viewMonthYear);
            }
        });

        this.$dropdown.on('change', '#nx-mp-year-select', function () {
            self.viewMonthYear = parseInt($(this).val(), 10);
            self.renderMonthPicker(self.viewMonthYear);
        });
    };

    DateNavController.prototype.renderMonthPicker = function (year) {
        var monthNames = getLocalizedMonthNames();

        // Header with year navigation
        var $header = $('<div>', { class: 'nwm-picker-header' });

        var $prevBtn = $('<button>', {
            type: 'button',
            class: 'nwm-picker-nav-btn',
            'data-action': 'prev-year',
            'aria-label': 'Previous year'
        }).append($('<i>', { class: 'mdi mdi-chevron-left' }));
        if (year <= this.startYear) {
            $prevBtn.prop('disabled', true);
        }

        var $select = $('<select>', {
            class: 'nwm-picker-select',
            id: 'nx-mp-year-select',
            'aria-label': 'Year'
        });
        for (var y = this.stopYear; y >= this.startYear; y--) {
            var $opt = $('<option>', { value: y, text: y });
            if (y === year) {
                $opt.prop('selected', true);
            }
            $select.append($opt);
        }

        var $nextBtn = $('<button>', {
            type: 'button',
            class: 'nwm-picker-nav-btn',
            'data-action': 'next-year',
            'aria-label': 'Next year'
        }).append($('<i>', { class: 'mdi mdi-chevron-right' }));
        if (year >= this.stopYear) {
            $nextBtn.prop('disabled', true);
        }

        $header.append($prevBtn, $select, $nextBtn);

        // 12 Months Grid with strict clamping
        var minMo = (year === this.startYear && this.startDate) ? (this.startDate.getMonth() + 1) : 1;
        var maxMo = (year === this.stopYear && this.stopDate) ? (this.stopDate.getMonth() + 1) : 12;

        var $grid = $('<div>', { class: 'nwm-picker-grid-months' });
        for (var m = 1; m <= 12; m++) {
            var mStr = pad2(m);
            var mName = monthNames[m - 1] || mStr;

            var firstDayStr = year + '-' + mStr + '-01';
            var daysInMonth = new Date(year, m, 0).getDate();
            var lastDayStr = year + '-' + mStr + '-' + pad2(daysInMonth);

            var isDisabled = (m < minMo || m > maxMo) || (lastDayStr < this.startDateStr) || (firstDayStr > this.stopDateStr);
            var isActive = (year === this.contextYear && m === this.contextMonth);

            if (isDisabled) {
                $grid.append($('<span>', {
                    class: 'nwm-picker-cell disabled',
                    'data-toggle': 'tooltip',
                    'data-placement': 'top',
                    title: 'No data',
                    text: mName
                }));
            } else {
                var url = this.basePath + 'archive/' + year + '/months/month-' + year + '-' + mStr + '.html';
                var $cell = $('<a>', {
                    href: url,
                    class: 'nwm-picker-cell' + (isActive ? ' active' : ''),
                    text: mName
                });
                $grid.append($cell);
            }
        }

        this.$dropdown.empty().append($header, $grid);
        if ($.fn.tooltip) {
            this.$dropdown.find('[data-toggle="tooltip"]').tooltip();
        }
    };

    // -------------------------------------------------------------------------
    // WEEK PICKER
    // -------------------------------------------------------------------------
    DateNavController.prototype.initWeekPicker = function () {
        var self = this;
        this.viewWeekYear = this.contextYear;
        this.weekMonthFilter = 'all'; // 'all' or 1..12
        this.renderWeekPicker(this.viewWeekYear);

        this.$dropdown.on('click', '[data-action="prev-year"]', function (e) {
            e.preventDefault();
            if (self.viewWeekYear > self.startYear) {
                self.viewWeekYear--;
                self.renderWeekPicker(self.viewWeekYear);
            }
        });

        this.$dropdown.on('click', '[data-action="next-year"]', function (e) {
            e.preventDefault();
            if (self.viewWeekYear < self.stopYear) {
                self.viewWeekYear++;
                self.renderWeekPicker(self.viewWeekYear);
            }
        });

        this.$dropdown.on('change', '#nx-wp-year-select', function () {
            self.viewWeekYear = parseInt($(this).val(), 10);
            self.renderWeekPicker(self.viewWeekYear);
        });

        this.$dropdown.on('click', '.nwm-picker-tab', function (e) {
            e.preventDefault();
            if ($(this).hasClass('disabled') || $(this).is(':disabled')) return;
            var filter = $(this).attr('data-month-filter') || 'all';
            self.weekMonthFilter = (filter === 'all' || /^(?:[1-9]|1[0-2])$/.test(filter)) ? filter : 'all';
            self.renderWeekPicker(self.viewWeekYear);
        });
    };

    DateNavController.prototype.renderWeekPicker = function (year) {
        var monthNames = getLocalizedMonthNames();
        var totalWeeks = getIsoWeeksInYear(year);

        // Header with year navigation
        var $header = $('<div>', { class: 'nwm-picker-header' });

        var $prevBtn = $('<button>', {
            type: 'button',
            class: 'nwm-picker-nav-btn',
            'data-action': 'prev-year',
            'aria-label': 'Previous year'
        }).append($('<i>', { class: 'mdi mdi-chevron-left' }));
        if (year <= this.startYear) {
            $prevBtn.prop('disabled', true);
        }

        var $select = $('<select>', {
            class: 'nwm-picker-select',
            id: 'nx-wp-year-select',
            'aria-label': 'Year'
        });
        for (var y = this.stopYear; y >= this.startYear; y--) {
            var $opt = $('<option>', { value: y, text: y });
            if (y === year) {
                $opt.prop('selected', true);
            }
            $select.append($opt);
        }

        var $nextBtn = $('<button>', {
            type: 'button',
            class: 'nwm-picker-nav-btn',
            'data-action': 'next-year',
            'aria-label': 'Next year'
        }).append($('<i>', { class: 'mdi mdi-chevron-right' }));
        if (year >= this.stopYear) {
            $nextBtn.prop('disabled', true);
        }

        $header.append($prevBtn, $select, $nextBtn);

        // Month quick filter tabs
        var minMo = (year === this.startYear && this.startDate) ? (this.startDate.getMonth() + 1) : 1;
        var maxMo = (year === this.stopYear && this.stopDate) ? (this.stopDate.getMonth() + 1) : 12;

        var $tabs = $('<div>', { class: 'nwm-picker-filter-tabs' });
        $tabs.append($('<button>', {
            type: 'button',
            class: 'nwm-picker-tab' + (this.weekMonthFilter === 'all' ? ' active' : ''),
            'data-month-filter': 'all',
            text: 'All'
        }));
        for (var m = 1; m <= 12; m++) {
            var isTabDisabled = (m < minMo || m > maxMo);
            var $tab = $('<button>', {
                type: 'button',
                class: 'nwm-picker-tab' +
                    (this.weekMonthFilter === String(m) ? ' active' : '') +
                    (isTabDisabled ? ' disabled' : ''),
                'data-month-filter': String(m),
                text: monthNames[m - 1] || String(m)
            });
            if (isTabDisabled) {
                $tab.prop('disabled', true);
            }
            $tabs.append($tab);
        }

        // Weeks Grid
        var $grid = $('<div>', { class: 'nwm-picker-grid-weeks' });
        for (var w = 1; w <= totalWeeks; w++) {
            var wStr = pad2(w);
            var range = getIsoWeekRange(year, w);
            var startIso = formatDateIso(range.start);
            var endIso = formatDateIso(range.end);

            // Filter check: does week overlap the filtered month?
            if (this.weekMonthFilter !== 'all') {
                var targetMo = parseInt(this.weekMonthFilter, 10) - 1;
                var overlap = (range.start.getMonth() === targetMo || range.end.getMonth() === targetMo);
                if (!overlap) continue;
            }

            var isDisabled = (endIso < this.startDateStr) || (startIso > this.stopDateStr);
            var isActive = (year === this.contextYear && w === this.contextWeek);

            var rangeLabel = pad2(range.start.getDate()) + '.' + pad2(range.start.getMonth() + 1) + ' - ' +
                             pad2(range.end.getDate()) + '.' + pad2(range.end.getMonth() + 1);

            if (isDisabled) {
                $grid.append($('<span>', {
                    class: 'nwm-picker-cell disabled',
                    'data-toggle': 'tooltip',
                    'data-placement': 'top',
                    title: 'No data (CW ' + wStr + ')',
                    text: 'CW ' + wStr
                }));
            } else {
                var url = this.basePath + 'archive/' + year + '/weeks/week-' + year + '-' + wStr + '.html';
                $grid.append($('<a>', {
                    href: url,
                    class: 'nwm-picker-cell' + (isActive ? ' active' : ''),
                    'data-toggle': 'tooltip',
                    'data-placement': 'top',
                    title: rangeLabel,
                    text: 'CW ' + wStr
                }));
            }
        }

        this.$dropdown.empty().append($header, $tabs, $grid);
        if ($.fn.tooltip) {
            this.$dropdown.find('[data-toggle="tooltip"]').tooltip();
        }
    };

    // -------------------------------------------------------------------------
    // DAY CALENDAR PICKER
    // -------------------------------------------------------------------------
    DateNavController.prototype.initDayPicker = function () {
        var self = this;
        var parts = this.contextDay.split('-');
        this.viewCalendarYear = parseInt(parts[0], 10) || this.contextYear;
        this.viewCalendarMonth = parseInt(parts[1], 10) || this.contextMonth;

        this.renderCalendarPicker(this.viewCalendarYear, this.viewCalendarMonth);

        this.$dropdown.on('click', '[data-action="prev-month"]', function (e) {
            e.preventDefault();
            var minMo = (self.viewCalendarYear === self.startYear && self.startDate) ? (self.startDate.getMonth() + 1) : 1;
            if (self.viewCalendarMonth > minMo) {
                self.viewCalendarMonth--;
            } else if (self.viewCalendarYear > self.startYear) {
                self.viewCalendarYear--;
                self.viewCalendarMonth = 12;
            }
            self.renderCalendarPicker(self.viewCalendarYear, self.viewCalendarMonth);
        });

        this.$dropdown.on('click', '[data-action="next-month"]', function (e) {
            e.preventDefault();
            var maxMo = (self.viewCalendarYear === self.stopYear && self.stopDate) ? (self.stopDate.getMonth() + 1) : 12;
            if (self.viewCalendarMonth < maxMo) {
                self.viewCalendarMonth++;
            } else if (self.viewCalendarYear < self.stopYear) {
                self.viewCalendarYear++;
                self.viewCalendarMonth = 1;
            }
            self.renderCalendarPicker(self.viewCalendarYear, self.viewCalendarMonth);
        });

        this.$dropdown.on('change', '#nx-dp-month-select', function () {
            self.viewCalendarMonth = parseInt($(this).val(), 10);
            self.renderCalendarPicker(self.viewCalendarYear, self.viewCalendarMonth);
        });

        this.$dropdown.on('change', '#nx-dp-year-select', function () {
            self.viewCalendarYear = parseInt($(this).val(), 10);
            self.renderCalendarPicker(self.viewCalendarYear, self.viewCalendarMonth);
        });
    };

    DateNavController.prototype.renderCalendarPicker = function (year, month) {
        var monthNames = getLocalizedMonthNames();
        var weekdayNames = getLocalizedWeekdayNames();

        var minMo = (year === this.startYear && this.startDate) ? (this.startDate.getMonth() + 1) : 1;
        var maxMo = (year === this.stopYear && this.stopDate) ? (this.stopDate.getMonth() + 1) : 12;

        // Auto-clamp month if out of bounds for the selected year
        if (month < minMo) month = minMo;
        if (month > maxMo) month = maxMo;
        this.viewCalendarMonth = month;

        var canGoPrev = (year > this.startYear) || (year === this.startYear && month > minMo);
        var canGoNext = (year < this.stopYear) || (year === this.stopYear && month < maxMo);

        // Header
        var $header = $('<div>', { class: 'nwm-picker-header' });

        var $prevBtn = $('<button>', {
            type: 'button',
            class: 'nwm-picker-nav-btn',
            'data-action': 'prev-month',
            'aria-label': 'Previous month'
        }).append($('<i>', { class: 'mdi mdi-chevron-left' }));
        if (!canGoPrev) {
            $prevBtn.prop('disabled', true);
        }

        var $selectGroup = $('<div>', {
            class: 'd-flex align-items-center',
            style: 'gap: 0.25rem;'
        });

        // Month dropdown with active boundary disabling
        var $monthSelect = $('<select>', {
            class: 'nwm-picker-select',
            id: 'nx-dp-month-select',
            'aria-label': 'Month'
        });
        for (var m = 1; m <= 12; m++) {
            var isMoDisabled = (m < minMo || m > maxMo);
            var $mOpt = $('<option>', {
                value: m,
                text: monthNames[m - 1] || String(m)
            });
            if (isMoDisabled) {
                $mOpt.prop('disabled', true);
            }
            if (m === month) {
                $mOpt.prop('selected', true);
            }
            $monthSelect.append($mOpt);
        }

        // Year dropdown strictly within [startYear, stopYear]
        var $yearSelect = $('<select>', {
            class: 'nwm-picker-select',
            id: 'nx-dp-year-select',
            'aria-label': 'Year'
        });
        for (var y = this.stopYear; y >= this.startYear; y--) {
            var $yOpt = $('<option>', { value: y, text: y });
            if (y === year) {
                $yOpt.prop('selected', true);
            }
            $yearSelect.append($yOpt);
        }

        $selectGroup.append($monthSelect, $yearSelect);

        var $nextBtn = $('<button>', {
            type: 'button',
            class: 'nwm-picker-nav-btn',
            'data-action': 'next-month',
            'aria-label': 'Next month'
        }).append($('<i>', { class: 'mdi mdi-chevron-right' }));
        if (!canGoNext) {
            $nextBtn.prop('disabled', true);
        }

        $header.append($prevBtn, $selectGroup, $nextBtn);

        // Weekday header row (Mo..Su)
        var $weekdays = $('<div>', { class: 'nwm-picker-grid-weekdays' });
        for (var wd = 0; wd < 7; wd++) {
            $weekdays.append($('<span>', { text: weekdayNames[wd] }));
        }

        // Days Grid
        var $daysGrid = $('<div>', { class: 'nwm-picker-grid-days' });
        var firstDayIndex = new Date(year, month - 1, 1).getDay(); // 0 is Sun, 1 is Mon
        var leadingBlanks = (firstDayIndex === 0) ? 6 : firstDayIndex - 1; // convert to Mon = 0
        for (var b = 0; b < leadingBlanks; b++) {
            $daysGrid.append($('<span>', { class: 'nwm-picker-empty-cell' }));
        }

        var daysInMonth = new Date(year, month, 0).getDate();
        var todayStr = formatDateIso(new Date());

        for (var d = 1; d <= daysInMonth; d++) {
            var dateStr = year + '-' + pad2(month) + '-' + pad2(d);
            var isDisabled = (dateStr < this.startDateStr) || (dateStr > this.stopDateStr);
            var isActive = (dateStr === this.contextDay);
            var isToday = (dateStr === todayStr);

            var cellClasses = 'nwm-picker-cell' +
                (isActive ? ' active' : '') +
                (isToday ? ' today' : '');

            if (isDisabled) {
                $daysGrid.append($('<span>', {
                    class: cellClasses + ' disabled',
                    'data-toggle': 'tooltip',
                    'data-placement': 'top',
                    title: 'No data',
                    text: String(d)
                }));
            } else {
                var isCurrentDay = (dateStr === this.stopDateStr || dateStr === todayStr);
                var url = isCurrentDay
                    ? (this.basePath + 'index.html')
                    : (this.basePath + 'archive/' + year + '/days/day-' + dateStr + '.html');
                $daysGrid.append($('<a>', {
                    href: url,
                    class: cellClasses,
                    text: String(d)
                }));
            }
        }

        this.$dropdown.empty().append($header, $weekdays, $daysGrid);
        if ($.fn.tooltip) {
            this.$dropdown.find('[data-toggle="tooltip"]').tooltip();
        }
    };

    // Run immediately if #nwm-nav-hub is already in the DOM (js.inc loads after header.inc),
    // so the month label and picker initialize without waiting for chart scripts to finish.
    if ($('#nwm-nav-hub').length) {
        new DateNavController();
    } else {
        $(function () {
            new DateNavController();
        });
    }

})(window.jQuery);
