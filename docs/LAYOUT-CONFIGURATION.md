# Responsive Layout & Grid Configuration Guide

NeoWX Material provides a unified, `skin.conf`-driven responsive grid system that lets you configure:

- **4 responsive viewport breakpoints** (`small`, `medium`, `large`, `desktop`) and how many items appear per row in each range
- **Multi-column desktop bands** (`desktop_columns = 1..5` and full-width `column = full` spanning sections)
- **Balanced row packing** (`balance_rows = true`) so cards, charts, forecasts, telemetry gauges, and embedded widgets automatically distribute cleanly across rows without awkward orphan cards
- **Section-level overrides** (`items_per_row` and per-range `items_per_row_*`) to mix single full-width items, 2-wide panels, and dense 4-card rows on the same page

All item types — **value cards, charts, customCharts, forecast cards, telemetry gauges/charts, and embedded widgets (iFrames, images, earthquake maps)** — follow the exact same grid rules.

---

## 1. Responsive Breakpoints (`[[Appearance]] -> [[[Layout]]]`)

Configure global breakpoints and item counts under `[Extras] -> [[Appearance]] -> [[[Layout]]]` in `skins/neowx-material/skin.conf`:

```ini
[Extras]
    [[Appearance]]
        desktop_columns = 2

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

### Default Ranges (Bootstrap 5 Standards)

| Range | Default `min_width` | Default `items_per_row` | Layout Columns | Typical Devices |
| :--- | :---: | :---: | :---: | :--- |
| `small` | `0px` | `1` | 1 (stacked) | Phones (`360–430px`) |
| `medium` | `768px` | `2` | 1 (stacked) | Tablets portrait (`768–834px`), split windows |
| `large` | `1200px` | `3` | 1 (stacked) | Tablets landscape, 13" laptops (`1280px`) |
| `desktop` | `1400px` | `4` | `desktop_columns` (`1..5`) | 14"/15" laptops (`1440–1512px`), 1080p @ 125% (`1536px`), FHD/QHD/4K |

> [!IMPORTANT]
> `min_width` values must be strictly ascending (`small < medium < large < desktop`, within `0..10000` px) and `items_per_row` values must be integers between `1` and `12`. Invalid or out-of-order values log a warning in WeeWX and safely fall back to the defaults above.

> [!NOTE]
> `[[[Layout]]]`, `[[[pages]]]`, and `[[[sections]]]` are ConfigObj subsections under `[[Appearance]]`. Keep all plain `[[Appearance]]` scalar settings (such as `enablePanels` and `desktop_columns`) **above** `[[[Layout]]]`.

---

## 2. Multi-Column Desktop Bands (`desktop_columns`)

When the viewport reaches **`Layout.desktop.min_width`** (default `1400px`), the layout switches from 1 stacked column to `desktop_columns` (`1..5`) side-by-side columns.

Each column in a multi-column band of `k` columns automatically calculates its maximum items per row as:

```
items_per_column = max(1, desktop.items_per_row // k)
```

| `desktop.items_per_row` | `column = full` / `cols = 1` | `cols = 2` | `cols = 3` | `cols = 4` | `cols = 5` |
| :---: | :---: | :---: | :---: | :---: | :---: |
| **`4`** *(default)* | `4` | `2` per col (4 across) | `1` per col | `1` per col | `1` per col |
| **`6`** *(ultrawide / 4K)* | `6` | `3` per col (6 across) | `2` per col (6 across) | `1` per col | `1` per col |
| **`8`** *(wide 4K)* | `8` | `4` per col | `2` per col | `2` per col | `1` per col |

This guarantees that items inside a full-width band and items inside a 2-column band stay visually consistent in width without needing separate per-column configuration keys.

---

## 3. Balanced Row Packing (`balance_rows`)

When **`balance_rows = true`** (enabled by default under `[[[Layout]]]`), each contiguous group of visible items in a section automatically computes an even column count so rows don't end with awkward single-card orphans when switching breakpoints or when a section has fewer items:

```
rows          = ceil(N / C)
effectiveCols = ceil(N / rows)
```

Where `N` is the number of visible items in the group and `C` is the configured max `items_per_row` for the current viewport range.

Because CSS keeps `flex-grow: 0`, any final row that cannot divide evenly (such as 5 items at max 3/row → `3 + 2`) strictly keeps the exact same card width and aspect ratio as the rows above it.

### Packing Behavior Table

| Visible Items (`N`) | Max `C = 2` | Max `C = 3` | Max `C = 4` |
| :---: | :--- | :--- | :--- |
| **1** | `1` (100%) | `1` (100%) | `1` (100%) |
| **2** | `2` (50%) | `2` (50%) | `2` (50%) |
| **3** | `2` (`2 + 1` @ 50%) | `3` (33.3%) | `3` (33.3% — e.g. 3-day forecast fills 100%) |
| **4** | `2` (`2 + 2` @ 50%) | `2` (**`2 + 2` @ 50%**, avoids `3 + 1`) | `4` (25%) |
| **5** | `2` (`2 + 2 + 1` @ 50%) | `3` (`3 + 2` @ 33.3%) | `3` (**`3 + 2` @ 33.3%**, avoids `4 + 1`) |
| **6** | `2` (`2 + 2 + 2` @ 50%) | `3` (`3 + 3` @ 33.3%) | `3` (**`3 + 3` @ 33.3%**, avoids `4 + 2`) |
| **7** | `2` (`2 + 2 + 2 + 1` @ 50%) | `3` (`3 + 3 + 1` @ 33.3%) | `4` (`4 + 3` @ 25%) |
| **9** | `2` (`4×2 + 1` @ 50%) | `3` (`3 + 3 + 3` @ 33.3%) | `3` (**`3 + 3 + 3` @ 33.3%**, avoids `4 + 4 + 1`) |

### Disabling Balanced Row Packing

You can turn off row balancing globally (to enforce strict fixed column widths everywhere) or on individual sections (for example, to align a 2-card section vertically underneath a 4-card section):

```ini
[[[Layout]]]
    balance_rows = false
```

Or per section:

```ini
[[[sections]]]
    [[[[aligned_two_cards]]]]
        items = inTemp, inHumidity
        title = Indoor
        column = full
        items_per_row = 4
        balance_rows = false
```

---

## 4. Section-Level Grid Overrides (`[[[sections]]]`)

Any section under `[[Appearance]] -> [[[sections]]]` (titled panel or untitled loose section, in either `column = full` or `column = 1..5`) supports:

| Key | Values | Behavior |
| :--- | :--- | :--- |
| `items_per_row` | `1..12` | Sets the section's target max items per row. Automatically clamped to `min(N, range.items_per_row)` on `small`, `medium`, and `large` (so phones never squash 4 items side-by-side) and applied at exact `N` on `desktop`. |
| `items_per_row_small` | `1..12` | Exact override for the `small` range. |
| `items_per_row_medium` | `1..12` | Exact override for the `medium` range. |
| `items_per_row_large` | `1..12` | Exact override for the `large` range. |
| `items_per_row_desktop` | `1..12` | Exact override for the `desktop` range. |
| `balance_rows` | `true` / `false` | Overrides global `Layout.balance_rows` for this section only. |

---

## 5. Configuration Recipes

### Mix Full-Width Maximized, 2-Wide, and 4-Wide Sections on One Page

```ini
[Extras]
    [[Appearance]]
        desktop_columns = 2

        [[[pages]]]
            [[[[today]]]]
                sections = overview_four, featured_charts, earthquake_full, cards_with_forecast, charts

        [[[sections]]]
            # 4 cards across 100% width on desktop (25% each)
            [[[[overview_four]]]]
                items = outTemp, outHumidity, barometer, rain
                title = Station Overview
                column = full
                items_per_row = 4

            # 2 wide charts spread 50% / 50% across full width
            [[[[featured_charts]]]]
                items = customChartTemp, customChartBarometer
                title = Primary Charts
                content = chart
                column = full
                items_per_row = 2

            # Single maximized item spanning 100% full width
            [[[[earthquake_full]]]]
                items = earthquake_map
                title = Regional Earthquakes
                column = full
                items_per_row = 1
```

### Dedicated Full-Width Forecast Panel (3 Days)

With `balance_rows = true` (default), a 3-day forecast in a `column = full` section automatically fills 100% as `3 × 33.3%`. You can also pin it explicitly:

```ini
            [[[[forecast_panel]]]]
                items = forecast
                title = Forecast
                column = full
                items_per_row = 3
```

### Keep the Legacy `1660px` Desktop Breakpoint

If you prefer the previous `1660px` switchover threshold before entering 2-column desktop mode:

```ini
        [[[Layout]]]
            [[[[desktop]]]]
                min_width = 1660
                items_per_row = 4
```

### Ultra-Wide / 4K Monitor (3 Columns × 2 Items = 6 Across)

```ini
        desktop_columns = 3

        [[[Layout]]]
            [[[[desktop]]]]
                min_width = 1920
                items_per_row = 6
```

---

## 6. Upgrade Notes

- **Restart WeeWX after upgrading** so Python loads the updated `bin/user/panelorder.py` (`v2.7.0`), which generates the dynamic `<style id="nwm-layout-css">` rules and `window.NWM_LAYOUT_CONFIG` metadata.
- Even before restarting WeeWX, zero-specificity `:where(...)` fallback rules in `style.min.css` keep the Bootstrap 5 default layout (`0 / 768 / 1200 / 1400` px, `1 / 2 / 3 / 4` items) intact.
- Existing `skin.conf` files without a `[[[Layout]]]` block automatically use the defaults above.
