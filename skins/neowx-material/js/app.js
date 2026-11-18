// v1.0.1
// Tooltip support
$(function () {
    $('[data-toggle="tooltip"]').tooltip()
});

// Number rounding based on weewx values
// Example: Number: 34.5678 Format: %.2f Result: 34.57
function formatNumber(no, format) {
    // Guard non-numeric values (null/undefined for hidden series or null data
    // gaps): calling .toFixed on them throws and aborts the whole tooltip
    // render, so the tooltip vanishes after toggling a series off the legend.
    if (typeof no !== 'number' || !isFinite(no)) {
        return no;
    }
    // Extract number of decimal places from format
    format = format.replace(/[^0-9]/g, '');
    return no.toFixed(format);
}

// --- HELPER FUNCTION: Debug logging (respects debug flag) ---
function debugLog(message) {
    if (window.MQTT_CONFIG && window.MQTT_CONFIG.debug) {
        console.log('[MQTT DEBUG] ' + message);
    }
}

document.addEventListener('DOMContentLoaded', function () {
    const valueCards = document.querySelectorAll('.card-value');
    const valueCardNames = Array.from(valueCards).map(card => card.getAttribute('data-name'));
    debugLog('Value cards: ' + valueCardNames);

    const cardCharts = document.querySelectorAll('.card-chart');
    const cardChartNames = Array.from(cardCharts).map(card => card.getAttribute('data-name'));
    debugLog('Chart cards: ' + cardChartNames);

    // An item can appear in several sections, so a name matches several cards.
    // Pick the one nearest the element clicked rather than the first in the
    // document: clicking a card in a Temperature panel should reach the chart
    // in that panel, not a copy at the top of the page.
    //
    // Scoping by data-section instead cannot work here - a value card is in a
    // content = card section and its chart in a content = chart section, so
    // their slugs never match.
    function findJumpTarget(selector, from) {
        if (!from) return null;
        var all = Array.from(document.querySelectorAll(selector));
        var visible = all.filter(function (el) { return el.offsetParent !== null; });
        if (!visible.length) return all[0] || null;
        var y = from.getBoundingClientRect().top + window.pageYOffset;
        return visible.reduce(function (best, el) {
            var d = Math.abs(el.getBoundingClientRect().top + window.pageYOffset - y);
            return d < best.d ? { el: el, d: d } : best;
        }, { el: visible[0], d: Infinity }).el;
    }

    // A target inside a collapsed panel has display:none, so its bounding rect
    // is all zeros and scrollToElement would scroll to (pageYOffset - 100):
    // a ~100px twitch upward, with the highlight flashing on something
    // invisible. Expand first, then scroll once layout has settled.
    //
    // Bootstrap 4.4.1, so this is the jQuery plugin, not a Bootstrap 5
    // static instance getter.
    function jumpTo(target) {
        if (!target) return;
        // A panel mid-animation carries .collapsing rather than .collapse, so
        // matching only the latter silently scrolls on geometry that is about
        // to change.
        var body = target.closest('.collapse, .collapsing');
        if (body && !body.classList.contains('show')) {
            // Bootstrap sets aria-expanded on the trigger itself during
            // show(), and the trigger IS this header - so we do not touch it.
            $(body).one('shown.bs.collapse', function () {
                scrollToElement(target, true);
            });
            $(body).collapse('show');
            return;
        }
        scrollToElement(target, true);
    }

    // --- PANEL STATE ---------------------------------------------------
    // head.inc has already painted the remembered state via an injected
    // stylesheet. Our job is to make the real DOM agree, then drop that
    // override, then keep storage in step with what the user does next.
    var NWM_PANEL_KEY = 'nwm-panels';

    function nwmReadPanels() {
        // sessionStorage THROWS rather than returning null in private mode
        // and wherever site data is blocked, so every access is guarded.
        try {
            var raw = sessionStorage.getItem(NWM_PANEL_KEY);
            var parsed = raw ? JSON.parse(raw) : {};
            if (!parsed || typeof parsed !== 'object' || Array.isArray(parsed)) {
                return {};
            }
            return parsed;
        } catch (e) {
            return {};
        }
    }

    function nwmWritePanel(slug, expanded) {
        try {
            var state = nwmReadPanels();
            state[slug] = !!expanded;
            sessionStorage.setItem(NWM_PANEL_KEY, JSON.stringify(state));
        } catch (e) {
            // Storage unavailable. The page still works; it just forgets.
        }
    }

    (function nwmPanelState() {
        // head.inc sets this only inside its own rememberPanelState #if, and
        // only it emits the head-injected restore override - so when it's
        // unset there is nothing to correct or remove, and no future toggle
        // is worth persisting. Without this gate, rememberPanelState = false
        // still bound these handlers and still corrected the DOM below,
        // reproducing the exact flash the setting exists to turn off.
        if (window.NWM_REMEMBER_PANELS !== true) return;
        var panels = document.querySelectorAll('.nwm-expansion-panel[data-section]');
        var stored = nwmReadPanels();

        try {
            panels.forEach(function (panel) {
                var header = panel.querySelector(':scope > .nwm-panel-header');
                // :scope > .collapse must keep selecting the same element as
                // head.inc's "header + .collapse" - the two halves agree on
                // which node is the panel body only because both selectors
                // resolve to it.
                var body = panel.querySelector(':scope > .collapse');
                if (!header || !body) return;
                // A collapsed = none panel has no toggle. Restoring one to
                // collapsed would hide it with no way to reopen it.
                if (header.classList.contains('nwm-panel-static')) return;

                var slug = panel.getAttribute('data-section');

                // STEP 1 of 3: correct the DOM to match what the injected CSS is
                // already showing. Set the class directly rather than going
                // through Bootstrap's animated API - this must not animate.
                if (Object.prototype.hasOwnProperty.call(stored, slug)) {
                    var wantOpen = !!stored[slug];
                    body.classList.toggle('show', wantOpen);
                    header.setAttribute('aria-expanded', wantOpen ? 'true' : 'false');
                }

                // STEP 3 of 3: keep storage in step from here on. Bootstrap fires
                // these after the transition completes, and it maintains
                // aria-expanded on the trigger itself, so we only record.
                $(body).on('shown.bs.collapse', function () { nwmWritePanel(slug, true); });
                $(body).on('hidden.bs.collapse', function () { nwmWritePanel(slug, false); });
            });
        } finally {
            // STEP 2 of 3: drop the override, whether or not the loop above
            // completed. This is visually inert ONLY because step 1 already
            // ran for every panel it reached before this fires - a finally
            // runs after the try body, so that ordering holds even on a
            // thrown error. Moving this above the loop (or back into the try,
            // after the loop, guarded by an early return on no panels) would
            // let a restored panel snap into place, or - worse, on a thrown
            // error - leave the override's unconditional rules pinning a
            // stored-collapsed panel hidden with no way to reopen it.
            var override = document.getElementById('nwm-panel-restore');
            if (override && override.parentNode) {
                override.parentNode.removeChild(override);
            }
        }
    })();

    // --- CLICK HANDLER 1: Value Cards -> Jump to Chart Cards ---
    valueCards.forEach(function(valueCard) {
        valueCard.addEventListener('click', function(e) {
            // Get the sensor name from data-name attribute
            const sensorName = valueCard.getAttribute('data-name');
            if (!sensorName) {
                debugLog('⚠️ No data-name found on value card');
                return;
            }

            // The wind icon jumps to the wind vector chart. This looked up
            // 'embedded_imageWindVect' from 2021 until 2026 and never worked -
            // no template has ever emitted that id - so the click fell through
            // to the i.wi guard below and was swallowed.
            //
            // No imageWindVect fallback: that card does not exist and will not.
            if (sensorName === 'windSpeed' && e.target.closest('i.wi')) {
                const vectorChart = findJumpTarget('.card-chart[data-name="windvec"]', valueCard);
                if (vectorChart) {
                    jumpTo(vectorChart);
                    e.preventDefault();
                    e.stopPropagation();
                    return;
                }
            }

            // Don't navigate if clicking on icons (except wind)
            if (e.target.closest('i.wi')) {
                e.preventDefault();
                e.stopPropagation();
                return;
            }

            // Map sensor to chart name
            let chartSensorName = sensorName;

            // Special mappings: some sensors share the same chart
            if (sensorName === 'appTemp' || sensorName === 'dewpoint') {
                chartSensorName = 'outTemp';
            } else if (sensorName === 'heatindex') {
                chartSensorName = 'windchill';
            }

            // Find the corresponding chart card by data-name (direct match first)
            let chartCard = findJumpTarget('.card-chart[data-name="' + chartSensorName + '"]', valueCard);

            // Fallback: search custom charts whose data-sensors attribute contains this sensor
            if (!chartCard) {
                const customCharts = document.querySelectorAll('.card-chart[data-sensors]');
                const matches = [];
                for (const cc of customCharts) {
                    const sensors = cc.getAttribute('data-sensors').split(',').map(function(s) { return s.trim(); });
                    if (sensors.indexOf(chartSensorName) !== -1) {
                        matches.push(cc);
                    }
                }
                if (matches.length) {
                    chartCard = findJumpTarget('.card-chart[data-sensors]', valueCard);
                    if (matches.indexOf(chartCard) === -1) chartCard = matches[0];
                }
            }

            if (chartCard) {
                jumpTo(chartCard);
                debugLog('✓ Jumped from value card "' + sensorName + '" to chart "' + chartCard.getAttribute('data-name') + '"');
            } else {
                debugLog('⚠️ No chart card found for sensor: ' + chartSensorName);
            }
        });
    });

    // --- CLICK HANDLER 2: Chart Cards -> Jump to Value Cards ---
    cardCharts.forEach(function(chartCard) {
        chartCard.addEventListener('click', function(e) {
            // Only respond to clicks on the title (H5 element)
            if (e.target.tagName !== 'H5') {
                return;
            }

            // Get the sensor name from data-name attribute
            const sensorName = chartCard.getAttribute('data-name');
            if (!sensorName) {
                debugLog('⚠️ No data-name found on chart card');
                return;
            }

            // Map chart name to value card name
            let valueSensorName = sensorName;

            // Special mappings
            if (sensorName === 'windvec') {
                valueSensorName = 'windSpeed';
            }

            // Find the corresponding value card by data-name (direct match first)
            let valueCard = findJumpTarget('.card-value[data-name="' + valueSensorName + '"]', chartCard);

            // Fallback: if this is a custom chart, use data-sensors to find first existing value card
            if (!valueCard) {
                const dataSensors = chartCard.getAttribute('data-sensors');
                if (dataSensors) {
                    const sensors = dataSensors.split(',').map(function(s) { return s.trim(); });
                    for (const sensor of sensors) {
                        const candidate = findJumpTarget('.card-value[data-name="' + sensor + '"]', chartCard);
                        if (candidate) {
                            valueCard = candidate;
                            break;
                        }
                    }
                }
            }

            if (valueCard) {
                jumpTo(valueCard);
                debugLog('✓ Jumped from chart "' + sensorName + '" to value card "' + valueCard.getAttribute('data-name') + '"');
                e.preventDefault();
            } else {
                debugLog('⚠️ No value card found for sensor: ' + valueSensorName);
            }
        });
    });

    // --- HELPER FUNCTION: Smooth scroll to element with visual feedback ---
    function scrollToElement(element, highlightBackground) {
        if (!element) return;

        const topOffset = element.getBoundingClientRect().top + window.pageYOffset - 100;
        window.scrollTo({
            top: topOffset,
            behavior: 'smooth'
        });

        // Optional green background flash
        if (highlightBackground) {
            element.style.transition = "background-color 0.5s";
            element.style.backgroundColor = "rgba(0, 255, 0, 0.1)";
            setTimeout(function() {
                element.style.backgroundColor = "";
            }, 1000);
        }
    }
});

// --- POP-UP WINDOW --------------------------------------------------------
// One shared modal, used by two things:
//
//   - charts, through an ApexCharts toolbar customIcon registered in js.inc
//     when Appearance/enableChartPopup is on (ApexCharts 5 binds the click as
//     click(chartCtx, w, event));
//   - [[Embedded]] iFrame and image cards, through the toolbar button the
//     templates emit when Appearance/enableEmbeddedPopup is on.

var NEOWX_POPUP_ID = 'neowx-popup';

// There is one modal, one mount point and one _neowxPopupChart slot, so only
// one pop-up can be open at a time. Every path that sets this must also clear
// it: a guard left stuck on makes every expand button on the page stop
// responding, with nothing on screen or in the console to explain why.
var _neowxPopupActive = false;
var _neowxPopupTeardown = null;

// Runs the pending teardown once. A teardown that throws must not abort the
// rest of the close: ApexCharts' destroy() reaches into state that only exists
// once render() has finished, and letting that escape would strand the guard.
function neowxRunTeardown() {
    var teardown = _neowxPopupTeardown;
    _neowxPopupTeardown = null;
    if (!teardown) {
        return;
    }
    try {
        teardown();
    } catch (e) {
        console.error('neowx-material: closing the pop-up window failed', e);
    }
}

// Builds the modal on first use and returns it on every call.
function neowxPopupModal() {
    var existing = document.getElementById(NEOWX_POPUP_ID);
    if (existing) {
        return existing;
    }

    var wrapper = document.createElement('div');
    wrapper.innerHTML = [
        '<div class="modal fade" id="' + NEOWX_POPUP_ID + '" tabindex="-1" role="dialog"',
        ' aria-labelledby="' + NEOWX_POPUP_ID + '-title" aria-hidden="true">',
        '<div class="modal-dialog modal-xl modal-dialog-centered" role="document">',
        '<div class="modal-content">',
        '<div class="modal-header">',
        '<h5 class="modal-title" id="' + NEOWX_POPUP_ID + '-title"></h5>',
        // The [[Embedded]] cards' toolbar, carrying nothing but a close button.
        // In "align" mode the chart's own toolbar takes this exact place and
        // ends with the same button, so the two never look like two different
        // controls, and the corner never shifts between modes.
        '<div class="nwm-embed-toolbar nwm-popup-close">',
        '<button type="button" class="nwm-embed-toolbar-icon" data-dismiss="modal">',
        NEOWX_CLOSE_ICON,
        '</button>',
        '</div>',
        '</div>',
        '<div class="modal-body">',
        '<div id="' + NEOWX_POPUP_ID + '-mount"></div>',
        '</div>',
        '</div>',
        '</div>',
        '</div>'
    ].join('');
    var modal = wrapper.firstElementChild;
    var closeLabel = neowxText('close', 'Close');
    var closeButton = modal.querySelector('.nwm-popup-close button');
    closeButton.setAttribute('aria-label', closeLabel);
    closeButton.title = closeLabel;

    // The dialog title is the card heading, so give it the classes the cards
    // give theirs (js.inc emits them, since the accent colour is a skin.conf
    // setting). Read here rather than at load time: js.inc runs after app.js.
    var titleClasses = (window.NEOWX_TITLE_CLASS || '').split(/\s+/);
    var titleEl = modal.querySelector('.modal-title');
    for (var i = 0; i < titleClasses.length; i++) {
        if (titleClasses[i] !== '') {
            titleEl.classList.add(titleClasses[i]);
        }
    }

    document.body.appendChild(modal);

    // Bootstrap already closes on ESC and on a click outside the dialog, so all
    // that is left here is emptying the dialog out again.
    $(modal).on('hidden.bs.modal', function () {
        // Bootstrap 4 ignores hide() while the dialog is still fading in, so a
        // pending shown handler should never outlive a close. Drop it anyway
        // rather than let that behaviour be load-bearing.
        // Released first, ahead of everything that could throw: jQuery does not
        // wrap handlers, so a throw below would strand the guard and, as the
        // note on it says, leave every expand button on the page dead with
        // nothing to explain why. Nothing after this re-opens the dialog.
        _neowxPopupActive = false;
        $(modal).off('shown.bs.modal.neowx');
        neowxRunTeardown();
        var mount = document.getElementById(NEOWX_POPUP_ID + '-mount');
        if (mount) {
            mount.innerHTML = '';
        }
        // In "align" mode the chart's toolbar was moved into the header, which
        // is outside the mount - so emptying that does not take it with it, and
        // it would still be sitting there over the next pop-up's title.
        var hosted = modal.querySelector('.modal-header > .apexcharts-toolbar');
        if (hosted) {
            neowxUnobserveCorner(hosted);
            hosted.remove();
        }
        // The next pop-up may come from a card aligned differently.
        modal.removeAttribute('data-item-title-align');
        modal.style.removeProperty('--nwm-item-title-reserve');
    });

    return modal;
}

// onShown is handed the (empty) mount point once the dialog is on screen, and
// onHidden - if given - runs when it closes, before the mount is emptied.
// ownClose says the content brings its own close button (the chart pop-up puts
// one in the chart toolbar, which lands where the header's would be), so the
// header's is taken out of the way for as long as that content is up.
function neowxOpenPopup(title, onShown, onHidden, ownClose, card) {
    if (_neowxPopupActive) {
        return;
    }
    _neowxPopupActive = true;
    _neowxPopupTeardown = onHidden || null;

    try {
        var modal = neowxPopupModal();
        // One modal serves both kinds of pop-up, so this is set on every open
        // rather than only when it is wanted.
        modal.classList.toggle('nwm-popup-own-close', !!ownClose);
        document.getElementById(NEOWX_POPUP_ID + '-title').textContent = title;
        // A pop-up is the card enlarged, so its title is aligned the way that
        // card's is. The room to leave is measured from the dialog's own corner
        // once it is on screen and the toolbar has landed - see the shown
        // handler below.
        modal.setAttribute('data-item-title-align', neowxCardItemTitleAlign(card));
        // Seeded from the card's own measurement, because the dialog's cannot
        // be taken yet: in "align" mode the toolbar it measures against does
        // not exist until the chart has rendered, which is after the dialog is
        // on screen. Without this the title is laid out against the 3rem
        // fallback and then visibly jumps when the real figure arrives. The two
        // controls hold the same buttons give or take the expand one, so the
        // seed is close enough that the correction does not show.
        if (card) {
            var seed = card.style.getPropertyValue('--nwm-item-title-reserve');
            if (seed) {
                modal.style.setProperty('--nwm-item-title-reserve', seed);
            }
        }

        // Fill the dialog only once it is on screen. Nothing inside a modal has
        // a size while Bootstrap still has it at display:none, and with .fade
        // that is not swapped inside modal('show') - the backdrop transition
        // runs first - so a chart or an iframe would measure its container as
        // zero and render collapsed.
        $(modal).one('shown.bs.modal.neowx', function () {
            try {
                onShown(document.getElementById(NEOWX_POPUP_ID + '-mount'));
            } catch (e) {
                // The header's close button was taken out of the way on the
                // promise that the content would bring its own. It cannot now,
                // so give it back rather than leave a dialog with no visible
                // way out of it.
                modal.classList.remove('nwm-popup-own-close');
                console.error('neowx-material: could not fill the pop-up window', e);
            }
            neowxSetPopupTitleReserve(modal);
        });
        $(modal).modal('show');
    } catch (e) {
        // The dialog never opened, so hidden.bs.modal will not fire to release
        // the guard.
        _neowxPopupActive = false;
        _neowxPopupTeardown = null;
        console.error('neowx-material: could not open the pop-up window', e);
    }
}

// The card heading is the only title either kind of pop-up has to work with.
function neowxCardTitle(el) {
    var card = el && el.closest ? el.closest('.card') : null;
    var heading = card ? card.querySelector('h5') : null;
    return heading ? heading.textContent.trim() : '';
}

// --- Charts ---------------------------------------------------------------

var _neowxPopupChart = null;

// Deep copy that keeps functions (axis formatters, chart event handlers) by
// reference. A JSON round-trip would drop them, and handing the new chart the
// source chart's own sub-objects would let ApexCharts mutate the card chart
// while normalising the copy.
function neowxCloneChartConfig(value, seen) {
    if (value === null || typeof value !== 'object') {
        return value;
    }
    if (value instanceof Date) {
        return new Date(value.getTime());
    }
    seen = seen || new WeakMap();
    if (seen.has(value)) {
        return seen.get(value);
    }
    var copy = Array.isArray(value) ? [] : {};
    seen.set(value, copy);
    for (var key in value) {
        if (Object.prototype.hasOwnProperty.call(value, key)) {
            copy[key] = neowxCloneChartConfig(value[key], seen);
        }
    }
    return copy;
}

// The chart config carries no title, so reuse the card heading. Every chart in
// the skin sits in a .card with an <h5>, telemetry included, so the series-name
// path below is a safety net for a chart mounted outside one.
function neowxPopupTitle(chartCtx) {
    var heading = neowxCardTitle(chartCtx.el);
    if (heading !== '') {
        return heading;
    }
    var series = chartCtx.w && chartCtx.w.config ? chartCtx.w.config.series : null;
    var names = Array.isArray(series) ? series.map(function (s) {
        return s && s.name;
    }).filter(Boolean) : [];
    return names.length > 0 ? names.join(' & ') : '';
}

function neowxChartPopup(chartCtx) {
    var config = neowxCloneChartConfig(chartCtx.w.config);
    // height '100%' measures the mount point's PARENT, the modal body, which
    // the stylesheet gives a viewport-relative height. width '100%' measures
    // the mount itself.
    config.chart.height = '100%';
    config.chart.width = '100%';
    // No chart in the skin sets either today. If one ever did, a duplicate id
    // would make ApexCharts.exec() resolve to whichever instance registered
    // first - the card chart, not the copy - and a duplicate group would pull
    // the copy into the card chart's zoom and hover syncing.
    delete config.chart.id;
    delete config.chart.group;
    // In "align" mode the toolbar lands on the dialog's header row, where the
    // header's own close button is, so the close moves into the toolbar instead.
    // The other modes leave the toolbar in the chart and keep the header's.
    var toolbarCloses = neowxChartToolbarMode() === 'align';
    if (config.chart.toolbar) {
        if (config.chart.toolbar.tools && !toolbarCloses) {
            // No expand button on the chart that is already expanded.
            config.chart.toolbar.tools.customIcons = [];
        } else if (config.chart.toolbar.tools) {
            config.chart.toolbar.tools.customIcons = [{
                icon: NEOWX_CLOSE_ICON,
                title: neowxText('close', 'Close'),
                class: 'neowx-chart-close-icon',
                click: function () {
                    $('#' + NEOWX_POPUP_ID).modal('hide');
                }
            }];
        }
        // ApexCharts turns these into style.top/right on the toolbar when it
        // builds it. "align" re-parents the toolbar and clears both, so pin
        // them at the vendor's own default here rather than let an offset set
        // on the card's chart follow the copy into a dialog it was not
        // measured against.
        config.chart.toolbar.offsetX = 0;
        config.chart.toolbar.offsetY = 0;
    }

    var chartCard = chartCtx.el && chartCtx.el.closest ? chartCtx.el.closest('.card') : null;
    neowxOpenPopup(neowxPopupTitle(chartCtx), function (mount) {
        // The copy inherits chart.events.mounted by reference. On a customChart*
        // using hidevalues that is the handler chart_hidden_series.inc builds,
        // so the copy re-hides those series even when they were un-hidden on
        // the card.
        _neowxPopupChart = new ApexCharts(mount, config);
        var rendering = _neowxPopupChart.render();
        if (rendering && rendering.then) {
            rendering.then(null, function (e) {
                console.error('neowx-material: the expanded chart could not be drawn', e);
            });
        }
    }, function () {
        var chart = _neowxPopupChart;
        _neowxPopupChart = null;
        if (!chart) {
            return;
        }
        // NEOWX_ALIGNED lives in js.inc and is filled by neowxMountedHandler,
        // which the copy inherits along with the cloned config. Without this an
        // align-mode chart leaves an entry behind on every open.
        if (typeof NEOWX_ALIGNED !== 'undefined' && chart.w && chart.w.globals) {
            delete NEOWX_ALIGNED[chart.w.globals.chartID];
        }
        chart.destroy();
    }, toolbarCloses, chartCard);
}

// --- Embedded iFrames and images ------------------------------------------

// Copies the card's own iframe/img rather than rebuilding one from the config,
// so the src (including the cache-busting timestamp on images) and the alt text
// come along for free. Any link around the image is left on the card.
function neowxEmbedPopup(button) {
    var card = button.closest('.card');
    // Scoped to the card body so a decorative image added to a card header
    // later would not be expanded in place of the embed itself.
    var media = card ? card.querySelector('.card-body iframe, .card-body img') : null;
    if (!media) {
        // The button is visible and focusable, so returning quietly here would
        // leave a dead control with nothing to diagnose it by.
        console.error('neowx-material: expand button found no iframe or image to ' +
            'show - has the card markup changed?', button);
        return;
    }

    var copy = media.cloneNode(true);
    // On the card the media is a flex child pinned to a fixed aspect ratio; in
    // the dialog the stylesheet sizes it instead.
    copy.classList.remove('flex-grow-1', 'w-100');
    copy.classList.add('nwm-embed-popup-media');
    copy.style.removeProperty('aspect-ratio');

    var title = neowxCardTitle(button) || media.getAttribute('alt') || '';
    neowxOpenPopup(title, function (mount) {
        mount.appendChild(copy);
    }, null, false, card);
}

// Registered unconditionally: app.js is a plain .js file with no access to the
// skin config, and with enableEmbeddedPopup off the templates emit no buttons
// for this to match.
document.addEventListener('click', function (e) {
    var button = e.target.closest ? e.target.closest('[data-nwm-embed-popup]') : null;
    if (button) {
        e.preventDefault();
        neowxEmbedPopup(button);
    }
});

// --- CHART TOOLBAR POSITION -----------------------------------------------
// Appearance/chart_toolbar in skin.conf decides where a chart's buttons go:
//
//   chart_default - ApexCharts' own placement, inside the chart below the
//                   heading
//   separate      - only the expand button moves, into the card's top-right
//                   corner, the same spot the [[Embedded]] cards put theirs
//   align         - the whole toolbar moves into that corner
//
// js.inc normalises the setting and hands it over as NEOWX_CHART_TOOLBAR, and
// calls neowxSetupChartToolbar from the chart's mounted and updated events.

// Shared with js.inc, which uses it for the toolbar's own expand button.
var NEOWX_EXPAND_ICON =
    '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" ' +
    'fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" ' +
    'stroke-linejoin="round"><path d="M15 3h6v6"/><path d="M9 21H3v-6"/>' +
    '<path d="M21 3l-7 7"/><path d="M3 21l7-7"/></svg>';

var NEOWX_CLOSE_ICON =
    '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" ' +
    'fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" ' +
    'stroke-linejoin="round"><path d="M18 6L6 18"/><path d="M6 6l12 12"/></svg>';

// The strings come from js.inc, which is emitted after this file is loaded, so
// they can only be read at click or mount time - never at the top level here.
function neowxText(key, fallback) {
    return (window.NEOWX_TEXTS && window.NEOWX_TEXTS[key]) || fallback;
}

function neowxChartToolbarMode() {
    var mode = window.NEOWX_CHART_TOOLBAR;
    return (mode === 'separate' || mode === 'align') ? mode : 'chart_default';
}

function neowxSetupChartToolbar(chartContext) {
    var mode = neowxChartToolbarMode();
    if (mode === 'align') {
        neowxAlignChartToolbar(chartContext);
    } else if (mode === 'separate') {
        neowxSeparateExpandButton(chartContext);
    }
}

// --- separate -------------------------------------------------------------
// The card gets the [[Embedded]] cards' own toolbar markup, so both kinds of
// card end up with the same button in the same place, from the same stylesheet
// rules. Built here rather than in the templates because a chart card is
// emitted from eight of them.
function neowxSeparateExpandButton(chartContext) {
    try {
        var el = chartContext && chartContext.el;
        var card = el && el.closest ? el.closest('.card') : null;
        // The pop-up's chart is not in a card, and is already expanded.
        if (!card) {
            return;
        }
        // 'updated' fires on every re-render, so this must not stack up. Scoped
        // to a direct child, as neowxCardCorner is: a match nested deeper is not
        // the one that would be measured, so treating it as "already done" would
        // leave a card with a button and no room reserved for it.
        if (card.querySelector(':scope > .nwm-embed-toolbar')) {
            return;
        }

        var label = neowxText('expand_chart', 'Expand chart');
        var toolbar = document.createElement('div');
        toolbar.className = 'nwm-embed-toolbar';
        var button = document.createElement('button');
        button.type = 'button';
        button.className = 'nwm-embed-toolbar-icon';
        button.title = label;
        button.setAttribute('aria-label', label);
        button.innerHTML = NEOWX_EXPAND_ICON;
        button.addEventListener('click', function () {
            neowxChartPopup(chartContext);
        });
        toolbar.appendChild(button);
        // A direct child of .card, which is what the stylesheet positions it
        // against - the templates place the [[Embedded]] one the same way.
        card.appendChild(toolbar);
        neowxObserveCorner(toolbar);
        neowxSetTitleSpacing(card);
    } catch (e) {
        console.error('neowx-material: could not add the chart expand button', e);
    }
}

// ApexCharts closes its export menu from a document-level click handler that
// looks for .apexcharts-menu inside the chart's own element. A hosted toolbar
// takes the menu with it, out of where that handler looks, so clicking the
// chart - or picking an item off the menu - would leave it hanging open with
// only the hamburger able to shut it. This does that job for menus that moved.
//
// Registered unconditionally: with chart_toolbar anything but "align" nothing
// carries .nwm-hosted-toolbar and this matches nothing.
document.addEventListener('click', function (e) {
    var open = document.querySelectorAll(
        '.nwm-hosted-toolbar .apexcharts-menu.apexcharts-menu-open');
    if (open.length === 0) {
        return;
    }
    // The hamburger toggles the menu itself; closing it here as well would
    // undo the opening click. Every other click closes, which is the rule
    // ApexCharts applies to the menus it can still see.
    if (e.target.closest && e.target.closest('.apexcharts-menu-icon')) {
        return;
    }
    for (var i = 0; i < open.length; i++) {
        open[i].classList.remove('apexcharts-menu-open');
    }
});

// --- align ----------------------------------------------------------------
// ApexCharts anchors its toolbar to the chart canvas, which on a card sits
// below the heading. Its offsetX/offsetY settings can shift it from there, but
// only in pixels measured against a canvas that resizes, re-renders and is
// mid-transition inside a dialog - which never stayed reliably on the same spot
// as the button it is meant to line up with.
//
// So the toolbar is moved instead: into the card, or into the dialog's header,
// where the very same stylesheet rule that places the [[Embedded]] cards'
// button places it. Being the same rule, the two cannot drift apart.
//
// The catch is that ApexCharts builds a fresh toolbar inside the canvas
// whenever it re-renders - the zoom handler in js.inc calls updateOptions on
// every zoom - so this runs on 'updated' as well as at mount, and takes the
// stale one away when it does.
function neowxAlignChartToolbar(chartContext) {
    try {
        var el = chartContext && chartContext.el;
        if (!el || !el.closest) {
            return;
        }
        // In the pop-up the header is the counterpart of the card: it is what
        // carries the dialog's own close button.
        var content = el.closest('.modal-content');
        var host = el.closest('.card') || (content && content.querySelector('.modal-header'));
        if (!host) {
            return;
        }
        // Only a toolbar still inside the chart is a new one. Once hosted it is
        // no longer in there, and a re-render that produced no new toolbar
        // must leave the hosted one alone.
        var toolbar = el.querySelector('.apexcharts-toolbar');
        if (!toolbar) {
            return;
        }
        var hosted = host.querySelector(':scope > .apexcharts-toolbar');
        // ApexCharts sets these from chart.toolbar.offsetX/offsetY; they are
        // measured against the canvas, which is no longer what it sits in.
        toolbar.style.top = '';
        toolbar.style.right = '';
        // Out here the vendor's theme classes on the canvas no longer reach it,
        // so the stylesheet dresses .nwm-hosted-toolbar from the skin's own
        // theme instead - the same rules that dress the [[Embedded]] button.
        // Reading the canvas class here would be no good anyway: ApexCharts has
        // not always set it by the time a card chart reports mounted.
        toolbar.classList.add('nwm-hosted-toolbar');
        host.appendChild(toolbar);
        // Appended before the old one is taken away: if appending throws, the
        // card keeps the toolbar it had rather than ending up with none.
        if (hosted && hosted !== toolbar) {
            // Unobserved first: ApexCharts builds a fresh toolbar on every
            // re-render, so without this the observer collects a detached one
            // per zoom for the life of the page.
            neowxUnobserveCorner(hosted);
            hosted.remove();
        }
        // Whatever now hosts it may have to leave the heading room for it. In
        // the dialog this is the only chance to measure: the pop-up asks for it
        // as soon as it is shown, which is before render() has fired mounted
        // and put a toolbar here at all.
        if (host.classList.contains('card')) {
            neowxObserveCorner(toolbar);
            neowxSetTitleSpacing(host);
        } else {
            neowxSetPopupTitleReserve(document.getElementById(NEOWX_POPUP_ID));
        }
    } catch (e) {
        // Nothing here is load-bearing - a chart whose toolbar stayed where
        // ApexCharts put it is still a working chart.
        console.error('neowx-material: could not place the chart toolbar', e);
    }
}

// --- CARD HEADING ALIGNMENT -----------------------------------------------
// Appearance/items_title_align picks how a card's heading sits: center (as it always
// has), left, or center-adjusted - centred in what is left once the card's
// expand button or toolbar is allowed for. head.inc emits the alignment as CSS,
// which is all "center" needs.
//
// "left" and "center-adjusted" also need to know how much room that control
// takes, and that is not a number the templates can know: it is a 32 px
// one-button toolbar on an [[Embedded]] card or a chart in "separate" mode, a
// whole toolbar in "align" mode, and nothing at all on a value card, on a
// chart in "chart_default" mode, or wherever the pop-up buttons are switched
// off. So it is measured from whatever is actually in the corner and written to
// the card as --nwm-item-title-reserve; "left" additionally needs
// --nwm-item-title-inset, since the card body's own left padding is not the
// same on every kind of card.

// Whether any card asks for an alignment other than the default. head.inc
// emits nothing at all for a site that leaves items_title_align alone, so
// without this the whole measuring apparatus below - two rects and two
// getComputedStyle per card, on load, on resize, and a ResizeObserver on every
// card and body for the life of the page - would run to feed CSS that does not
// exist. Set by head.inc, so read lazily: this file loads first.
function neowxItemTitleAlignInUse() {
    return window.NEOWX_ITEM_TITLE_ALIGN_SET === true;
}

// Gap between the heading and the control, so the two never quite touch.
var NEOWX_TITLE_GUTTER = 8;

// How far a left-aligned heading sits from the card's edge. The card body's
// own padding is not enough on its own and is not even the same everywhere -
// half a rem on most cards, none at all on the [[Embedded]] ones, whose body
// is px-0 so the media can run to the edge - which would leave their headings
// flush against it. This is the dialog header's padding, so a heading starts
// on the same line whether it is on a card or in the pop-up that card opens.
var NEOWX_TITLE_INSET = 16;

// The control a card carries in its top-right corner, if it has one at all:
// the [[Embedded]] cards' expand button, the one app.js adds to a chart card in
// "separate" mode, or a toolbar hosted there in "align" mode.
function neowxCardCorner(card) {
    return card.querySelector(':scope > .nwm-embed-toolbar, :scope > .nwm-hosted-toolbar');
}

function neowxSetTitleSpacing(card) {
    try {
        if (!card || !card.querySelector || !neowxItemTitleAlignInUse()) {
            return;
        }
        var body = card.querySelector(':scope > .card-body');
        var heading = body ? body.querySelector(':scope > .h5-responsive') : null;
        if (!heading) {
            return;
        }
        var bodyBox = body.getBoundingClientRect();
        if (bodyBox.width === 0) {
            return;
        }
        var bodyStyle = window.getComputedStyle(body);

        // Left-aligned headings start on NEOWX_TITLE_INSET from the card's
        // edge, whatever the card body contributes towards it. Taken off the
        // heading's own box, which is where the text starts from and is not
        // affected by the padding being set here - reading the body's padding
        // instead would miss that an [[Embedded]] card's body is not only
        // unpadded but sits slightly inside the card.
        var cardBox = card.getBoundingClientRect();
        var headingLeft = heading.getBoundingClientRect().left;
        var inset = Math.max(0, NEOWX_TITLE_INSET - (headingLeft - cardBox.left));
        card.style.setProperty('--nwm-item-title-inset', inset + 'px');

        var corner = neowxCardCorner(card);
        // No control, or one that is laid out but not shown: nothing to leave
        // room for, and center-adjusted then behaves as center.
        if (!corner || window.getComputedStyle(corner).display === 'none') {
            card.style.removeProperty('--nwm-item-title-reserve');
            return;
        }
        var cornerBox = corner.getBoundingClientRect();
        // A control that measures zero is one that is not laid out yet, which is
        // the same thing as not having one as far as this pass is concerned -
        // and it has to clear the property for the same reason, or a value
        // measured against some earlier control survives a pass that is
        // supposed to recompute from scratch.
        if (cornerBox.width === 0) {
            card.style.removeProperty('--nwm-item-title-reserve');
            return;
        }
        // Measured against the card body's content edge rather than the
        // heading's own, which already carries whatever reserve was set last
        // time and would creep outwards on every pass.
        var contentRight = bodyBox.right - (parseFloat(bodyStyle.paddingRight) || 0);
        var reserve = Math.max(0, contentRight - cornerBox.left + NEOWX_TITLE_GUTTER);
        card.style.setProperty('--nwm-item-title-reserve', reserve + 'px');
    } catch (e) {
        console.error('neowx-material: could not measure the heading reserve', e);
    }
}

// What head.inc resolved for this card, from the custom property it set there.
// A pop-up opened from a card with no alignment of its own - or from nothing at
// all - centres, which is the skin's default.
function neowxCardItemTitleAlign(card) {
    if (!card) {
        return 'center';
    }
    var align = window.getComputedStyle(card).getPropertyValue('--nwm-item-title-align').trim();
    return (align === 'left' || align === 'center-adjusted') ? align : 'center';
}

// The dialog's counterpart of neowxSetTitleSpacing. What sits in its corner is
// either the chart toolbar - "align" mode hosts it on the header row - or the
// dialog's own close button, and both are in the header.
function neowxSetPopupTitleReserve(modal) {
    try {
        if (!modal) {
            return;
        }
        var align = modal.getAttribute('data-item-title-align');
        if (align !== 'left' && align !== 'center-adjusted') {
            return;
        }
        var header = modal.querySelector('.modal-header');
        var title = header ? header.querySelector('.modal-title') : null;
        // In precedence order, not as one selector list: querySelector returns
        // the first match in TREE order, and the close button is built into the
        // header while the toolbar is appended after it - so a list would always
        // answer with the close button, which "align" mode hides.
        var corner = header ? header.querySelector('.apexcharts-toolbar') : null;
        if (corner && window.getComputedStyle(corner).display === 'none') {
            corner = null;
        }
        if (!corner && header) {
            corner = header.querySelector('.nwm-popup-close');
        }
        if (!title || !corner || window.getComputedStyle(corner).display === 'none') {
            return;
        }
        var cornerBox = corner.getBoundingClientRect();
        var headerBox = header.getBoundingClientRect();
        if (cornerBox.width === 0 || headerBox.width === 0) {
            return;
        }
        var contentRight = headerBox.right -
            (parseFloat(window.getComputedStyle(header).paddingRight) || 0);
        var reserve = Math.max(0, contentRight - cornerBox.left + NEOWX_TITLE_GUTTER);
        modal.style.setProperty('--nwm-item-title-reserve', reserve + 'px');
    } catch (e) {
        console.error('neowx-material: could not measure the dialog heading reserve', e);
    }
}

// Every card on the page. Charts call neowxSetTitleSpacing themselves once
// their toolbar exists, which is after this has run for them.
function neowxSetTitleSpacings() {
    var cards = document.querySelectorAll('.card');
    for (var i = 0; i < cards.length; i++) {
        neowxSetTitleSpacing(cards[i]);
    }
}

// js.inc loads this file at the end of the page, so the cards are already
// there and the first pass can just run. The later hooks are for anything that
// changes the measurement afterwards: a card still parsing when this ran, web
// fonts settling on load, and a resize, which can change the card body's own
// padding at a breakpoint. All idempotent - each pass recomputes from scratch.
neowxSetTitleSpacings();

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', neowxSetTitleSpacings);
}

window.addEventListener('load', neowxSetTitleSpacings);

var _neowxTitleResizeTimer = null;
window.addEventListener('resize', function () {
    clearTimeout(_neowxTitleResizeTimer);
    _neowxTitleResizeTimer = setTimeout(function () {
        neowxSetTitleSpacings();
        // An open pop-up's title measures against the dialog, which the resize
        // moved as well.
        neowxSetPopupTitleReserve(document.getElementById(NEOWX_POPUP_ID));
    }, 250);
});

// A card's own box can move without the window changing at all: an
// [[Embedded]] card's image arrives and lays its body out differently, a chart
// finishes rendering, a panel is expanded. The measurement comes off those
// boxes, so watch them rather than hope the first pass caught them settled.
var _neowxTitleObserver = null;

// The control is watched too, and it is the one that cannot be picked up in the
// sweep below: a chart's is built in JavaScript long after this runs, and its
// width changes again the first time a zoom adds the reset button. Called by
// whichever function put it there.
function neowxObserveCorner(corner) {
    if (_neowxTitleObserver && corner) {
        try {
            _neowxTitleObserver.observe(corner);
        } catch (e) {
            console.error('neowx-material: could not watch a card control', e);
        }
    }
}

function neowxUnobserveCorner(corner) {
    if (_neowxTitleObserver && corner) {
        try {
            _neowxTitleObserver.unobserve(corner);
        } catch (e) {
            console.error('neowx-material: could not stop watching a card control', e);
        }
    }
}

if (typeof ResizeObserver !== 'undefined' && neowxItemTitleAlignInUse()) {
    _neowxTitleObserver = new ResizeObserver(function (entries) {
        for (var i = 0; i < entries.length; i++) {
            var target = entries[i].target;
            var card = target.classList.contains('card')
                ? target
                : (target.closest ? target.closest('.card') : null);
            neowxSetTitleSpacing(card);
        }
    });
    // The body as well as the card: an image arriving can move the body inside
    // a card whose own box never changes.
    var _neowxWatched = document.querySelectorAll(
        '.card, .card > .card-body, .card > .nwm-embed-toolbar');
    for (var _c = 0; _c < _neowxWatched.length; _c++) {
        try {
            _neowxTitleObserver.observe(_neowxWatched[_c]);
        } catch (e) {
            // One element failing must not leave the rest unwatched.
            console.error('neowx-material: could not watch a card', e);
        }
    }
}