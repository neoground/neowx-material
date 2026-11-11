#
# Copyright (c) 2026 PanelOrder Extension for NeoWX Material
#
# Distributed under the terms of the GNU GENERAL PUBLIC LICENSE
#

"""Parses the skin's page layout out of skin.conf so the templates don't have to.

Every page has to answer the same question: which cards and charts appear here,
in what order, and which of them are grouped into a collapsible panel?  The
answer lives in skin.conf, and getting it out takes more work than it looks.
configobj returns a list when a value contains commas and a bare string when it
doesn't.  Sections have to be filtered down to the part of the site being
rendered.  An item named twice has to be resolved rather than drawn twice.

Cheetah gives you nowhere good to put that.  A #def is private to the template
that declares it, and an #include does not export its defs to the file that
included it, so any parsing helper written in a template can only spread by
copy-paste.  That is what this module replaced: the same marker-parsing code
inlined into all eight page templates, kept in step by hand.  A SearchList gives
every page one parser, and a layout bug one place to be fixed.

Layout is entirely in this module's hands.  When parse_sections returns nothing,
the page renders no cards and no charts, which is the intended outcome for a
config that was never migrated off the old *_order settings (reported once by
_report_unmigrated).  The side effect is that a parsing bug here surfaces as a
blank page rather than a traceback, so prefer degrading with a warning over
raising.

Layout lives in [Extras][[Appearance]][[[sections]]].  Each section names its
items and, optionally, how to present them:

    [[[[soil]]]]
        items = soilMoist1, soilMoist2
        title = Soil Moisture
        collapsed = true
        content = card

  items      ordered list of cards/charts.  Required.
  title      omit for no panel - the items render loose in the surrounding row
             and 'collapsed' is ignored.
  collapsed  true  -> panel, starts collapsed
             false -> panel, starts expanded (the default)
             none  -> panel chrome, always open, no toggle
  content    card (default), chart, embedded, telemetry, telemetry_chart

Which sections a page shows, and in what order, lives in
[Extras][[Appearance]][[[pages]]]:

    [[[[today]]]]
        sections = cards_with_forecast, additional_cards, charts

    [[[[yesterday]]]]
        sections = cards_without_forecast, additional_cards, charts

  Keys are the $page values the templates carry: today (index), yesterday,
  week, month (this month and the archives), year (likewise), telemetry.
  [[[pages]]] is mandatory: every page must be explicitly listed with the
  sections it displays.  Sections not listed do not appear on that page;
  there is no implicit fallback to all declared sections.  Writing
  'sections =' with nothing after it means this page deliberately shows
  nothing.  Order sorts WITHIN a content region - cards and charts are
  separate columns, so a mixed list does not interleave them.

Two of the page values are shared by two templates apiece: 'month' covers this
month and the archived month pages, and 'year' covers this year and the
archived year pages. Each such key may carry sub-blocks that override it for
one template only:

    [[[[month]]]]
        sections = cards, charts
        [[[[[month_archive]]]]]
            sections = cards, archive_charts

  month  -> month, month_archive
  year   -> year, year_archive
  today, yesterday, week and telemetry take no sub-blocks (one template each) -
  so [[[[[week]]]]] is never consulted even if someone writes it.

Settings resolve sub-block, then page block, for settings per setting
independently.

configobj folds a plain setting written below a sub-block into that
sub-block, where it silently stops working. _warn_page_blocks() can only
catch this when the absorbing block's name is not a valid sub-page name;
absorption into a valid sub-block is indistinguishable, after parsing, from a
setting deliberately written inside it, so nothing can warn about that case.
skin.conf's [[[pages]]] comments are where this is explained in full.

Sections render in declaration order within each content value.  Items are
de-duplicated within a section, first occurrence winning - the same item may
appear in as many sections as you like and renders once in each.  'forecast' is
the exception: one per page, later occurrences dropped.  The guard is scoped to
a single parse_sections() call, i.e. per content value - which is enough
because 'forecast' only has a render path under content = card.
Each segment also carries a 'slug': the section name reduced to [A-Za-z0-9_-]
and made unique across every section, which the templates emit as data-section.

Registration is not optional.  Without this, the templates call names that were
never added to the search list and the pages fail to generate:

    [CheetahGenerator]
        search_list_extensions = user.panelorder.PanelOrder

Then, in a template that has declared #attr $page (and, where relevant,
#attr $subpage):

    #set $segments   = $panelSegments('card', page=$page)
    #set $flat       = $panelItems('chart', page=$page)
    #set $isBattery  = $isTelemetryItem('outTempBatteryStatus')
    #set $isEmbed    = $isEmbeddedItem('iFrame1')

panelSegments carries the grouping and is what you loop over to draw a row or a
panel.  panelItems flattens the same data to bare names, for the places that
only need to know whether an item is present, such as the chart JavaScript.
isTelemetryItem classifies a single item name so a template can pick its
rendering: true if the name has a [[[<name>]]] block under [[Telemetry]], or
if it appears in any content = telemetry / telemetry_chart section's items on
any page - either signal is enough, and neither is page-scoped.
isEmbeddedItem classifies a single item name: true if the name has a
[[[<name>]]] block under [[Embedded]], or starts with 'iFrame' or 'image'
(case-insensitive).
"""

import logging
import weakref

from weewx.cheetahgenerator import SearchList

log = logging.getLogger(__name__)

VERSION = "2.4.0"

# Segment types.  'type' is None for a section with no title, whose items
# render loose rather than inside a panel.
EXPANDED = "expanded"
COLLAPSED = "collapsed"
STATIC = "static"

CARD = "card"
CONTENTS = (CARD, "chart", "telemetry", "telemetry_chart")

# Items that render at most once per page however many sections list them.
# The forecast is one large card with fixed inner element ids
# (forecast-table, forecast_hour_icons) and its own #include-plus-#set global
# handoff; a second copy would duplicate those ids and the two would fight
# over the globals.  Everything else may now repeat once per section.
SINGLETON_ITEMS = ("forecast",)

# Keys a section may carry that are settings rather than typos.
# items_title_align is read by inc/head.inc, not here - it is listed so a panel
# that sets it is not reported as carrying an unknown setting.
SETTING_KEYS = ("items", "title", "collapsed", "content", "items_title_align")

# Sub-page override levels inside [[[pages]]].  A page key absent from this
# map takes no sub-blocks at all, which is how 'today', 'yesterday', 'week',
# and 'telemetry' - one template each - are expressed without a special case.
#
# Note that a template ALWAYS declares a $subpage name, even those; only
# the config sub-block is restricted.  The two ideas are separate and
# conflating them is the easy mistake here.
SUBPAGES = {
    "month": ("month", "month_archive"),
    "year": ("year", "year_archive"),
}

# Pages that do not support sections or panels by design. When unconfigured in
# [[[pages]]], they return an empty section list without error.
NON_PANEL_PAGES = ("almanac", "archive", "history")

# Keys valid in a [[[[page]]]] or [[[[[subpage]]]]] block.
PAGE_SETTING_KEYS = ("sections",)

# Order settings from 1.68.x.  Only used to recognise an unmigrated config.
LEGACY_KEYS = (
    "values_order",
    "charts_order",
    "embedded_order",
    "telemetry_order",
    "telemetry_chart_order",
)

TRUE_WORDS = ("true", "yes", "1")
FALSE_WORDS = ("false", "no", "0")

# The unmigrated-config error, and the per-section parse warnings below, are
# worth reporting once per config, not once per call.  A set of raw id()
# values would grow without bound across report cycles and, worse, CPython
# can recycle a freed object's address, wrongly suppressing the diagnostic
# for an unrelated config that happens to land at the same address.  Every
# id() stored below is paired with a weakref.finalize() callback that
# discards it the moment the config object it names is actually collected,
# so storage stays bounded to the configs currently alive and a recycled
# id() always starts out "not yet reported".
_legacy_reported = set()


def _remember(seen_set, obj):
    """Record obj (by identity) in seen_set, auto-forgetting it once obj is
    garbage collected."""
    key = id(obj)
    if key not in seen_set:
        seen_set.add(key)
        weakref.finalize(obj, seen_set.discard, key)


def _already_seen(seen_set, obj):
    return id(obj) in seen_set


# Per-section parse warnings (unknown content/collapsed/setting-key values)
# are memoised the same way, but scoped per problem so a config with several
# distinct issues still reports each of them once.  Bucket is
# id(appearance) -> set of (kind, section_id, detail) tuples already logged.
_warned_problems = {}

# parse_sections() itself is memoised per (appearance identity, content,
# enable_panels, page) so the ~40-60 calls a single template can make for
# one content region (chart JS generation is called from inside nested
# loops) collapse to one real parse per report cycle.  Bucket is
# id(appearance) -> {(content, enable_panels, page): segments}.
_sections_cache = {}

# parse_sections() memoises per (content, enable_panels, page); the slug map
# is none of those - it's the same for every page - so it gets its own key
# inside the same weakref-bounded bucket.
_SLUG_KEY = ("__slugs__",)


def _problem_seen(appearance, problem):
    bucket = _warned_problems.get(id(appearance))
    return bucket is not None and problem in bucket


def _mark_problem(appearance, problem):
    appearance_id = id(appearance)
    bucket = _warned_problems.get(appearance_id)
    if bucket is None:
        bucket = set()
        _warned_problems[appearance_id] = bucket
        weakref.finalize(appearance, _warned_problems.pop, appearance_id, None)
    bucket.add(problem)


def _cache_get(appearance, key):
    bucket = _sections_cache.get(id(appearance))
    return None if bucket is None else bucket.get(key)


def _cache_set(appearance, key, value):
    appearance_id = id(appearance)
    bucket = _sections_cache.get(appearance_id)
    if bucket is None:
        bucket = {}
        _sections_cache[appearance_id] = bucket
        weakref.finalize(appearance, _sections_cache.pop, appearance_id, None)
    bucket[key] = value


def _as_list(value):
    """Normalise a configobj value to a list of non-empty strings.

    configobj yields a list when the value contained commas and a bare string
    when it did not; an unset value comes through as None or "".
    """
    if value is None:
        return []
    if isinstance(value, (list, tuple)):
        parts = list(value)
    else:
        parts = str(value).split(",")
    return [str(p).strip() for p in parts if str(p).strip()]


def _appearance(skin_dict):
    return skin_dict.get("Extras", {}).get("Appearance", {})


def _slug_char_ok(ch):
    """ASCII-only test.  str.isalnum() passes 'e-acute' and other non-ASCII
    letters, which would then appear raw in an attribute selector."""
    return ("a" <= ch <= "z") or ("A" <= ch <= "Z") or ("0" <= ch <= "9") or ch in "_-"


def _slugs(appearance, sections):
    """Map every section id to a unique slug for use as an attribute value.

    Computed over ALL sections, not per content region: one page renders the
    card, chart and embedded regions into a single document, so two sections
    with different content values can still collide there.

    Case is preserved.  The slug goes verbatim into a CSS attribute selector,
    which is case sensitive, so a user writing [data-section="soilCharts"]
    must get back the name they wrote in skin.conf.
    """
    cached = _cache_get(appearance, _SLUG_KEY)
    if cached is not None:
        return cached

    used = {}
    result = {}
    for idx, section_id in enumerate(
            getattr(sections, "sections", list(sections.keys()))):
        base = "".join(
            c if _slug_char_ok(c) else "-" for c in str(section_id).strip())
        while "--" in base:
            base = base.replace("--", "-")
        base = base.strip("-")
        if not base:
            base = "section-%d" % idx
        slug = base
        suffix = 2
        while slug in used:
            slug = "%s-%d" % (base, suffix)
            suffix += 1
        if slug != base:
            log.warning(
                "panelorder: sections '%s' and '%s' both reduce to the slug "
                "'%s'; using '%s' for the second one. Rename one of them to "
                "keep data-section values predictable.",
                used[base], section_id, base, slug)
        used[slug] = section_id
        result[section_id] = slug

    _cache_set(appearance, _SLUG_KEY, result)
    return result


def section_slug(skin_dict, section_id):
    """The data-section value for one section, by its skin.conf name.

    inc/head.inc builds its items_title_align rules straight from
    Extras.Appearance.sections rather than from parsed segments, and needs the
    same slug the templates emit.  Exposed so that rule lives in exactly one
    place: re-deriving it there would drop the collision suffix and the
    empty-name fallback, and emit CSS that silently matches nothing.
    """
    appearance = _appearance(skin_dict)
    sections = appearance.get("sections")
    if not sections:
        return ""
    return _slugs(appearance, sections).get(str(section_id), "")


def _report_unmigrated(appearance):
    """Complain once per config if this looks like an unmigrated 1.68.x setup."""
    if _already_seen(_legacy_reported, appearance):
        return
    stale = [k for k in LEGACY_KEYS if appearance.get(k) is not None]
    if stale:
        _remember(_legacy_reported, appearance)
        log.error(
            "panelorder: no [[[sections]]] found, but %s still present. "
            "These settings were replaced by [[[sections]]] and are no longer "
            "read, so no cards or charts will render. See the [[Appearance]] "
            "comments in skin.conf for the new syntax.",
            ", ".join(stale),
        )


def _panel_type(appearance, section, section_id):
    """Map a section's 'collapsed' setting to a segment type."""
    raw = section.get("collapsed")
    if raw is None:
        return EXPANDED
    word = str(raw).strip().lower()
    if word in TRUE_WORDS:
        return COLLAPSED
    if word in FALSE_WORDS:
        return EXPANDED
    if word == "none":
        return STATIC
    problem = ("collapsed", section_id, str(raw))
    if not _problem_seen(appearance, problem):
        _mark_problem(appearance, problem)
        log.warning(
            "panelorder: section '%s' has collapsed = %s, which is not "
            "true, false or none; treating it as false.",
            section_id,
            raw,
        )
    return EXPANDED


def _warn_unknown_keys(appearance, section, section_id):
    for key in getattr(section, "scalars", list(section.keys())):
        if key not in SETTING_KEYS:
            problem = ("key", section_id, key)
            if _problem_seen(appearance, problem):
                continue
            _mark_problem(appearance, problem)
            log.warning(
                "panelorder: section '%s' has unknown setting '%s'; ignoring "
                "it. Items belong in 'items'.",
                section_id,
                key,
            )


def enable_panels_setting(skin_dict):
    """Read [[Appearance]] enablePanels, defaulting to true."""
    raw = _appearance(skin_dict).get("enablePanels", "true")
    return str(raw).strip().lower() not in FALSE_WORDS


def _page_entries(appearance, page, subpage):
    """(page block, sub-block) for one template, either may be None.

    Guards the same two shapes _page_order already guards: a missing block,
    and a scalar written where a subsection belongs ("pages = today" instead
    of a [[[[day]]]] block).  Without the hasattr checks, 'in' below becomes a
    substring test and the indexing raises.
    """
    if page is None:
        return None, None
    pages = appearance.get("pages")
    if not pages or not hasattr(pages, "get"):
        return None, None
    key = str(page).strip()
    if key not in pages:
        if key == "today" and "current" in pages:
            key = "current"
        elif key == "current" and "today" in pages:
            key = "today"
        elif key == "day" and "yesterday" in pages:
            key = "yesterday"
        elif key == "day" and "today" in pages:
            key = "today"
        else:
            return None, None
    entry = pages[key]
    if not hasattr(entry, "get"):
        return None, None
    if subpage is None:
        return entry, None
    sub_key = str(subpage).strip()
    # Only look for a sub-block where one is actually valid.  'today', 'yesterday',
    # 'week' and 'telemetry' have no SUBPAGES entry, so sub-blocks are never consulted.
    if sub_key not in SUBPAGES.get(key, ()):
        return entry, None
    if sub_key not in entry:
        return entry, None
    sub = entry[sub_key]
    if not hasattr(sub, "get"):
        return entry, None
    return entry, sub


def _report_missing_pages(appearance):
    """Complain once per config if [[[pages]]] is missing from [[Appearance]]."""
    problem = ("missing-pages",)
    if not _problem_seen(appearance, problem):
        _mark_problem(appearance, problem)
        log.error(
            "panelorder: no [[[pages]]] found in [[Appearance]]. "
            "[[[pages]]] is mandatory; each page must explicitly define "
            "which sections to render. See skin.conf for the syntax."
        )


def page_setting(skin_dict, key, page=None, subpage=None):
    """Resolve one boolean page setting: sub-block, then page, then False."""
    appearance = _appearance(skin_dict)
    _warn_page_blocks(appearance, page)
    entry, sub = _page_entries(appearance, page, subpage)
    for block in (sub, entry):
        if block is None:
            continue
        raw = block.get(key)
        if raw is None:
            continue
        word = str(raw).strip().lower()
        if word in TRUE_WORDS:
            return True
        if word in FALSE_WORDS:
            return False
        problem = ("page-setting", str(page), str(subpage), key, str(raw))
        if not _problem_seen(appearance, problem):
            _mark_problem(appearance, problem)
            log.warning(
                "panelorder: %s = %s on page '%s' is not true or false; "
                "treating as false.",
                key,
                raw,
                subpage or page,
            )
        break
    return False


def _warn_page_blocks(appearance, page):
    """Diagnose a page block: bad sub-block names, absorbed settings, typos.

    configobj folds any plain setting written AFTER a subsection into that
    subsection.  At five levels deep that stops being an edge case - people
    naturally write:

        [[[[month]]]]
            [[[[[month_archive]]]]]
                sections = cards, archive_charts
            sections = cards, charts      # absorbed into month_archive, silently dead

    A recognised page-setting key found inside a subsection that is not a
    valid sub-page name IS that signature, so it gets named rather than left
    to be discovered on a live install.
    """
    if page is None:
        return
    pages = appearance.get("pages")
    if not pages or not hasattr(pages, "get"):
        return

    key = str(page).strip()
    if key not in pages:
        if key == "today" and "current" in pages:
            key = "current"
        elif key == "current" and "today" in pages:
            key = "today"
        elif key == "day" and "yesterday" in pages:
            key = "yesterday"
        elif key == "day" and "today" in pages:
            key = "today"
        else:
            return
    entry = pages[key]
    if not hasattr(entry, "get"):
        return
    valid = SUBPAGES.get(key, ())

    for name in getattr(entry, "sections", []):
        if name in valid:
            block = entry[name]
            for bad in getattr(block, "scalars", []):
                if bad in ("show_forecast", "show_embedded"):
                    problem = ("deprecated-" + bad, key, name)
                    if not _problem_seen(appearance, problem):
                        _mark_problem(appearance, problem)
                        log.warning(
                            "panelorder: '%s' setting in [[[[[%s]]]]] under page '%s' has been removed; "
                            "configure content via section items instead.",
                            bad, name, key,
                        )
                    continue
                if bad not in PAGE_SETTING_KEYS:
                    problem = ("subpage-key", key, name, bad)
                    if _problem_seen(appearance, problem):
                        continue
                    _mark_problem(appearance, problem)
                    log.warning(
                        "panelorder: [[[[[%s]]]]] under page '%s' has unknown "
                        "setting '%s'; ignoring it. Valid settings are %s.",
                        name, key, bad, ", ".join(PAGE_SETTING_KEYS),
                    )
            continue

        problem = ("subpage-name", key, name)
        if _problem_seen(appearance, problem):
            continue
        _mark_problem(appearance, problem)
        absorbed = [k for k in getattr(entry[name], "scalars", [])
                    if k in PAGE_SETTING_KEYS]
        if absorbed and valid:
            log.error(
                "panelorder: page '%s' has a subsection '[[[[[%s]]]]]' that "
                "is not a valid sub-page, and it contains %s. This is almost "
                "certainly a setting written BELOW a sub-page block - "
                "configobj folds it into that block, where it stops working. "
                "Move it ABOVE the sub-page blocks. Valid sub-pages for '%s' "
                "are: %s.",
                key, name, ", ".join(absorbed), key, ", ".join(valid),
            )
        else:
            log.warning(
                "panelorder: page '%s' has a subsection '[[[[[%s]]]]]' that "
                "is not a valid sub-page; ignoring it. Valid sub-pages for "
                "'%s' are: %s.",
                key, name, key, ", ".join(valid) if valid else "none",
            )

    for bad in getattr(entry, "scalars", []):
        if bad in ("show_forecast", "show_embedded"):
            problem = ("deprecated-" + bad, key)
            if not _problem_seen(appearance, problem):
                _mark_problem(appearance, problem)
                log.warning(
                    "panelorder: '%s' setting on page '%s' has been removed; "
                    "configure content via section items instead.",
                    bad, key,
                )
            continue
        if bad in PAGE_SETTING_KEYS:
            continue
        problem = ("page-key", key, bad)
        if _problem_seen(appearance, problem):
            continue
        _mark_problem(appearance, problem)
        log.warning(
            "panelorder: page '%s' has unknown setting '%s'; ignoring it. "
            "Valid settings are %s.",
            key, bad, ", ".join(PAGE_SETTING_KEYS),
        )


def _page_order(appearance, page, subpage=None):
    """Ordered section ids for one page, or None if no page was specified.

    None means 'no page specified' (unscoped/global call, e.g. from
    order_items(skin_dict, content)), allowing parse_sections to aggregate
    across all configured pages in [[[pages]]].

    For a specified page, returns the list of section ids configured in
    [[[pages]]] (sub-block first, then page block). If [[[pages]]] is missing
    or the page has no 'sections' configured, logs an error and returns []
    (showing nothing, with no implicit fallback).
    """
    if page is None:
        return None

    pages = appearance.get("pages")
    if not pages or not hasattr(pages, "get"):
        _report_missing_pages(appearance)
        return []

    key = str(page).strip()
    if key not in pages:
        if key == "today" and "current" in pages:
            key = "current"
        elif key == "current" and "today" in pages:
            key = "today"
        elif key == "day" and "yesterday" in pages:
            key = "yesterday"
        elif key == "day" and "today" in pages:
            key = "today"
        elif key in NON_PANEL_PAGES:
            return []
        else:
            problem = ("unconfigured-page", key)
            if not _problem_seen(appearance, problem):
                _mark_problem(appearance, problem)
                log.error(
                    "panelorder: page '%s' has no entry in [[[pages]]]. "
                    "Configure sections for this page in [[[pages]]].",
                    key,
                )
            return []

    entry, sub = _page_entries(appearance, page, subpage)
    for block in (sub, entry):
        if block is not None and "sections" in block:
            return _as_list(block.get("sections"))

    problem = ("unconfigured-page-sections", key, str(subpage))
    if not _problem_seen(appearance, problem):
        _mark_problem(appearance, problem)
        log.error(
            "panelorder: page '%s'%s has no 'sections' configured in [[[pages]]].",
            key,
            (" (subpage '%s')" % subpage) if subpage else "",
        )
    return []


def parse_sections(skin_dict, content=None, enable_panels=True, page=None,
                   subpage=None):
    """Return the layout segments for one content region or all sections.

    When 'content' is None or 'all', sections of all recognized content types
    are returned in the exact order listed in the page configuration.
    When 'content' is a specific type (e.g. 'card', 'chart'), only sections
    matching that content type are returned.

    With enable_panels false the grouping is discarded but every item is kept,
    so turning panels off degrades to a flat row rather than losing cards.

    'page' is one of the $page values the templates carry - day, week, month,
    year, telemetry.  When [[[pages]]] names that page, only the sections it
    lists are returned, in the order it lists them.  With [[[pages]]] mandatory,
    an unconfigured page returns [] rather than falling back to all sections.
    A global call (page=None) aggregates sections across all configured pages.

    Results are memoised per (appearance identity, content, enable_panels,
    page), since a single template can call this dozens of times for the same
    content region (chart JS generation runs inside nested per-item loops).
    That also bounds the unknown-content/-collapsed/-key warnings below to
    once per distinct problem per config, instead of once per call.
    """
    appearance = _appearance(skin_dict)
    sections = appearance.get("sections")
    if not sections:
        _report_unmigrated(appearance)
        return []

    wanted = (
        None
        if content is None or str(content).strip().lower() == "all"
        else str(content).strip().lower()
    )
    enable_panels = bool(enable_panels)
    page_key = None if page is None else str(page).strip()
    sub_key = None if subpage is None else str(subpage).strip()
    cache_key = (wanted, enable_panels, page_key, sub_key)
    cached = _cache_get(appearance, cache_key)
    if cached is not None:
        return cached

    slugs = _slugs(appearance, sections)

    segments = []
    # Spans every section, unlike the per-section 'seen' below.
    claimed = set()

    _warn_page_blocks(appearance, page)
    order = _page_order(appearance, page, subpage)
    if order is None:
        # page is None: unscoped/global call across all configured pages
        # (e.g. openmeteo checking forecast presence or telemetry discovering sensor names).
        pages = appearance.get("pages")
        if not pages or not hasattr(pages, "get"):
            _report_missing_pages(appearance)
            return []
        aggregated = []
        seen = set()
        for pkey in getattr(pages, "sections", list(pages.keys())):
            pentry = pages[pkey]
            if not hasattr(pentry, "get"):
                continue
            if "sections" in pentry:
                for sid in _as_list(pentry.get("sections")):
                    if sid not in seen:
                        seen.add(sid)
                        aggregated.append(sid)
            for sub_key_name in SUBPAGES.get(pkey, ()):
                if sub_key_name in pentry and hasattr(pentry[sub_key_name], "get"):
                    sub_entry = pentry[sub_key_name]
                    if "sections" in sub_entry:
                        for sid in _as_list(sub_entry.get("sections")):
                            if sid not in seen:
                                seen.add(sid)
                                aggregated.append(sid)
        section_ids = [sid for sid in aggregated if sid in sections]
    else:
        # Iterate the PAGE's order, not the declaration order - filtering the
        # declared list instead would give per-page selection but not per-page
        # ordering, which is half the feature.
        section_ids = []
        seen_ids = set()
        for sid in order:
            if sid in seen_ids:
                continue
            seen_ids.add(sid)
            if sid not in sections:
                problem = ("page-section", page_key, sid)
                if not _problem_seen(appearance, problem):
                    _mark_problem(appearance, problem)
                    log.warning(
                        "panelorder: page '%s' lists section '%s', which is "
                        "not defined in [[[sections]]]; ignoring it.",
                        page_key,
                        sid,
                    )
                continue
            section_ids.append(sid)

    for section_id in section_ids:
        section = sections[section_id]

        raw_content = section.get("content", CARD)
        section_content = str(raw_content).strip().lower()
        if section_content not in CONTENTS:
            problem = ("content", section_id, str(raw_content))
            if not _problem_seen(appearance, problem):
                _mark_problem(appearance, problem)
                log.warning(
                    "panelorder: section '%s' has content = %s, which is "
                    "not one of %s; skipping the section.",
                    section_id,
                    raw_content,
                    ", ".join(CONTENTS),
                )
            continue

        if wanted is not None and section_content != wanted:
            continue

        _warn_unknown_keys(appearance, section, section_id)

        items = []
        seen = set()
        for item in _as_list(section.get("items")):
            if item in seen:
                continue
            if item in SINGLETON_ITEMS:
                if item in claimed:
                    continue
                claimed.add(item)
            seen.add(item)
            items.append(item)
        if not items:
            continue

        raw_title = section.get("title", "")
        if isinstance(raw_title, (list, tuple)):
            # configobj returns a list when a value contains a comma, e.g. an
            # unquoted "title = Wind, Rain"; join it back into the string the
            # user meant rather than rendering the Python repr.
            raw_title = ", ".join(str(p) for p in raw_title)
        title = str(raw_title).strip()
        if enable_panels and title:
            segments.append(
                {
                    "type": _panel_type(appearance, section, section_id),
                    "title": title,
                    "items": items,
                    "slug": slugs[section_id],
                    "content": section_content,
                }
            )
        else:
            segments.append({
                "type": None,
                "title": "",
                "items": items,
                "slug": slugs[section_id],
                "content": section_content,
            })

    _cache_set(appearance, cache_key, segments)
    return segments


# The set of names any page would treat as telemetry, memoised per config in
# the same weakref-bounded bucket as the segments cache.  A 1-tuple key like
# _SLUG_KEY, so it can never collide with the 4-tuple segment keys.
_TELEMETRY_KEY = ("__telemetry__",)


def _telemetry_names(skin_dict):
    appearance = _appearance(skin_dict)
    cached = _cache_get(appearance, _TELEMETRY_KEY)
    if cached is not None:
        return cached
    names = set()
    # Signal 1: a [[Telemetry]] [[[<name>]]] subsection.  Only subsections
    # count - scalars like chart_days live at the same level and are settings,
    # not sensors.
    telemetry = skin_dict.get("Extras", {}).get("Telemetry", {})
    for key in getattr(telemetry, "sections", []):
        names.add(str(key).strip())
    # Signal 2: listed in any telemetry section, on any page.  No page filter,
    # so an item that only the telemetry page shows still counts when a card
    # section on another page names it.
    for content in ("telemetry", "telemetry_chart"):
        for item in order_items(skin_dict, content):
            names.add(item)
    _cache_set(appearance, _TELEMETRY_KEY, names)
    return names


def is_telemetry_item(skin_dict, name):
    """True when a listed item should render as telemetry, not weather.

    Either signal is enough: a [[Telemetry]] subsection for the name, or
    membership in a content = telemetry / telemetry_chart section.  The
    shipped config has no live subsections, so membership is what makes the
    shipped lists work unconfigured; the subsection keeps an item classified
    after someone deletes the telemetry sections, which is the natural thing
    to do once the items live elsewhere.
    """
    if name is None:
        return False
    return str(name).strip() in _telemetry_names(skin_dict)


_EMBEDDED_KEY = ("__embedded__",)


def _embedded_names(skin_dict):
    appearance = _appearance(skin_dict)
    cached = _cache_get(appearance, _EMBEDDED_KEY)
    if cached is not None:
        return cached
    names = set()
    # A [[Embedded]] [[[<name>]]] subsection. Only subsections count.
    embedded = skin_dict.get("Extras", {}).get("Embedded", {})
    for key in getattr(embedded, "sections", list(embedded.keys())):
        val = embedded[key]
        if hasattr(val, "get"):
            names.add(str(key).strip())
    _cache_set(appearance, _EMBEDDED_KEY, names)
    return names


def is_embedded_item(skin_dict, name):
    """True when a listed item represents embedded content (iframe or image).

    Either of:
    - defined under [Extras][[Embedded]]
    - starts with 'iFrame' or 'image' (case-insensitive)
    """
    if name is None:
        return False
    sname = str(name).strip()
    if sname in _embedded_names(skin_dict):
        return True
    sname_lower = sname.lower()
    return sname_lower.startswith("iframe") or sname_lower.startswith("image")


def order_items(skin_dict, content=None, page=None, subpage=None):
    """Flat, de-duplicated item names for one content region or all sections.

    For loops that need the items themselves rather than the layout, such as
    the chart JavaScript generation.  De-duplicates here rather than relying
    on parse_sections, which keeps a repeat that appears in a second section -
    a chart's config and series data must still be emitted once however many
    times the chart is displayed.

    Passing the page narrows this to the charts that page actually draws,
    which is where the report-generation saving comes from.
    """
    out = []
    seen = set()
    for segment in parse_sections(skin_dict, content, True, page, subpage):
        for item in segment["items"]:
            if item not in seen:
                seen.add(item)
                out.append(item)
    return out


class PanelOrder(SearchList):
    """Exposes the two helpers above to Cheetah."""

    def __init__(self, generator):
        SearchList.__init__(self, generator)
        log.info("PanelOrder version %s", VERSION)

    def get_extension_list(self, timespan, db_lookup):
        skin_dict = self.generator.skin_dict

        def panel_segments(content=None, enable_panels=None, page=None,
                            subpage=None):
            if enable_panels is None:
                enable_panels = enable_panels_setting(skin_dict)
            return parse_sections(skin_dict, content, enable_panels, page,
                                  subpage)

        def panel_items(content=None, page=None, subpage=None):
            return order_items(skin_dict, content, page, subpage)

        def panel_page_setting(key, page=None, subpage=None):
            return page_setting(skin_dict, key, page, subpage)

        def panel_section_slug(section_id):
            return section_slug(skin_dict, section_id)

        def is_telemetry(name):
            return is_telemetry_item(skin_dict, name)

        def is_embedded(name):
            return is_embedded_item(skin_dict, name)

        return [{
            "panelSegments": panel_segments,
            "panelItems": panel_items,
            "panelPageSetting": panel_page_setting,
            "panelSectionSlug": panel_section_slug,
            "isTelemetryItem": is_telemetry,
            "isEmbeddedItem": is_embedded,
        }]
