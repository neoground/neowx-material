# Implementation Plan: Multi-Column Desktop Layout (`v2.x`)

## 1. Goal & Requirement Analysis

In **v1.x**, wide desktop screens (`>= 1660px`) rendered two side-by-side columns using `.temprow` (left 50%: value cards) and `.graphrow` (right 50%: charts).
In **v2.0**, the layout was unified into a single sequential flow of sections (`[[[pages]]]` & `[[[sections]]]`), where all sections stack vertically across the full width (`100%`), resulting in a very long page on wide / 4K monitors.

The goal is to combine the flexibility of v2's section and collapsible panel engine with an optional multi-column desktop layout (`desktop_columns = 1..5`) on wide screens (`min-width: 1660px`), while preserving full responsive stacking on smaller screens (`< 1660px`).

---

## 2. Decisions & Open Items

### Decided
1. **Configuration Key Name:** Use **`desktop_columns`** (under `[Extras] -> [[Appearance]]`, with optional per-page override in `[[[pages]]]`).

### Open (To Be Decided)
2. **[OPEN] Column Distribution Strategy:** How sections are distributed across columns when `desktop_columns = 2..5` (see the options in Section 3 below).
3. **[OPEN] Card & Chart Grid Ratios Inside Columns (`>= 1660px`):** Confirming the number of cards/charts per row inside `2..5` columns (see Section 4 below).

---

## 3. [OPEN] Options for Distributing Content to Columns

In v2, sections are arbitrary blocks (`card`, `chart`, `telemetry`, `telemetry_chart`, or `embedded` items) listed under `[[[pages]]]`. Below are the candidate ways to distribute sections into `N` columns:

### Option A: Explicit Page Columns (`column1`, `column2`, ...) in `[[[pages]]]`
In `[[[pages]]]`, a page can explicitly assign sections to `column1 .. column5`:
```ini
[[[pages]]]
    [[[[today]]]]
        # Used when desktop_columns = 1:
        sections = cards_with_forecast, additional_cards, external_sensors_cards, charts, additional_charts, external_sensor_charts, embedded_items

        # Explicit column assignment when desktop_columns >= 2:
        column1 = cards_with_forecast, additional_cards, external_sensors_cards, embedded_items
        column2 = charts, additional_charts, external_sensor_charts
```
- **Pros:** Zero ambiguity; the user sees the exact top-to-bottom order of every column per page right in `[[[pages]]]`.

### Option B: Automatic Content-Type Split (Classic v1 Behavior for 2 Columns)
When `desktop_columns = 2` is set globally (or per page), automatically split the page's `sections` list by `content` type:
- **Column 1 (Left):** All value/card sections (`content = card` and `content = telemetry`) in their configured page order.
- **Column 2 (Right):** All chart sections (`content = chart` and `content = telemetry_chart`) in their configured page order.
- **Pros:** Turning on the classic v1 two-column layout across the entire site requires changing **only one line** in `skin.conf` (`desktop_columns = 2`), without rewriting `[[[pages]]]` or `[[[sections]]]`.

### Option C: Per-Section `column = 1..5` Attribute in `[[[sections]]]` (or Sequential Round-Robin)
- Any section in `[[[sections]]]` can set `column = 1..5` to pin itself to a specific column.
- Unpinned sections are distributed sequentially (`idx % N`) or by content type.

### Option D: Hybrid Resolution (Combines A + B + C)
In `bin/user/panelorder.py`, expose `$panelColumns(page=$page, subpage=$subpage)` returning `[{"index": 1, "segments": [...]}, ...]`:
1. Determine active column count `N` (`1..5`, default `1`) from page/subpage `desktop_columns` override -> global `[[Appearance]] desktop_columns`.
2. If `N == 1`: return a single column containing all `page_segments` (100% identical to current v2.0.1 output).
3. If `N >= 2`:
   - **Priority 1 (Explicit Page Columns):** If the page/subpage defines `column1 .. columnN`, resolve each column's section list directly.
   - **Priority 2 (Per-Section `column` key):** If any active section on the page defines `column = 1..N`, place it in that column.
   - **Priority 3 (Auto-Distribution for remaining sections):**
     - When `N == 2`: Card/telemetry sections (`content in ('card', 'telemetry')`) go to **Column 1**; Chart sections (`content in ('chart', 'telemetry_chart')`) go to **Column 2** (exact v1 classic behavior).
     - When `N >= 3`: Distribute sections across columns `1..N` in declaration order (`idx % N`).
   - Strip any empty columns (e.g. if a page only has cards and no charts, don't leave a blank 50% right column).

---

## 4. Responsive Grid & Card Sizing Inside Multi-Column Layouts

When the screen is wide (`>= 1660px`) and `N` columns are rendered side-by-side, each column has `100% / N` of the viewport width. Inside a narrower column, Bootstrap's standard `.col-xl-4` (3 value cards per row) or `.col-xl-6` (2 charts per row) would make cards and charts too cramped unless adjusted for the column width—just like v1 did with `.temprow .col-xl-4 { max-width: 50% }`.

Proposed SCSS scaling (`@media screen and (min-width: 1660px)`) using wrapper `.nwm-desktop-cols-$N` and `.nwm-layout-col`:

| `desktop_columns` (`N`) | Column Width (`>=1660px`) | Value Cards per Row in Column | Charts per Row in Column |
| :--- | :--- | :--- | :--- |
| **`1` (Default v2)** | `100%` | 3 cards (`col-xl-4` = 33.3%) | 2 charts (`col-xl-6` = 50%) |
| **`2` (Classic v1)** | `50%` | 2 cards (`50%` each, matching v1 `.temprow`) | 2 charts (`50%` each, matching v1 `.graphrow`) |
| **`3` Columns** | `33.333%` | 2 cards (`50%` each) | 1 chart (`100%` full column width) |
| **`4` Columns** | `25%` | 1 card (`100%` full column width) | 1 chart (`100%` full column width) |
| **`5` Columns** | `20%` | 1 card (`100%` full column width) | 1 chart (`100%` full column width) |

- **Below `1660px` (`< 1660px`):**
  - `.nwm-layout-col` stacks vertically at `100%` width (`flex: 0 0 100%; max-width: 100%`), so tablets, laptops, and mobile devices keep the exact existing responsive layout.
- **Collapsible Expansion Panels (`nwm-expansion-panel`):**
  - Panels sit *inside* each `.nwm-layout-col` (`col-12 mb-3`), so each column can have its own independent collapsible panels (e.g., "Additional Sensors" collapsing on the left while "Custom Charts" stays open on the right).
  - Panel IDs (`nwm-val-panel-$panel_idx`, `nwm-chart-panel-$panel_idx`) and `data-section` slugs remain globally unique across the page so `rememberPanelState` in `sessionStorage` works without changes.

---

## 5. Proposed Configuration Syntax (`skin.conf`)

Under `[Extras] -> [[Appearance]]`:

```ini
    [[Appearance]]
        # Number of side-by-side layout columns on wide desktop screens (>= 1660px).
        #   1    - (default) single-column vertical section flow (v2 default)
        #   2    - classic two-column split: value cards on the left, charts on the right (v1 style)
        #   3..5 - multi-column dashboard layout for ultra-wide / 4K monitors
        # Can also be overridden per page under [[[pages]]] (e.g. desktop_columns = 2).
        desktop_columns = 1
```

---

## 6. Step-by-Step Implementation Plan

### Step 1: Backend Layout Parser (`bin/user/panelorder.py`)
1. Bump `VERSION` in `bin/user/panelorder.py` (`2.4.0` -> `2.5.0`).
2. Add `"desktop_columns"` (and any chosen column assignment keys, e.g. `"column1" .. "column5"` / `"column"`) to `PAGE_SETTING_KEYS` / `SETTING_KEYS`.
3. Implement `parse_columns(skin_dict, enable_panels=True, page=None, subpage=None)`:
   - Resolves `desktop_columns` from `subpage -> page -> [[Appearance]]`, clamped to `1..5` (default `1`).
   - Distributes sections into columns according to the selected strategy from Section 3.
4. Expose `panelColumns` in `PanelOrder.get_extension_list()`.

### Step 2: Template Loop Update (`skins/neowx-material/*.html.tmpl`)
1. Update the 6 panel-enabled page templates (`index.html.tmpl`, `day-%Y-%m-%d.html.tmpl`, `week-%Y-%V.html.tmpl`, `month-%Y-%m.html.tmpl`, `year-%Y.html.tmpl`, `telemetry.html.tmpl`) — preserving native LF / CRLF line endings.
2. When `len($page_columns) <= 1`, emit the exact current `<div class="row mt-3 align-content-start">...</div>` markup.
3. When `len($page_columns) > 1`, emit:
   ```html
   <div class="row mt-3 align-content-start nwm-desktop-cols-$len($page_columns)">
       #for $col in $page_columns
           <div class="col-12 nwm-layout-col" data-column="$col['index']">
               <div class="row align-content-start">
                   ... (existing segment loop for $col['segments']) ...
               </div>
           </div>
       #end for
   </div>
   ```

### Step 3: SCSS Multi-Column Responsive Rules (`skins/neowx-material/scss/_neowx-material.scss`)
1. Replace/extend the legacy `@media screen and (min-width: 1660px)` `.temprow` / `.graphrow` block in `_neowx-material.scss` with rules for `.nwm-desktop-cols-2` through `.nwm-desktop-cols-5`:
   - Set `.nwm-layout-col` flex basis and `max-width` to `50%`, `33.3333%`, `25%`, `20%` at `min-width: 1660px`.
   - Adjust inner card/chart grid columns (`.card-value`, `.card-forecast`, `.card-embedded`, `.card-chart` parent `.col-*`) inside `.nwm-desktop-cols-2..5` so cards and charts scale gracefully inside narrower columns.
2. Compile CSS via `cd skins/neowx-material && yarn run build`.

### Step 4: Configuration & Documentation (`skin.conf`, `docs/CONFIGURATION.md`)
1. Document `desktop_columns` in `skins/neowx-material/skin.conf` above `[[[pages]]]`.
2. Update `docs/CONFIGURATION.md` with examples for the multi-column desktop layout.
