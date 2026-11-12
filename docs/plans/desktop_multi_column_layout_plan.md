# Implementation Plan: Multi-Column Desktop Layout (`v2.x`)

## 1. Goal & Requirement Analysis

In **v1.x**, wide desktop screens (`>= 1660px`) rendered two side-by-side columns using `.temprow` (left 50%: value cards) and `.graphrow` (right 50%: charts).
In **v2.0**, the layout was unified into a single sequential flow of sections (`[[[pages]]]` & `[[[sections]]]`), where all sections stack vertically across the full width (`100%`), resulting in a very long page on wide / 4K monitors.

The goal is to combine the flexibility of v2's section and collapsible panel engine with a multi-column desktop layout (`desktop_columns = 1..5`, **default `2`**) on wide screens (`min-width: 1660px`), including support for **full-width spanning sections** (e.g., embedded weather maps, webcams, or wide panels across all columns), while preserving full responsive stacking on smaller screens (`< 1660px`).

---

## 2. Decisions & Recommendations

### Decided
1. **Configuration Key Name:** Use **`desktop_columns`** (under `[Extras] -> [[Appearance]]`, with optional per-page override in `[[[pages]]]`).
2. **Default Value:** **`desktop_columns = 2`** (restores the popular v1 side-by-side desktop layout out of the box on screens `>= 1660px`, while `desktop_columns = 1` gives the single-column vertical flow).

### Recommended (Pending Final Sign-Off)
3. **[RECOMMENDED] Column Distribution Strategy:** **Option D (Hybrid Resolution)** — zero-config automatic Card-Left / Chart-Right split when `desktop_columns = 2`, plus optional per-section `column` and per-page `column1..5` overrides.
4. **[RECOMMENDED] Full-Width Spanning Sections (`column = full`):** Allow any section (such as `embedded_items`, weather maps, or custom sections) to span `100%` of the desktop width across all columns via `column = full` (with `full_width = true` or `span = full` as intuitive aliases).

---

## 3. Distributing Content to Columns (Recommended: Option D Hybrid)

In v2, sections are arbitrary blocks (`card`, `chart`, `telemetry`, `telemetry_chart`, or `embedded` items) listed under `[[[pages]]]`.

### Option A: Explicit Page Columns (`column1`, `column2`, ...) in `[[[pages]]]`
In `[[[pages]]]`, a page can explicitly assign sections to `column1 .. column5`:
```ini
[[[pages]]]
    [[[[today]]]]
        sections = cards_with_forecast, additional_cards, external_sensors_cards, charts, additional_charts, external_sensor_charts, embedded_items

        # Optional explicit column assignment when desktop_columns >= 2:
        column1 = cards_with_forecast, additional_cards, external_sensors_cards
        column2 = charts, additional_charts, external_sensor_charts
```

### Option B: Automatic Content-Type Split (Classic v1 Behavior for 2 Columns)
When `desktop_columns = 2` is active and no explicit `column1`/`column2` is specified:
- **Column 1 (Left):** All value/card sections (`content = card` and `content = telemetry`) in their configured page order.
- **Column 2 (Right):** All chart sections (`content = chart` and `content = telemetry_chart`) in their configured page order.

### Option C: Per-Section `column = 1..5 | full` Attribute in `[[[sections]]]`
- Any section in `[[[sections]]]` can set `column = 1..5` to pin itself to a specific column, or `column = full` to span the full width across all columns.

### ⭐ [RECOMMENDED] Option D: Hybrid Resolution (Combines A + B + C)
1. Determine active column count `N` (`1..5`, **default `2`**) from `subpage -> page -> [[Appearance]] desktop_columns`.
2. If `N == 1`: render all `page_segments` in a single full-width flow.
3. If `N >= 2`:
   - **Priority 1 (Explicit Page Columns):** If the page/subpage defines `column1 .. columnN`, resolve those columns directly (and any section with `column = full` in `sections` renders full-width before/after the column block according to its position in `sections`).
   - **Priority 2 (Per-Section `column` key):**
     - `column = full` (or `all` / `full_width = true`): Section breaks out of the multi-column split and spans `100%` across all columns!
     - `column = 1..N`: Section is placed in column `X`.
   - **Priority 3 (Auto-Distribution for remaining unpinned sections):**
     - When `N == 2`: Card/telemetry sections (`content in ('card', 'telemetry')`) go to **Column 1 (Left)**; Chart sections (`content in ('chart', 'telemetry_chart')`) go to **Column 2 (Right)**.
     - When `N >= 3`: Distribute unpinned sections across columns `1..N` in declaration order (`idx % N`).
   - Strip any empty columns in a multi-column band (e.g. if a page only has cards and no charts, don't leave a blank 50% right column).

---

## 4. Evaluation & Proposal: Full-Width Spanning Sections (`column = full`)

### Use Case
A user has `desktop_columns = 2` (cards on the left, charts on the right), plus an `embedded_items` section (Windy map, RainViewer radar, Blitzortung, or wide webcam images) or a top banner/summary section that looks much better spanning the **entire desktop width (`100%`)** across both columns rather than being squeezed into `50%` of one column.

### How Full-Width Sections Fit into the Layout Architecture ("Layout Bands")
Instead of forcing the entire page into a single static N-column wrapper from top to bottom, `panelLayout(page=$page, subpage=$subpage)` in `bin/user/panelorder.py` groups the page's ordered sections into **Layout Bands**:

1. **Multi-Column Band (`type = "columns"`):**
   - Consecutive non-full-width sections are grouped into `1..N` side-by-side columns (`.nwm-layout-col`).
   - For example, `cards_with_forecast`, `additional_cards`, `charts`, `additional_charts` form a 2-column band where cards sit in Column 1 (left) and charts sit in Column 2 (right).
2. **Full-Width Band (`type = "full"`):**
   - Whenever a section has `column = full` (e.g., `embedded_items` at the bottom of the page, or a full-width section at the top or middle of `sections`), it emits a full-width (`100%`) row across the entire container!
   - Inside a `full` band, the full desktop width is available, so cards and embedded maps use the spacious full-width grid (`3` cards/maps per row via `col-xl-4`, or `2` charts per row via `col-xl-6`).
   - If another set of column sections appears *after* a full-width section in `sections`, a new multi-column band starts below it seamlessly.

### Visual Diagram (`desktop_columns = 2` with `embedded_items` set to `column = full`)
```
+-----------------------------------+-----------------------------------+
| COLUMN 1 (Left 50%)               | COLUMN 2 (Right 50%)              |
| - cards_with_forecast             | - charts (Custom Charts)          |
| - [Panel] Additional Sensors      | - [Panel] Additional Charts       |
| - [Panel] External Sensors        | - [Panel] External Sensor Charts  |
+-----------------------------------+-----------------------------------+
| FULL-WIDTH SECTION (100% across both columns)                         |
| - [Panel] Embedded Items (iFrame_windy, iFrame_lightnings, radar...)  |
+-----------------------------------------------------------------------+
```

### Proposed Configuration for Full-Width Sections (`skin.conf`)

Using the existing `column` key on any section in `[[[sections]]]`, we accept `column = full` (as well as `column = all` or `full_width = true` as convenience aliases):

```ini
        [[[sections]]]
            [[[[cards_with_forecast]]]]
                items = inTemp, outTemp, outHumidity, forecast, barometer, windSpeed, windrun, rain, inHumidity

            [[[[charts]]]]
                title = Custom Charts
                items = inTemp, outTemp, customChartOutTemp, customChartInTemp, customChartTemp, windchill, customChartBarometer, imageLogo
                content = chart

            [[[[embedded_items]]]]
                title = Embedded Items
                items = iFrame_windy, iFrame_lightnings, iFrame_rainviewer
                # Span the full desktop width across all columns (1..5 -> column, full -> 100% width):
                column = full
```
- **Shipped Default Suggestion:** In the default `skin.conf`, we can set `column = full` on `[[[[embedded_items]]]]` by default so that with `desktop_columns = 2`, the cards and charts sit side-by-side at the top and the 3 embedded weather maps (`iFrame_windy`, `iFrame_lightnings`, `iFrame_rainviewer`) sit cleanly in a 3-across full-width panel at the bottom!

---

## 5. Responsive Grid & Card Sizing Inside Multi-Column Layouts

When the screen is wide (`>= 1660px`) and `N` columns are rendered side-by-side in a multi-column band, each column has `100% / N` of the viewport width. Inside a full-width band (`column = full`), the section always has `100%` of the viewport width.

Proposed SCSS scaling (`@media screen and (min-width: 1660px)`) using wrapper `.nwm-desktop-cols-$N` and `.nwm-layout-col`:

| Context (`>= 1660px`) | Column Width | Value / Embedded Cards per Row | Charts per Row |
| :--- | :--- | :--- | :--- |
| **`desktop_columns = 1` or `column = full`** | `100%` | 3 cards (`col-xl-4` = 33.3%) | 2 charts (`col-xl-6` = 50%) |
| **`desktop_columns = 2` (Classic v1)** | `50%` | 2 cards (`50%` each, matching v1 `.temprow`) | 2 charts (`50%` each, matching v1 `.graphrow`) |
| **`desktop_columns = 3`** | `33.333%` | 2 cards (`50%` each) | 1 chart (`100%` full column width) |
| **`desktop_columns = 4`** | `25%` | 1 card (`100%` full column width) | 1 chart (`100%` full column width) |
| **`desktop_columns = 5`** | `20%` | 1 card (`100%` full column width) | 1 chart (`100%` full column width) |

- **Below `1660px` (`< 1660px`):**
  - `.nwm-layout-col` stacks vertically at `100%` width (`flex: 0 0 100%; max-width: 100%`), so tablets, laptops, and mobile devices keep the exact existing responsive layout.
- **Collapsible Expansion Panels (`nwm-expansion-panel`):**
  - Panels inside a column expand/collapse within that column (`50%` width in 2-col mode).
  - Panels on a `column = full` section expand/collapse across the full `100%` width.
  - Panel IDs (`nwm-val-panel-$panel_idx`, `nwm-chart-panel-$panel_idx`) and `data-section` slugs remain globally unique across the page so `rememberPanelState` in `sessionStorage` works without changes.

---

## 6. Proposed Configuration Syntax (`skin.conf`)

Under `[Extras] -> [[Appearance]]`:

```ini
    [[Appearance]]
        # Number of side-by-side layout columns on wide desktop screens (>= 1660px).
        #   1    - single-column vertical section flow
        #   2    - (default) classic two-column split: value cards on the left,
        #          charts on the right (v1 style)
        #   3..5 - multi-column dashboard layout for ultra-wide / 4K monitors
        #
        # Can also be overridden per page under [[[pages]]] (e.g. desktop_columns = 1).
        # Individual sections under [[[sections]]] can set:
        #   column = 1..5   (pin section to a specific column)
        #   column = full   (span the full desktop width across all columns)
        desktop_columns = 2
```

---

## 7. Step-by-Step Implementation Plan

### Step 1: Backend Layout Parser (`bin/user/panelorder.py`)
1. Bump `VERSION` in `bin/user/panelorder.py` (`2.4.0` -> `2.5.0`).
2. Update `SETTING_KEYS` to include `"column"` (accepting `1..5`, `"full"`, `"all"`) and `"full_width"` (`true`/`false`).
3. Update `PAGE_SETTING_KEYS` to include `"desktop_columns"` and `"column1" .. "column5"`.
4. Implement `parse_layout(skin_dict, enable_panels=True, page=None, subpage=None)`:
   - Resolves `desktop_columns` from `subpage -> page -> [[Appearance]]`, clamped to `1..5` (default **`2`**).
   - Returns a list of layout bands:
     - `{"type": "full", "segments": [seg, ...]}` for `desktop_columns == 1` or sections marked `column = full`.
     - `{"type": "columns", "columns": [{"index": 1, "segments": [...]}, ...]}` for multi-column bands.
   - Ensures `order_items()` / `panelItems()` includes all sections from both `sections` and any `column1..N` overrides so chart JS generation never misses a chart.
5. Expose `panelLayout` in `PanelOrder.get_extension_list()` (keeping `panelSegments` intact for backwards compatibility).

### Step 2: Template Loop Update (`skins/neowx-material/*.html.tmpl`)
1. Update the 6 panel-enabled page templates (`index.html.tmpl`, `day-%Y-%m-%d.html.tmpl`, `week-%Y-%V.html.tmpl`, `month-%Y-%m.html.tmpl`, `year-%Y.html.tmpl`, `telemetry.html.tmpl`) — preserving native LF / CRLF line endings.
2. Render `$panelLayout(page=$page, subpage=$subpage)`:
   - For a `"full"` band: render segments directly inside `<div class="row mt-3 align-content-start">` (or `<div class="col-12 nwm-layout-full"><div class="row align-content-start">...</div></div>`).
   - For a `"columns"` band: render `<div class="col-12"><div class="row align-content-start nwm-desktop-cols-$len($band['columns'])">` containing `<div class="col-12 nwm-layout-col" data-column="$col['index']"><div class="row align-content-start">...</div></div>`.

### Step 3: SCSS Multi-Column Responsive Rules (`skins/neowx-material/scss/_neowx-material.scss`)
1. Replace/extend the legacy `@media screen and (min-width: 1660px)` `.temprow` / `.graphrow` block in `_neowx-material.scss` with rules for `.nwm-desktop-cols-2` through `.nwm-desktop-cols-5`:
   - Set `.nwm-layout-col` flex basis and `max-width` to `50%`, `33.3333%`, `25%`, `20%` at `min-width: 1660px`.
   - Adjust inner card/chart grid columns inside `.nwm-desktop-cols-2..5` so cards and charts scale gracefully inside narrower columns while `.nwm-layout-full` keeps full-width sizing.
2. Compile CSS via `cd skins/neowx-material && yarn run build`.

### Step 4: Configuration & Documentation (`skin.conf`, `docs/CONFIGURATION.md`)
1. Set `desktop_columns = 2` in `skins/neowx-material/skin.conf` and add `column = full` to `[[[[embedded_items]]]]`.
2. Document `desktop_columns`, `column = 1..5 | full`, and `column1..5` in `skin.conf` and `docs/CONFIGURATION.md`.
