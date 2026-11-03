# 🚀 NeoWX Material – Release 2.0.0

Release **2.0.0** is a major architectural milestone for **NeoWX Material**. It introduces a unified historical archive and interactive date-navigation system, a flexible per-page section layout engine, cross-page telemetry and embedded widget support, and smart live-first incremental archive generation.

---

## 🎉 Major New Features in 2.0

### 🗓️ 1. Interactive Archive Navigation & Full Historical Timeline
- **Header Navigation Hub (`inc/header.inc` & `js/neowx-date-nav.js`)**:
  - Adaptive `<` / `>` stepper controls and Material dropdown pickers for **Day** (calendar), **ISO Week** (`YYYY - CW WW`), **Month**, and **Year** directly in the top navbar.
  - Strict `[archive_start_date, archive_stop_date]` date bounds so users cannot select dates outside the station's available history.
  - **Proactive 404 Protection**: Verifies target archive pages before navigating and presents a helpful modal with an optional auto-redirect to the Live view if a date has no recorded data.
  - **Direct NOAA Access**: Dedicated header button on Month and Year views to open the corresponding `NOAA-YYYY-MM.txt` or `NOAA-YYYY.txt` report.
- **Unified Archive Generator (`user.neowx_archive.NeowxArchiveGenerator`)**:
  - Replaces static `yesterday.html`, `week.html`, `month.html`, `year.html`, and `archive.html` pages with a structured archive hierarchy:
    - `archive/YYYY/days/day-YYYY-MM-DD.html` (`SummaryByDay`)
    - `archive/YYYY/weeks/week-YYYY-WW.html` (`SummaryByWeek` – Monday-to-Monday ISO weeks)
    - `archive/YYYY/months/month-YYYY-MM.html` (`SummaryByMonth`)
    - `archive/YYYY/year-YYYY.html` (`SummaryByYear`)
    - `archive/NOAA/NOAA-YYYY-MM.txt` & `NOAA-YYYY.txt`

### ⚡ 2. Live-First Two-Pass Generation & Incremental Backfill
- **No More Multi-Hour Startup Delays**:
  - **Pass 1 (Current & Live Pages First)**: `live.json`, `manifest.json`, `index.html`, `almanac.html`, `telemetry.html`, `history.html`, and the **current** active `year`, `month`, `week`, `day`, and `NOAA` summaries are always generated first (~5–10 seconds).
  - **Pass 2 (Reverse-Chronological Batch Backfill)**: Missing or outdated historical pages are generated from **newest to oldest** (`Years → Months → Weeks → Days`) up to a configurable per-cycle limit (`archive_batch_size = 50`, `archive_max_run_time = 240`), continuing automatically across subsequent 5-minute cycles without ever delaying live updates.
  - **Version Metadata & Automatic Historical Refresh**: Every generated HTML file carries `<meta name="neowx-version" content="$Extras.version">`. `NeowxArchiveGenerator` checks `MIN_ARCHIVE_VERSION` per summary type (`year`, `month`, `week`, `day`) and automatically refreshes outdated historical HTML files in the background when required by a new release.
- **Flexible Archive Horizon & Granularity Controls (`[CheetahGenerator]`)**:
  - **`archive_start_date`**: Supports explicit `YYYY-MM-DD` dates as well as smart keywords: `all` (`1970-01-01`), `current_year`, `current_month`, and `current` (`today` / `none`).
  - **`archive_history`**: Control which historical granularities are generated (`all`, `months_years`, `years`, `none`, or a comma-separated list like `years, months, weeks, days`) while always keeping the current active week/month/year online for the navbar.

### 🧩 3. Per-Page Section Layout & Collapsible Panels (`[[[pages]]]` & `[[[sections]]]`)
- **Explicit Per-Page Layout (`[[Appearance]] → [[[pages]]]`)**:
  - Choose and order which sections appear on each page (`today`, `day`, `yesterday`, `week`, `month`, `year`, `telemetry`), with optional archive-specific overrides (`month_archive`, `year_archive`).
  - Sections render sequentially down the page in the exact order listed, allowing cards, charts, and embedded panels to be freely ordered or interleaved.
- **Persistent Expansion Panels**:
  - Group any set of cards or charts into collapsible Material Design panels (`collapsed = true | false | none`).
  - **History** (`history.html`) and **Almanac** (`almanac.html`) tables and record sections are now also wrapped in collapsible Material expansion panels.
  - Browser tab panel state is preserved across auto-refreshes (`rememberPanelState = true`) and restored before first paint to prevent layout flash.

### 🔌 4. Universal Telemetry & Embedded Widgets on Any Page
- **Telemetry Everywhere**: List any telemetry sensor (battery gauge, voltage bar, `rxCheckPercent` signal quality) inside *any* `card` or `chart` section on any dashboard page. Telemetry cards on the Current page also update live via **MQTT** when mapped under `[[MQTT]] → [[[SensorMapping]]]`.
- **Embedded iFrames & Images Everywhere**: Place `[[Embedded]]` items (`iFrame_*`, `image_*` such as Windy, RainViewer, Blitzortung, or custom webcam/logo cards) into any card or chart section on any page.

### 🧹 5. Clean Template Architecture & Live Header/Footer Sync
- **Dedicated `inc/` Partials Folder**: All 23 template partials (`head.inc`, `header.inc`, `footer.inc`, `telemetry.inc`, `embedded_region.inc`, `chart_*.inc`, `graph_*.inc`) are now organized under `skins/neowx-material/inc/`.
- **Anti-Flash Live Header/Footer (`js/neowx-live.js` & `live.json`)**: Static archived pages dynamically sync the current station date, sun/moon almanac, WeeWX/skin version, and uptimes from `live.json` without flashing stale build-time values.

---

## ⚠️ Breaking Changes & Upgrade Notes (1.x → 2.0)

1. **`[Generators]` in `skin.conf`**:
   Release 2.0 requires `user.neowx_archive.NeowxArchiveGenerator`:
   ```ini
   [Generators]
       generator_list = weewx.reportengine.CopyGenerator, user.neowx_archive.NeowxArchiveGenerator
   ```
2. **Mandatory `[[[pages]]]` under `[Extras] → [[Appearance]]`**:
   Every page (`today`, `day`, `yesterday`, `week`, `month`, `year`, `telemetry`) must be listed under `[[[pages]]]` with the `sections` it displays.
3. **Config Patcher Users (`skin.conf.patch`)**:
   - Note that the shipped default card sections are now named `cards_with_forecast` (used on `today`) and `cards` (used on archive pages).
   - Re-apply your patch against the new `skin.conf` (`./config_patcher.py skin.conf skin.conf.patch`) and verify any custom `[CheetahGenerator]` or `[[[pages]]]` / `[[[sections]]]` overrides.

---

## 📚 Documentation Updates
- Streamlined [`README.md`](../README.md) with a quick-start overview and documentation index.
- Added [`docs/CONFIGURATION.md`](CONFIGURATION.md) covering layout (`[[[pages]]]`, `[[[sections]]]`), archive navigation & backfill settings, forecasts, and embedded content.
- Added [`docs/LOCALIZATION.md`](LOCALIZATION.md) covering all 12 supported languages, OS/systemd locale configuration, and translation contributions.

