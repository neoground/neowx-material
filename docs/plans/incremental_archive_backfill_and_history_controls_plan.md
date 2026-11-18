# Plan: Live-First Incremental Archive Generation & History Controls (`NeowxArchiveGenerator`)

## 1. Problem & Goals
On initial installation or with a large WeeWX database, generating every historical daily (`365/yr`), weekly (`52/yr`), monthly (`12/yr`), and yearly (`1/yr`) page in a single report cycle can take hours. Because WeeWX runs `CheetahGenerator` sequentially in one thread and skips new 5-minute report cycles while a previous cycle is still running:
1. Live pages (`index.html`, `live.json`, current week/month/year) were previously generated **after** all historical pages, leaving the site empty or stale on first startup.
2. Users testing the skin or running on constrained storage had no way to disable historical daily/weekly generation without breaking current-period pages.

### Goals
- **Idea 1 (Two-Pass Live-First + Reverse-Chronological Incremental Backfill):**
  - Always generate **live and current-period pages first** (~5–10 seconds) on every 5-minute run.
  - Generate missing historical pages in **reverse chronological order** (newest → oldest) after current pages are done, capped by a per-cycle batch/time budget so every report run finishes well before the next 5-minute cycle.
- **Idea 2 (Orthogonal `archive_start_date` Keywords + `archive_history` Granularity):**
  - Allow `archive_start_date` to accept smart keywords (`all`, `current_year`, `current_month`, `current` / `today`) in addition to `YYYY-MM-DD`.
  - Add `archive_history` to control *which* historical granularities are generated (`all`, `months_years`, `years`, `none`, or a comma-separated list like `months, years`), while always generating the **current** active week/month/year so the navbar works.
- **Idea 3 (Version Metadata & Automatic Refresh of Outdated Historical Files):**
  - Emit `<meta name="neowx-version" content="$Extras.version">` (alongside `<!-- NeoWX Material skin v$Extras.version -->`) in the `<head>` of every generated HTML file.
  - Define `MIN_ARCHIVE_VERSION` per summary type (`SummaryByYear`, `SummaryByMonth`, `SummaryByWeek`, `SummaryByDay`) in `NeowxArchiveGenerator` (with optional `min_archive_version` override in `skin.conf`).
  - When checking existing historical HTML files, read the first 1 KB to extract the file's version; if `file_version < min_required_version`, automatically queue the file for background regeneration in Pass 2 (`_pass='history'`).

---

## 2. Detailed Design

### A. Idea 1: Two-Pass Live-First & Chunked Backfill in `bin/user/neowx_archive.py`

1. **Generator Execution Order (`skin.conf` & `NeowxArchiveGenerator`):**
   - In `[Generators]`, place `weewx.reportengine.CopyGenerator` **before** `user.neowx_archive.NeowxArchiveGenerator` so CSS, JS, and fonts are on disk immediately before HTML generation starts.
   - Reorder `[CheetahGenerator]` sections in `skin.conf` (and enforce in `NeowxArchiveGenerator`) so single-page targets run first:
     1. `[[LiveData]]` (`live.json`)
     2. `[[Global]]` (`manifest.json`)
     3. `[[ToDate]]` (`index.html`, `almanac.html`, `telemetry.html`, `history.html`)
     4. `[[SummaryByYear]]`
     5. `[[SummaryByMonth]]`
     6. `[[SummaryByWeek]]`
     7. `[[SummaryByDay]]`

2. **Two-Pass Generation for `SummaryBy*` Sections:**
   - **Pass 1 (Current Active Spans First):**
     - For every `SummaryBy*` section, immediately generate the span covering `stop_ts` (`timespan.includesArchiveTime(stop_ts)`).
     - This ensures `year-YYYY.html`, `month-YYYY-MM.html`, `week-YYYY-WW.html`, `day-YYYY-MM-DD.html`, and current NOAA reports are updated within the first few seconds of every 5-minute cycle, completely independent of any historical backlog.
   - **Pass 2 (Reverse-Chronological Historical Backfill with Budget):**
     - Collect historical spans (`not timespan.includesArchiveTime(stop_ts)`) in **newest → oldest** order (`reversed`).
     - Priority order across sections: **Years → Months → Weeks → Days** (so all historical years and months complete on the very first run, followed by recent weeks and days).
     - Skip files that already exist on disk (`os.path.exists(_fullname)`), as `NeowxArchiveGenerator` already does.
     - Track a shared per-cycle counter and timer across the `NeowxArchiveGenerator.run()` invocation:
       - `archive_batch_size` *(default: `30` historical files per cycle; `0` = unlimited)*
       - `archive_max_run_time` *(default: `180` seconds max per cycle; `0` = unlimited)*
     - When either budget is reached, log an `INFO` message (`Backfill batch limit reached (generated X historical files in Ys); continuing on next cycle`) and skip remaining missing historical spans until the next 5-minute cycle.

---

### B. Idea 2: `archive_start_date` Keywords & `archive_history` Granularity

Both settings live under `[CheetahGenerator]` in `skin.conf` and work together seamlessly:

```ini
[CheetahGenerator]
    # How far back to generate historical archives:
    # - YYYY-MM-DD     : explicit start date (e.g. 2025-01-01)
    # - all            : from first record in WeeWX database (same as 1970-01-01)
    # - current_year   : from Jan 1 of the current year
    # - current_month  : from the 1st of the current month
    # - current / none : current period only (no past archive dates)
    archive_start_date = 1970-01-01

    # Which historical archive granularities to generate back to archive_start_date:
    # - all            : generate historical years, months, weeks, and days (default)
    # - months_years   : generate historical months and years only (skip past days & weeks)
    # - years          : generate historical years only (skip past days, weeks & months)
    # - none           : generate only the current active week/month/year (fastest, for testing)
    # (Also accepts a comma-separated list of: years, months, weeks, days)
    archive_history = all

    # Maximum number of missing historical archive files to backfill per 5-minute run (0 = unlimited)
    archive_batch_size = 30

    # Maximum seconds to spend backfilling historical archives per 5-minute run (0 = unlimited)
    archive_max_run_time = 180
```

#### How `archive_start_date` + `archive_history` Work Together
Example:
```ini
archive_start_date = current_year
archive_history = months_years
```
- **Current pages (`index.html`, current `week-YYYY-WW.html`, current `month-YYYY-MM.html`, current `year-YYYY.html`)**: Always generated so all top navbar links work.
- **Historical Months & Years**: Generated back to `YYYY-01-01` of the current year.
- **Historical Days & Weeks**: Skipped! Only the current active week is generated.
- **Navigation Context (`_build_nav_context` & `inc/header.inc`)**:
  - When `days` is not in `archive_history`, `has_prev_day` is set to `False` on `index.html` and `context_day_url` always points to `index.html`.
  - When `weeks` is not in `archive_history`, `has_prev_week` is set to `False` on the current week page, and `context_week_url` on Month/Year archive pages points to `current_week_url` (the current active week) rather than an ungenerated historical week.
  - When `months` is not in `archive_history`, `has_prev_month` is set to `False` on the current month page, and `context_month_url` on Year archive pages points to `current_month_url`.

---

## 3. Files to Modify

1. **[`bin/user/neowx_archive.py`](../../bin/user/neowx_archive.py)**
   - Bump `VERSION` (e.g. `1.0.8` → `1.1.0`).
   - Add resolution helpers for `archive_start_date` keywords (`all`, `current_year`, `current_month`, `current` / `today` / `none` / `YYYY-MM-DD`) and `archive_history` (`all`, `months_years`, `years`, `none`, or comma-separated list).
   - Override `run()` / `generate()` to execute **Pass 1** (all `ToDate`/`LiveData`/`Global` templates + current active span of each `SummaryBy*`) before **Pass 2** (reverse-chronological historical spans bounded by `archive_batch_size` and `archive_max_run_time`).
   - Update `_build_nav_context()` so `has_prev_day`, `has_prev_week`, `has_prev_month`, `has_prev_year` and `context_*_url` respect `archive_history` (preventing broken stepper buttons or navbar links when daily/weekly history is disabled).
2. **[`skins/neowx-material/inc/header.inc`](../../skins/neowx-material/inc/header.inc)**
   - Ensure fallback `has_prev_*` logic on `ToDate` pages (`index.html`) respects the `has_prev_day` value provided by `NeowxArchiveGenerator`.
3. **[`skins/neowx-material/skin.conf`](../../skins/neowx-material/skin.conf)**
   - Document `archive_start_date` keywords, `archive_history`, `archive_batch_size`, and `archive_max_run_time` under `[CheetahGenerator]`.
   - Reorder `[CheetahGenerator]` subsections so `[[LiveData]]`, `[[Global]]`, and `[[ToDate]]` appear above `[[SummaryBy*]]`, and put `CopyGenerator` first in `[Generators]`.
4. **[`docs/CONFIGURATION.md`](../CONFIGURATION.md)**
   - Update section 3 (*Historical Archives & Interactive Date Navigation*) to document the new options and incremental backfill behavior.
