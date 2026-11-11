# Configuration & Layout Guide

All skin customization takes place in `skins/neowx-material/skin.conf` ( primarily under `[Extras]` and `[CheetahGenerator]`). After editing `skin.conf`, either wait for the next WeeWX report cycle or run `weectl report run`.

> **Tip:** Use [`config_patcher.py`](CONFIG-PATCHER.md) to store your personal customizations in a `skin.conf.patch` file so they can be re-applied automatically after any skin update.

---

## 1. Appearance, Themes & Dark Mode

Under `[Extras]` and `[[Appearance]]`, you can customize the visual style of your weather dashboard:

```ini
[Extras]
    # Primary Material Design color theme (20+ options: indigo, blue, teal, blue-grey, deep-orange, ...)
    color = indigo

    [[Appearance]]
        # Theme mode: light, dark, or auto (follows OS / browser preference)
        mode = auto

        # Card & chart visual effects
        elevation_level = 1
        hover_effect = lift          # none, lift, glow, border
        transition_speed = normal    # slow, normal, fast
        card_border_radius = medium  # none, small, medium, large

        # Collapsible panels behavior
        enablePanels = true
        rememberPanelState = true    # Keep user-expanded/collapsed state across tab refreshes
        panel_header_bg_light = "#e8eaf6"
        panel_header_text_light = "#1a237e"
        panel_header_bg_dark = "#283593"
        panel_header_text_dark = "#e8eaf6"
```

---

## 2. Page & Section Layout (`[[[pages]]]` & `[[[sections]]]`)

Release 2.0 provides a declarative layout system under `[Extras] → [[Appearance]]` with two main blocks:
1. **`[[[sections]]]`** — defines named groups of cards, charts, telemetry items, or embedded items, optionally wrapped in collapsible Material Design expansion panels.
2. **`[[[pages]]]`** — defines which sections appear on each page (`today`, `day`, `yesterday`, `week`, `month`, `year`, `telemetry`) and in what top-to-bottom order.

> **Important ConfigObj Ordering Rule:** Always write plain `key = value` settings inside a block **above** any `[[[subsection]]]` in `skin.conf`. ConfigObj folds plain settings written below a subsection into that subsection.

### Defining Sections (`[[[sections]]]`)

Each section under `[[[sections]]]` supports:
- **`items`** *(required)* — comma-separated list of observations, custom charts, `forecast`, telemetry fields, or `[[Embedded]]` block names (`iFrame_*`, `image_*`).
- **`title`** *(optional)* — omit for an ungrouped row; set a string (or translation key) to wrap the section in a Material expansion panel with a header.
- **`collapsed`** *(optional)* — `false` (default, starts expanded), `true` (starts collapsed), or `none` (panel header shown, always open, no toggle button).
- **`content`** *(optional)* — `card` (default), `chart`, `telemetry`, or `telemetry_chart`.

```ini
[Extras]
    [[Appearance]]
        [[[sections]]]
            [[[[cards_with_forecast]]]]
                items = inTemp, outTemp, outHumidity, forecast, barometer, windSpeed, windrun, rain, inHumidity

            [[[[cards]]]]
                items = inTemp, outTemp, outHumidity, barometer, windSpeed, windrun, rain, inHumidity

            [[[[additional_cards]]]]
                title = Additional Sensors
                items = windchill, heatindex, dewpoint, UV, radiation
                collapsed = true

            [[[[charts]]]]
                title = Custom Charts
                items = outTemp, customChartTemp, barometer, windSpeed, windvec, rain
                content = chart

            [[[[embedded_items]]]]
                title = Weather Maps & Webcams
                items = iFrame_windy, iFrame_rainviewer, imageLogo
                collapsed = false
```

- **Cross-Section Flexibility:**
  - **Telemetry items** (battery gauges, voltages, `rxCheckPercent`) can be added to any `card` or `chart` section on any page. In a `card` section they render as a gauge (if configured under `[[Telemetry]]`), and in a `chart` section they render a time-series chart using that page's time window.
  - **Embedded items** (`iFrame_*`, `image_*` defined under `[[Embedded]]`) can be placed inside any `card`, `chart`, or dedicated section.
- **Auto-Hide Empty Panels:** Sections whose items all have no data in the database are automatically hidden rather than rendered empty.

### Assigning Sections to Pages (`[[[pages]]]`)

`[[[pages]]]` is **mandatory**: every page displays only the sections listed for it, rendered sequentially down the page in the exact order listed:

```ini
[Extras]
    [[Appearance]]
        [[[pages]]]
            [[[[today]]]]
                sections = cards_with_forecast, additional_cards, charts, embedded_items

            [[[[day]]]]
                sections = cards, additional_cards, charts

            [[[[week]]]]
                sections = cards, additional_cards, charts

            [[[[month]]]]
                sections = cards, additional_cards, charts
                # Optional sub-block to override historical archive month pages only:
                # [[[[[month_archive]]]]]
                #     sections = cards, charts

            [[[[year]]]]
                sections = cards, additional_cards, charts
                # Optional sub-block to override historical archive year pages only:
                # [[[[[year_archive]]]]]
                #     sections = cards, charts

            [[[[telemetry]]]]
                sections = telemetry_cards, telemetry_charts
```

---

## 3. Historical Archives & Interactive Date Navigation (Release 2.0)

Release 2.0 uses `user.neowx_archive.NeowxArchiveGenerator` (`[Generators]`) to generate a complete, navigable hierarchy of historical pages:

| Period | Template | Output Path |
| :--- | :--- | :--- |
| **Current / Live** | `index.html.tmpl` | `index.html` |
| **Daily Archive** | `day-%Y-%m-%d.html.tmpl` | `archive/YYYY/days/day-YYYY-MM-DD.html` |
| **ISO Weekly Archive** | `week-%Y-%V.html.tmpl` | `archive/YYYY/weeks/week-YYYY-WW.html` |
| **Monthly Archive** | `month-%Y-%m.html.tmpl` | `archive/YYYY/months/month-YYYY-MM.html` |
| **Yearly Archive** | `year-%Y.html.tmpl` | `archive/YYYY/year-YYYY.html` |
| **NOAA Reports** | `archive/NOAA/NOAA-*.txt.tmpl` | `archive/NOAA/NOAA-YYYY-MM.txt` & `NOAA-YYYY.txt` |

### Live-First Incremental Backfill & Archive Controls

`NeowxArchiveGenerator` runs in **two passes** on every 5-minute report cycle so historical archive generation never blocks live station updates:
1. **Pass 1 (Current & Live Pages First):** Immediately updates `live.json`, `index.html`, `almanac.html`, `telemetry.html`, `history.html`, and the **current** active `year`, `month`, `week`, `day`, and `NOAA` summaries (~5–10 seconds).
2. **Pass 2 (Reverse-Chronological Backfill):** Generates missing historical pages in **newest-to-oldest** order (`Years → Months → Weeks → Days`) up to `archive_batch_size` files (or `archive_max_run_time` seconds) per cycle, automatically continuing on subsequent 5-minute runs until the backlog is complete.

Configure your archive horizon, granularity, and per-cycle backfill budget under `[CheetahGenerator]` in `skin.conf`:

```ini
[CheetahGenerator]
    encoding = utf8

    # How far back to generate historical archives:
    # - YYYY-MM-DD    : explicit calendar date (e.g. 2025-01-01)
    # - all           : from the first record in the WeeWX database (same as 1970-01-01)
    # - current_year  : from January 1 of the current year
    # - current_month : from the 1st of the current month
    # - current       : current period only (aliases: today, none)
    archive_start_date = 1970-01-01

    # Which historical archive granularities to generate back to archive_start_date:
    # - all           : generate historical years, months, weeks, and days (default)
    # - months_years  : generate historical months and years only (skip past days & weeks)
    # - years         : generate historical years only (skip past days, weeks & months)
    # - none          : generate only the current active week/month/year (fastest, for testing)
    # (Also accepts a comma-separated list of: years, months, weeks, days)
    archive_history = all

    # Incremental backfill limits per 5-minute report cycle (0 = unlimited)
    archive_batch_size = 50
    archive_max_run_time = 240
```

> **Note:** Both `archive_start_date` and `archive_history` can be combined (for example, `archive_start_date = current_year` and `archive_history = months_years`). When daily or weekly history is disabled via `archive_history`, the current active week/month/year are still generated for the navbar, and the `<` stepper buttons on disabled scales are automatically deactivated.
>
> **Automatic Version-Based Refresh:** Every generated HTML page embeds `<meta name="neowx-version" content="X.Y.Z">`. When a future release updates an archive template and bumps `MIN_ARCHIVE_VERSION` in `bin/user/neowx_archive.py` (or if you set `min_archive_version` / `min_month_version` / `min_day_version` under `[CheetahGenerator]`), existing historical HTML files with a lower version are automatically refreshed in the background during Pass 2.

### Header Navigation & Stepper Pickers

Every page includes an adaptive navigation hub in the header:
- **Stepper Controls (`<` / `>`) & Dropdown Pickers:** Step backward/forward day-by-day, week-by-week, month-by-month, or year-by-year, or jump directly to any date via the Material calendar/week/month/year dropdown picker.
- **404 Protection Modal:** Before navigating to an archived date, the skin checks whether the target archive page exists and shows a helpful modal if a specific date has no recorded data.
- **Direct NOAA Button:** On Month and Year pages, the document icon in the header opens the corresponding NOAA text summary (`NOAA-YYYY-MM.txt` or `NOAA-YYYY.txt`).
- **Menu Links (`[[Header]]`):** Toggle which links appear in the top navigation bar:
  ```ini
  [Extras]
      [[Header]]
          current_nav_link = yes
          day_nav_link = no
          week_nav_link = yes
          month_nav_link = yes
          year_nav_link = yes
          almanac_nav_link = yes
  ```

---

## 4. Weather Forecast (`[[Forecast]]`)

NeoWX Material integrates 7-day weather forecasts from [Open-Meteo](https://open-meteo.com) (no API key required):

1. Include `forecast` in the `items` list of the card section shown on your front page (e.g., `cards_with_forecast` under `[[[[today]]]]`).
2. Configure `[[Forecast]]` under `[Extras]`:

```ini
[Extras]
    [[Forecast]]
        # Optional: Override coordinates for forecast.
        # Leave empty to automatically use station coordinates from weewx.conf [Station].
        latitude =
        longitude =

        days = 7
        show_icon = yes
        show_description = yes
        variables = temperature, precipitation, wind, sun
```

---

## 5. Embedded iFrames & Images (`[[Embedded]]`)

You can embed external weather maps (Windy, RainViewer, Blitzortung), webcam snapshots, or custom graphics as Material cards on any page:

1. Define an entry under `[Extras] → [[Embedded]]`:
   ```ini
   [Extras]
       [[Embedded]]
           [[[iFrame_windy]]]
               url = "https://embed.windy.com/embed.html?type=map&location=coordinates&zoom=5&overlay=wind"
               title = Windy Map
               aspect_ratio = "16/9"

           [[[image_webcam]]]
               url = "https://example.com/webcam/latest.jpg"
               title = Station Webcam
               aspect_ratio = "16/9"
               alt = Live Webcam
               enable_link = yes
               new_tab = yes
               padding = no
   ```
2. Add the block name (`iFrame_windy`, `image_webcam`) to the `items` list of any section under `[[Appearance]] → [[[sections]]]`.

---

## 6. Further Configuration Guides

- **[Configuration Patcher Guide](CONFIG-PATCHER.md)** — Automatically preserve your `skin.conf` customizations across skin upgrades.
- **[MQTT Real-Time Updates Guide](MQTT.md)** — Live WebSocket updates, flash effects, and sensor mapping.
- **[Telemetry Configuration Guide](TELEMETRY-CONFIG-GUIDE.md)** & **[Voltage Battery Gauges](VOLTAGE-BATTERY-GAUGE-UPDATE.md)** — Station health page, battery gauges, voltage thresholds, and signal quality.
- **[Multi-Axis Charts Usage Guide](MULTI-AXIS-USAGE-GUIDE.md)** & **[Quick Reference](MULTI-AXIS-QUICK-REFERENCE.md)** — Combining multiple Y-axes on a single ApexChart.
- **[Trend Indicators Guide](TREND-INDICATORS.md)** — Unit-aware barometric and sensor trend arrows (`multi`, `davis`, `strict`).
- **[Date & Time Formatting Guide](DATE-FORMATTING.md)** — Customizing chart (Moment.js) and card (`strftime`) date/time formats.
- **[Localization & Language Guide](LOCALIZATION.md)** — Configuring any of the 12 supported languages and OS locales.

