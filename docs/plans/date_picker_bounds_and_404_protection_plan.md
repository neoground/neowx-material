# Implementation Plan: Date Picker Range Clamping & Proactive 404 Protection

## 1. Goal
Ensure users cannot navigate outside valid historical data ranges in NeoWX Material and seamlessly handle the current date:
1. Dynamically retrieve the earliest recorded observation date (`$alltime.start.format('%Y-%m-%d')` / `manager.firstGoodStamp()`) from WeeWX and clamp all navigation controls (Day, Week, Month, and Year pickers and previous/next chevrons).
2. Intercept date navigation requests with a lightweight proactive `HEAD` availability probe. If an archive page is missing (HTTP 404), display a Material Design modal popup warning the user and offering redirection to the live page or staying in place.
3. Automatically redirect navigation to `index.html` whenever navigating to the current day (e.g. via date picker, next-chevron from yesterday, scale switcher, or direct URL), rather than displaying a static `archive/.../day-YYYY-MM-DD.html`.

---

## 2. Architecture & Design

### Phase 1: WeeWX Data Source & Template Bounds
- **Backend / Python (`bin/user/neowx_archive.py`)**:
  - Bump `VERSION` to `1.0.8`.
  - In `NeowxArchiveGenerator.generate()` and `_build_nav_context()`:
    - Ensure `archive_start_date` defaults to the database's `firstGoodStamp()` if `archive_start_date` in `skin.conf` is unset, default (`1970-01-01`), or earlier than the actual first record.
    - Set `current_day_url = f"{base_path}index.html"`.
    - If `next_day_dt.date() >= stop_dt.date()`, route `next_day_url` directly to `f"{base_path}index.html"`.
    - Week Stepper Bounds: Compare the previous week's end date (Sunday: `prev_wk_monday + 6 days`) to `start_dt.date()`. If the previous week contains records (e.g. data starts mid-week on Tuesday Jan 3, 2017), CW 01 is recognized as having data, allowing navigation back from CW 02 to CW 01.
    - Scale Switcher Day link: When viewing current year (`is_current_year`), current month (`is_current_month`), or current week (`is_current_week`), route `context_day_link` directly to `f"{base_path}index.html"`.
    - Expose `archive_start_date`, `archive_stop_date`, `archive_start_year`, `archive_stop_year`, `archive_years_reversed`.
- **Templates (`skins/neowx-material/inc/header.inc`)**:
  - Query `$alltime.start.format('%Y-%m-%d')` directly from WeeWX Cheetah search list as `$weewx_first_date`.
  - If `archive_start_date` is not set or set to `1970-01-01`, set `$archive_start_date = $weewx_first_date`.
  - If `archive_start_date` was configured to a date earlier than `$weewx_first_date`, clamp it to `$weewx_first_date`.
  - Calculate `$archive_start_year = int($archive_start_date.split('-')[0])`.
  - In Year Picker: `#for $yr in $archive_years_reversed`, strictly render years from `archive_stop_year` down to `archive_start_year`.
  - Pass `data-start-date`, `data-stop-date`, `data-start-year`, and `data-stop-year` on `#nwm-nav-hub`.
  - If `$context_day == $archive_stop_date` or today, set `$context_day_url = $base_path + 'index.html'`.
  - Normalize any incoming `$context_day_url` or `$next_day_url` targeting today to `$base_path + 'index.html'`.
- **Live Updates (`skins/neowx-material/live.json.tmpl` & `neowx-live.js`)**:
  - Set `"archive_urls.day": "index.html"`.
  - Include `"archive_start_date"` and `"archive_stop_date"` in `live.json` so dynamic client-side updates keep bounds up to date without a page rebuild.

### Phase 2: Client-Side Picker Clamping (`skins/neowx-material/js/neowx-date-nav.js`)
- **Year Dropdowns in Day, Week, and Month Pickers**:
  - Populate year options strictly down to `startYear`.
- **Month Dropdown in Day (Calendar) Picker**:
  - When `year === startYear`, disable/hide months earlier than `startDate.getMonth() + 1`.
  - When `year === stopYear`, disable/hide months later than `stopDate.getMonth() + 1`.
- **Month Picker**:
  - When `year === startYear`, disable months whose last day `< startDateStr`.
- **Week Picker**:
  - Disable weeks whose end date precedes `startDateStr`.
  - When viewing `startYear`, filter out months before `startDate` from quick tabs.
- **Calendar (Day) Picker**:
  - Disable any day `< startDateStr`.

### Phase 3: Current-Day Routing to `index.html`
- **Day (Calendar) Picker Cell Links**:
  - In `renderCalendarPicker()`, if a day cell corresponds to today (`dateStr === this.stopDateStr` or `dateStr === todayStr`), set its URL directly to `${this.basePath}index.html` instead of `archive/.../day-YYYY-MM-DD.html`.
- **Stepper Chevrons & Next Button Navigation**:
  - When stepping "Next" from yesterday (or any date approaching today), route directly to `index.html`.
- **Scale Switcher Day Links**:
  - When switching scale to Day from current year, month, or week, link directly to `index.html`.
- **Dynamic URL Interceptor & Normalizer**:
  - In `navigateWithCheck(url)`, if the URL targets `/days/day-${stopDateStr}.html` or `/days/day-${todayStr}.html`, normalize target URL to `${this.basePath}index.html` and bypass 404 HEAD check for instant response.
- **Quick Jump (`nxJumpDate`)**:
  - If jumping to today's date, redirect directly to `${base_path}index.html`.
- **Day Template Guard (`day-%Y-%m-%d.html.tmpl`)**:
  - If `day-%Y-%m-%d.html` is visited for the current date (`is_current_day` or date equals stop date / today), execute immediate JavaScript redirect (`window.location.replace`) and meta-refresh to `${base_path}index.html`.

### Phase 4: Proactive Availability Verification & 404 Modal
- **Modal Component (`skins/neowx-material/inc/header.inc`)**:
  - Add `#nwm-date-missing-modal` with Material styling, alert icon, localized title and text, countdown badge, and "Stay" / "Go to Live" buttons.
- **Navigation Interceptor (`skins/neowx-material/js/neowx-date-nav.js`)**:
  - Attach listener to all navigation actions (`.nwm-nav-btn[href]`, `.nwm-picker-cell[href]`, and `nxJumpDate`).
  - If HTTP/HTTPS: send `fetch(url, { method: 'HEAD', cache: 'no-cache' })` with an `AbortController` (1.2s timeout).
  - If response is 200..399: proceed directly (`window.location.href = url`).
  - If response is 404: prevent navigation, show `#nwm-date-missing-modal`, and start a 3-second auto-redirect countdown to `index.html`.
  - If user clicks "Stay Here", cancel countdown and dismiss modal.
  - If running under `file://` protocol or request times out: fall back to normal direct navigation.
- **Styles (`skins/neowx-material/scss/_neowx-material.scss`)**:
  - Add modal styling for `#nwm-date-missing-modal` and compile with `yarn run build`.
- **Localization (`skins/neowx-material/lang/en.conf`, `de.conf`)**:
  - Add localized strings:
    - `no_data_modal_title`
    - `no_data_modal_text`
    - `redirecting_to_live`
    - `stay_here`
    - `go_to_live`
