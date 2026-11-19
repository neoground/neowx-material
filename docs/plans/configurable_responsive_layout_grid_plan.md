# Configurable Responsive Layout Breakpoints & Balanced Section Grid Plan

## 1. Goals & Requirements

1. **Remove the forecast layout restriction**:
   - Remove the `:not(:has(> .card-forecast))` special-case in `skins/neowx-material/scss/_neowx-material.scss` so forecast cards follow the exact same responsive sizing rules as all other items.
2. **Unified, minimal breakpoint + `items_per_row` config in `skin.conf`**:
   - Eliminate the separate `card` vs `chart` grid split and redundant per-column settings.
   - Define **4 named viewport ranges** (`small`, `medium`, `large`, `desktop`) under `[Extras] -> [[Appearance]] -> [[[Layout]]]` using Bootstrap 5 standard defaults (`0 / 768 / 1200 / 1400` px) with `1 / 2 / 3 / 4` items per row.
   - `desktop.min_width` (default `1400`) controls when the layout switches from 1 stacked column to side-by-side multi-column bands (`desktop_columns = 2..5`).
   - Inside multi-column desktop bands of `k` columns (`2..5`), max items per column are derived automatically as `max(1, desktop.items_per_row // k)`.
3. **Balanced Row Packing (`balance_rows`)**:
   - Automatically avoid ugly orphan rows when switching breakpoints or when a section has fewer/uneven items (e.g. 4 items dropping from `4/row` to max `3/row` pack cleanly as **2 + 2 at 50%** instead of **3 + 1**; 2 items in a 3- or 4-slot row scale to **50% / 50%**; a 3-day forecast in a 4-slot row fills **3 × 33.3%**).
   - When items cannot divide evenly into rows (e.g. 5 items at max `3/row` → `3 + 2`), every item keeps the exact same column width (`33.3%`) as the row above — incomplete last rows never stretch out of proportion.
   - Configurable globally via `balance_rows = true` (default) under `[[[Layout]]]` and overridable per section (`balance_rows = true | false`).
4. **Section-level `items_per_row` overrides**:
   - Allow any section under `[[[sections]]]` to set `items_per_row = N` (plus optional per-range `items_per_row_small`, `items_per_row_medium`, `items_per_row_large`, `items_per_row_desktop`), enabling mixed densities on the same page (e.g. `column = full` + `items_per_row = 1` for a maximized single item, `items_per_row = 2` for 2 wide items, `items_per_row = 4` for 4 cards).

---

## 2. Configuration Design (`skins/neowx-material/skin.conf`)

### 2.1 Global Breakpoints, Items Per Row & Row Balancing (`[[Appearance]] -> [[[Layout]]]`)

Placed under `[Extras] -> [[Appearance]]` above `[[[pages]]]`:

```ini
        # Responsive Layout Breakpoints & Items Per Row
        # ---------------------------------------------------------------------
        # Defines the 4 mobile-first viewport width ranges (min_width in px) and
        # the maximum number of items across the viewport in each range. All item
        # types (value cards, charts, forecast, telemetry, embedded) follow the
        # exact same rules.
        #
        # Defaults match Bootstrap 5 breakpoints:
        #   small   : 0px .. medium.min_width - 1       (phones, 1 layout column)
        #   medium  : 768px .. large.min_width - 1      (tablets, Bootstrap md, 1 layout column)
        #   large   : 1200px .. desktop.min_width - 1   (laptops, Bootstrap xl, 1 layout column)
        #   desktop : >= 1400px                         (wide desktop, Bootstrap xxl; activates desktop_columns 2..5)
        #
        # In multi-column desktop bands (desktop_columns = 2..5), each column
        # automatically holds up to max(1, desktop.items_per_row // desktop_columns)
        # items per row (e.g. with items_per_row = 4: full=4, 2-col=2, 3..5-col=1).
        #
        # balance_rows = true (default):
        #   Balances each row group using effective_cols = ceil(N / ceil(N / C))
        #   so items fill rows evenly without orphans (e.g. 4 items at max 3/row
        #   render as 2+2 at 50% instead of 3+1; 2 items at max 3/row scale to
        #   50%/50%; 5 items at max 3/row render as 3+2 keeping row 1's 33% width).
        #   Set balance_rows = false globally or on an individual section to keep
        #   strict fixed column widths.
        [[[Layout]]]
            balance_rows = true

            [[[[small]]]]
                min_width = 0
                items_per_row = 1

            [[[[medium]]]]
                min_width = 768
                items_per_row = 2

            [[[[large]]]]
                min_width = 1200
                items_per_row = 3

            [[[[desktop]]]]
                min_width = 1400
                items_per_row = 4
```

### 2.2 Section-Level Overrides (`[[Appearance]] -> [[[sections]]]`)

Any section (titled panel or untitled loose section, in either `column = full` or `column = 1..5`) may set:

```ini
            [[[[featured_map]]]]
                items = earthquake_map
                title = Earthquakes
                column = full
                items_per_row = 1

            [[[[wide_charts]]]]
                items = customChartTemp, customChartBarometer
                title = Primary Charts
                content = chart
                column = full
                items_per_row = 2

            [[[[overview_cards]]]]
                items = outTemp, outHumidity, barometer, rain
                title = Overview
                column = full
                items_per_row = 4
                # Optional: disable auto-balancing for this section only
                # balance_rows = false
```

- **Smart responsive clamping for scalar `items_per_row = N`**:
  - `small`: `min(N, small.items_per_row)` (stays `1` on phones)
  - `medium`: `min(N, medium.items_per_row)` (`1` when `N=1`, `2` when `N>=2`)
  - `large`: `min(N, large.items_per_row)` (`1` when `N=1`, `2` when `N=2`, `3` when `N>=3`)
  - `desktop`: exact **`N`** (`1..12` — e.g. `1` = 100%, `2` = 50%, `3` = 33.33%, `4` = 25%)
- **Optional per-range overrides** on a section (for exact control at individual breakpoints):
  - `items_per_row_small`, `items_per_row_medium`, `items_per_row_large`, `items_per_row_desktop` (integers `1..12`, taking precedence over `items_per_row` for that range).
- **Optional `balance_rows = true | false`** per section to override the global `[[[Layout]]] -> balance_rows` setting.

---

## 3. Technical Architecture

### 3.1 Unified CSS Custom Property Grid (`--nwm-cols` & `--nwm-cols-eff`)

1. Every grid item wrapper across all templates uses one uniform wrapper:
   ```html
   <div class="col-12 nwm-grid-item mb-4" data-item-section="$section">
   ```
   - `col-12` preserves Bootstrap's 15px horizontal gutters; hardcoded `col-md-6`, `col-xl-4`, and `col-xl-6` are removed so Bootstrap's fixed breakpoints never conflict with user-configured breakpoints.
   - `embedded_region.inc` drops its card-vs-chart `_emb_col_class` branching.
2. Base sizing rule in `skins/neowx-material/scss/_neowx-material.scss`:
   ```scss
   .nwm-grid-item {
     flex: 0 0 calc(100% / var(--nwm-cols-eff, var(--nwm-cols, 1)));
     max-width: calc(100% / var(--nwm-cols-eff, var(--nwm-cols, 1)));
   }
   ```
   - `--nwm-cols` is set purely by CSS (`@media` breakpoints + section rules) and represents the configured upper bound `C`.
   - `--nwm-cols-eff` is optionally set by `nwmBalanceGrid()` when `balance_rows` is active and `effectiveCols != C`.
   - `flex-grow` stays `0`, guaranteeing incomplete final rows (e.g., `3 + 2`) always maintain the exact same item width/aspect ratio as rows above.
3. **Zero-specificity SCSS fallback (`:where(...)`)**:
   - `_neowx-material.scss` provides default `:where(...)` fallback rules (`0 / 768 / 1200 / 1400`, `1 / 2 / 3 / 4`, no forecast exclusion) so pages still render cleanly if templates/CSS are deployed before WeeWX restarts.
   - Once `panelLayoutCSS()` emits `<style id="nwm-layout-css">` in `head.inc` using normal selectors (outranking `:where(...)`), custom breakpoints work cleanly whether raised (e.g. `1660px` / `1920px`) or lowered (e.g. `1280px`).

### 3.2 Balanced Row Packing (`skins/neowx-material/js/app.js`)

- `head.inc` emits a minimal JSON config object from `panelorder.py` (`window.NWM_LAYOUT_CONFIG = $panelLayoutConfigJSON()`), containing:
  ```json
  {
    "breakpoints": [768, 1200, 1400],
    "balanceRows": true,
    "sectionBalance": {"some_section": false}
  }
  ```
- In `js/app.js`, `nwmBalanceGrid()` runs **immediately at script parse time** (after `</main>` and **before** ApexCharts / Leaflet chart init scripts run in the templates — zero flash, zero double chart initialization):
  1. Finds every `.row` element containing direct `.nwm-grid-item` children.
  2. Partitions direct children into contiguous runs of `.nwm-grid-item` elements, breaking a run whenever encountering either:
     - a non-`.nwm-grid-item` divider (e.g. `<div class="col-12"></div>` separating loose sections or isolating forecast cards), or
     - a change in `data-item-section`.
  3. For each contiguous group of length `N`:
     - Determines if balancing is enabled (`sectionBalance[slug]` if defined, else `balanceRows`).
     - Clears any previous inline `--nwm-cols-eff` on all items in the group.
     - If enabled and `N > 0`:
       - Reads CSS `--nwm-cols` (`C`, integer `>= 1`) from `getComputedStyle(group[0]).getPropertyValue('--nwm-cols')`.
       - Computes `rows = Math.ceil(N / C)` and `eff = Math.ceil(N / rows)`.
       - If `eff !== C`, sets `item.style.setProperty('--nwm-cols-eff', String(eff))` on every item in the group.
  4. Registers `window.matchMedia('(min-width: ' + bp + 'px)')` change listeners for each configured breakpoint in `breakpoints` so crossing a breakpoint re-balances automatically in `< 1ms` without polling on every resize pixel.

| `N` items | `C = 2` | `C = 3` | `C = 4` |
| :---: | :--- | :--- | :--- |
| **1** | `1` (100%) | `1` (100%) | `1` (100%) |
| **2** | `2` (50%) | `2` (50%) | `2` (50%) |
| **3** | `2` (2+1 @ 50%) | `3` (33%) | `3` (33%) |
| **4** | `2` (2+2 @ 50%) | `2` (**2+2 @ 50%**) | `4` (25%) |
| **5** | `2` (2+2+1 @ 50%) | `3` (3+2 @ 33%) | `3` (**3+2 @ 33%**) |
| **6** | `2` (2+2+2 @ 50%) | `3` (3+3 @ 33%) | `3` (**3+3 @ 33%**) |
| **7** | `2` (2+2+2+1 @ 50%) | `3` (3+3+1 @ 33%) | `4` (4+3 @ 25%) |
| **9** | `2` (4×2+1 @ 50%) | `3` (3+3+3 @ 33%) | `3` (**3+3+3 @ 33%**) |

### 3.3 Backend Parsing & Helpers (`bin/user/panelorder.py`)

- Bump `VERSION = "2.7.0"`.
- Add `"items_per_row"`, `"items_per_row_small"`, `"items_per_row_medium"`, `"items_per_row_large"`, `"items_per_row_desktop"`, and `"balance_rows"` to `SETTING_KEYS`.
- Add `parse_layout_config(skin_dict)`:
  - Reads `[Extras] -> [[Appearance]] -> [[[Layout]]]` with defaults `balance_rows = True`, `small(0, 1)`, `medium(768, 2)`, `large(1200, 3)`, `desktop(1400, 4)`.
  - Validates `min_width` (`0..10000`, strictly ascending `small < medium < large < desktop`) and `items_per_row` (`1..12`), logging a memoized warning and falling back to safe defaults on invalid input.
- Add `generate_layout_css(skin_dict)` exposed to Cheetah as `$panelLayoutCSS()`:
  - Emits compact CSS inside `<style id="nwm-layout-css">` in `head.inc`:
    1. Base (`small`): `.nwm-layout-col { flex: 0 0 100%; max-width: 100%; }`, `.nwm-grid-item { --nwm-cols: <small>; }`, plus any `small` section overrides.
    2. `@media screen and (min-width: <medium>px)`: `.nwm-grid-item { --nwm-cols: <medium>; }`, plus `medium` section overrides.
    3. `@media screen and (min-width: <large>px)`: `.nwm-grid-item { --nwm-cols: <large>; }`, plus `large` section overrides.
    4. `@media screen and (min-width: <desktop>px)`:
       - Sets `.nwm-desktop-cols-2..5 > .nwm-layout-col` widths (`50%`, `33.333333%`, `25%`, `20%`).
       - Sets `--nwm-cols` for `.nwm-layout-full .nwm-grid-item` (`desktop.items_per_row`) and `.nwm-desktop-cols-<k> > .nwm-layout-col .nwm-grid-item` (`max(1, desktop.items_per_row // k)` for `k = 2..5`).
       - Emits section-level overrides `.nwm-layout-full .nwm-grid-item[data-item-section="<slug>"], .nwm-layout-col .nwm-grid-item[data-item-section="<slug>"] { --nwm-cols: <N>; }`.
- Add `generate_layout_config_json(skin_dict)` exposed to Cheetah as `$panelLayoutConfigJSON()`:
  - Returns safe, JSON-encoded `{breakpoints, balanceRows, sectionBalance}` for `js/app.js`.

---

## 4. Files to Modify

| File | Change |
| :--- | :--- |
| `bin/user/panelorder.py` | Bump `VERSION = "2.7.0"`; add `items_per_row*` and `balance_rows` to `SETTING_KEYS`; add `parse_layout_config()`, `generate_layout_css()` (`$panelLayoutCSS()`), and `generate_layout_config_json()` (`$panelLayoutConfigJSON()`) |
| `skins/neowx-material/skin.conf` | Add documented `[Extras] -> [[Appearance]] -> [[[Layout]]]` block (`balance_rows` + 4 ranges with `min_width` & `items_per_row`) and document section-level `items_per_row` & `balance_rows` |
| `skins/neowx-material/inc/head.inc` | Emit `<style id="nwm-layout-css">$panelLayoutCSS()</style>` and `window.NWM_LAYOUT_CONFIG = $panelLayoutConfigJSON();` guarded by `$varExists(...)` |
| `skins/neowx-material/js/app.js` | Add synchronous `nwmBalanceGrid()` helper + `matchMedia` breakpoint listeners |
| `skins/neowx-material/scss/_neowx-material.scss` | Remove `:not(:has(> .card-forecast))`; add `.nwm-grid-item` `calc(100% / var(--nwm-cols-eff, var(--nwm-cols, 1)))` base rule and `:where(...)` default fallback rules; rebuild `css/style.css` & `css/style.min.css` |
| `skins/neowx-material/inc/forecast.inc` | Switch day card wrapper to `col-12 nwm-grid-item mb-4` + `data-item-section="$section"` |
| `skins/neowx-material/inc/telemetry.inc` | Switch card/chart wrappers to `col-12 nwm-grid-item mb-4` + `data-item-section="$section"` |
| `skins/neowx-material/inc/embedded_region.inc` | Simplify all embedded wrappers to `col-12 nwm-grid-item mb-4` + `data-item-section="$section"` |
| `skins/neowx-material/index.html.tmpl` | Switch `valuesCard` / `chartCard` wrappers to `col-12 nwm-grid-item mb-4` + `data-item-section="$section"` |
| `skins/neowx-material/day-%Y-%m-%d.html.tmpl` | Switch card / chart / threshold wrappers to `col-12 nwm-grid-item mb-4` + `data-item-section="$section"` |
| `skins/neowx-material/week-%Y-%V.html.tmpl` (CRLF) | Same wrapper updates, preserving CRLF line endings |
| `skins/neowx-material/month-%Y-%m.html.tmpl` (CRLF) | Same wrapper updates, preserving CRLF line endings |
| `skins/neowx-material/year-%Y.html.tmpl` (CRLF) | Same wrapper updates, preserving CRLF line endings |
| `docs/LAYOUT-CONFIGURATION.md` | New dedicated guide covering responsive breakpoints (`[[[Layout]]]`), `desktop_columns`, automatic multi-column derivation, `balance_rows` balanced row packing (with examples & tables), and section-level `items_per_row` overrides |
| `README.md` (CRLF) | Highlight the new configurable responsive layout & balanced grid feature in the Features section and add a link to `docs/LAYOUT-CONFIGURATION.md` in the Documentation table (preserving CRLF) |

---

## 5. Verification Plan

### 5.1 Functional & Unit Verification
1. **Python unit tests for `bin/user/panelorder.py`**:
   - Default config (explicit or omitted `[[[Layout]]]`) emits `0 / 768 / 1200 / 1400`, `1 / 2 / 3 / 4`, `balanceRows = true`, and multi-column `max(1, items_per_row // k)`.
   - Custom breakpoints (both lowered e.g. `1280px` and raised e.g. `1660px` / `1920px`) emit accurate `@media` rules and matching `breakpoints` array in JSON.
   - Invalid/out-of-order `min_width`, `items_per_row`, or `balance_rows` logs a warning and falls back safely.
   - Section-level `items_per_row = 1..4`, per-range `items_per_row_small..desktop`, and per-section `balance_rows = false` all resolve accurately.
2. **JavaScript unit tests for `nwmBalanceGrid()`**:
   - Verify balanced column calculation across all key `(N, C)` pairs: `(4, 3) -> 2`, `(2, 3) -> 2`, `(1, 4) -> 1`, `(3, 4) -> 3`, `(5, 3) -> 3`, `(6, 4) -> 3`, `(7, 4) -> 4`, `(9, 4) -> 3`.
   - Verify contiguous grouping splits cleanly across `<div class="col-12"></div>` dividers (e.g. mixed card + forecast sections) and across different `data-item-section` slugs.
   - Verify `balance_rows = false` (global or per-section) skips setting `--nwm-cols-eff`.
   - Verify `matchMedia` change clears old `--nwm-cols-eff` and recomputes cleanly for the new breakpoint.
3. **Build, Line Endings & Template Checks**:
   - Run `cd skins/neowx-material && yarn run build` with zero Sass errors.
   - Verify CRLF line endings are preserved on `week`, `month`, and `year` templates.
   - Verify Cheetah directive balance across all edited templates and `node -c skins/neowx-material/js/app.js`.
4. **Visual / Layout Scenario Checks**:
   - Verify cards, charts, forecast, telemetry, and embedded items size uniformly at all 4 ranges.
   - Verify mixed full-width sections (`items_per_row = 1`, `2`, `4`) and balanced 4-item sections switching from `4/row` → `3/row` (`2+2 @ 50%`) render cleanly.
   - Verify pre-restart `:where(...)` fallback works cleanly when `panelLayoutCSS` is absent.

### 5.2 Security Verification
- **Strict Config Validation & CSS/Script Injection Prevention**:
  - All numeric values (`min_width`, `items_per_row`, `items_per_row_*`) are strictly parsed as bounded Python `int`s (`0..10000` for px, `1..12` for counts) before formatting into CSS or JSON — raw `skin.conf` strings are never interpolated directly into `<style>` or `<script>`.
  - Section slugs emitted into `[data-item-section="<slug>"]` selectors and `sectionBalance` JSON keys come exclusively from `panelorder.py`'s sanitized slug generator and are verified against `^[A-Za-z0-9_-]+$` before inclusion.
  - JSON emitted into `<script>` via `$panelLayoutConfigJSON()` escapes `<` / `>` / `&` (`\u003c`, `\u003e`, `\u0026`) so `</script>` breakout is impossible even on malformed input.
- **Template Attribute Quoting & Safe DOM Handling**:
  - All `data-item-section="$section"` and `class="..."` attributes in modified templates are double-quoted.
  - `nwmBalanceGrid()` in `js/app.js` uses only `getComputedStyle` and `style.setProperty` / `style.removeProperty` — zero `innerHTML`, `outerHTML`, `document.write`, or `eval`.

---

## 6. Documentation Deliverables

After implementation and verification pass:
1. Create **`docs/LAYOUT-CONFIGURATION.md`** documenting:
   - Overview of the 4 responsive viewport ranges (`small`, `medium`, `large`, `desktop`) and Bootstrap 5 standard defaults (`0 / 768 / 1200 / 1400` px).
   - Multi-column desktop layout (`desktop_columns`, `column = 1..5 | full`) and automatic `max(1, desktop.items_per_row // desktop_columns)` sizing.
   - Balanced row packing (`balance_rows = true`) with visual packing table and when to disable it per section (`balance_rows = false`).
   - Section-level `items_per_row` (plus per-range `items_per_row_small..desktop`) with copy-pasteable recipes (full-width maximized map/chart, 2-wide charts, 3-day forecast, 4-card overview, 4K/ultrawide tuning).
   - Upgrade & WeeWX restart notes.
2. Update **`README.md`** (preserving CRLF line endings):
   - Highlight the configurable responsive breakpoints, balanced row packing, and per-section grid sizing in the Key Features list and Quick Configuration section.
   - Add **`docs/LAYOUT-CONFIGURATION.md`** to the Documentation table.

---

## 7. GitHub PR Description (Copy-Ready)

````markdown
## Summary

Introduces a unified, `skin.conf`-driven responsive grid system with configurable viewport breakpoints, **balanced row packing**, section-level `items_per_row` overrides, and consistent sizing across all item types (removing the forecast layout special-case).

### What Changed

- **Unified Item Grid (Forecast Special-Case Removed)**
  - Removed `:not(:has(> .card-forecast))` in `_neowx-material.scss` and the card-vs-chart Bootstrap column split (`col-md-6` / `col-xl-4` / `col-xl-6`).
  - All item types (value cards, charts, customCharts, forecast, telemetry cards/charts, and embedded widgets) now use a single `.nwm-grid-item` wrapper driven by CSS custom properties (`--nwm-cols` / `--nwm-cols-eff`).
- **Configurable Viewport Breakpoints & Items Per Row (`[[Appearance]] -> [[[Layout]]]`)**
  - Added 4 named mobile-first viewport ranges with Bootstrap 5 standard defaults:
    - `small`: `0px` (`items_per_row = 1`)
    - `medium`: `768px` (`items_per_row = 2`)
    - `large`: `1200px` (`items_per_row = 3`)
    - `desktop`: `1400px` (`items_per_row = 4`, activates `desktop_columns = 2..5`)
  - Multi-column desktop bands (`desktop_columns = 2..5`) automatically derive `max(1, desktop.items_per_row // desktop_columns)` items per column.
- **Balanced Row Packing (`balance_rows = true`)**
  - Added `nwmBalanceGrid()` in `js/app.js` (runs synchronously before ApexCharts/Leaflet init, re-evaluates only on `matchMedia` breakpoint crossings) using `effective_cols = ceil(N / ceil(N / C))`.
  - Eliminates awkward orphan rows on resize or smaller sections (e.g., 4 items at max 3/row pack cleanly as 2+2 at 50% instead of 3+1; 2 items scale to 50%/50%; 3-day forecast in a 4-slot band fills 3×33.3%) while keeping incomplete final rows aligned to the exact same width as rows above (`flex-grow: 0`).
  - Can be toggled globally under `[[[Layout]]]` or overridden per section (`balance_rows = true | false`).
- **Section-Level Grid Overrides (`[[[sections]]]`)**
  - Added `items_per_row` (with smart responsive clamping on smaller viewports) and optional per-range `items_per_row_small` / `_medium` / `_large` / `_desktop` settings so individual sections can maximize a single full-width item (`items_per_row = 1`), spread 2 wide items (`items_per_row = 2`), or display 4 cards (`items_per_row = 4`) on the same page.
- **Zero-Breakage Upgrade Fallback & Validation (`panelorder.py` v2.7.0)**
  - Strictly validates numeric bounds and sanitized section slugs before emitting `<style id="nwm-layout-css">` (`$panelLayoutCSS()`) and `$panelLayoutConfigJSON()`.
  - Keeps zero-specificity `:where(...)` default rules in SCSS so pages render cleanly even before WeeWX is restarted.
- **Documentation**
  - Added `docs/LAYOUT-CONFIGURATION.md` and updated `README.md`.

### Testing & Verification

- [ ] Python unit tests for `panelorder.py` (`parse_layout_config`, `generate_layout_css`, `generate_layout_config_json`, bounds/slug validation)
- [ ] JS unit tests for `nwmBalanceGrid()` across `(N, C)` combinations, divider splitting, per-section overrides, and `matchMedia` updates
- [ ] `cd skins/neowx-material && yarn run build` compiles cleanly
- [ ] Preserved CRLF line endings on legacy archive templates and `README.md`
- [ ] Visual check across `small`, `medium`, `large`, and `desktop` viewports (including forecast, mixed full-width sections, and multi-column bands)
````

---

## 8. GitHub Release Notes (Copy-Ready)

````markdown
### New Features

- **Configurable Responsive Layout Breakpoints (`[[Appearance]] -> [[[Layout]]]`)**: Customize the 4 viewport breakpoints (`small`, `medium`, `large`, `desktop`) and maximum `items_per_row` directly in `skin.conf` without editing CSS. Defaults now follow modern Bootstrap 5 breakpoints (`0 / 768 / 1200 / 1400` px with `1 / 2 / 3 / 4` items across), enabling the multi-column desktop layout earlier on 14"/15" laptops and 125%-scaled 1080p displays.
- **Balanced Row Packing (`balance_rows`)**: Automatically packs rows evenly as screen widths change or sections contain fewer items (for example, 4 items in a 3-per-row viewport render as 2+2 at 50% instead of 3+1, and 2 items in a 3- or 4-slot row scale to 50%/50%), while keeping uneven final rows (such as 3+2) locked to the exact same card width as the row above. Enabled by default (`balance_rows = true`) and overridable globally or per section.
- **Section-Level `items_per_row` Overrides**: Configure `items_per_row` (and optional `items_per_row_small`..`_desktop`) on any section in `[[[sections]]]` to mix layouts on the same page — e.g., a maximized single full-width earthquake map or chart (`column = full`, `items_per_row = 1`) right above a 2-chart band (`items_per_row = 2`) and a 4-card overview (`items_per_row = 4`).
- **Unified Sizing for Forecast & All Item Types**: Removed the special-case forecast width restriction and card-vs-chart grid split — value cards, charts, forecast cards, telemetry gauges, and embedded widgets now follow the exact same responsive layout rules.

### Documentation

- Added the [Layout Configuration Guide](docs/LAYOUT-CONFIGURATION.md) with breakpoint references, balanced row packing tables, and copy-pasteable `skin.conf` recipes.

### Upgrade Notes

- Restart WeeWX after upgrading so the updated `panelorder.py` (`v2.7.0`) extension is loaded (existing default layouts continue to render via CSS fallbacks until restart).
- Existing `skin.conf` files without a `[[[Layout]]]` section automatically inherit the new defaults (`0 / 768 / 1200 / 1400` px, `1 / 2 / 3 / 4` items, `balance_rows = true`). To keep the previous `1660px` desktop threshold, set `min_width = 1660` under `[[Appearance]] -> [[[Layout]]] -> [[[[desktop]]]]`.
````
