# Date & Time Formats Guide

A practical guide to controlling how dates and times appear on your NeoWX Material charts and cards.

Everything here lives in **`skin.conf`** under `[Extras] → [[Formatting]]` and (for charts)
`[[Appearance]]`. No template editing required. After any change, regenerate your report
(`weectl report run` or wait for the next archive cycle) and hard-refresh the page.

---

## Overview

- **Charts** (the x-axis labels and the hover tooltip) use **moment.js** format tokens - uppercase
  `DD`, `MM`, `YYYY`, `HH`, lowercase `mm` for minutes.
- **Cards** (the little "min/max at 14:32" time under a value) use **Python strftime** - `%d`, `%m`,
  `%Y`, `%H`, `%M`. Different language entirely.
- You can change formats at three levels for charts and one level for cards:

| What you want to change | Where |
|---|---|
| One custom chart's dates | `datetime_label_format` / `datetime_tooltip_format` on that chart |
| **Every** chart on a page (e.g. the whole Month page) | `datetime_graph_label_month` / `datetime_graph_tooltip_month` (keyed defaults) |
| Every **card** on a page | `[[[CardPageFormats]]]` |
| The global default for all charts | `datetime_graph_label` / `datetime_graph_tooltip` (already in your `skin.conf`) |

You can write a format **two ways**: define a **named constant** once and refer to it by name, or write
the **literal format string inline**. This works everywhere a format is set - the per-page keyed
defaults (Example 2), per-chart overrides (Example 1), and `CardPageFormats` (Example 3). Named
constants follow a convention:

- `datetime_custom_graph_*` → a **moment** string (for charts)
- `datetime_custom_card_*` → a **strftime** string (for cards)

Keeping the prefixes straight matters: feed a moment string to a card (or vice-versa) and you'll get
literal gibberish on the page. See *Named constant vs. literal* (below) for which to use when.

---

## Token Cheat-Sheet

**Charts (moment.js)** - examples: `dd DD HH:mm`, `DD.MM.YYYY`, `MMM`

| Token | Meaning | Example |
|---|---|---|
| `DD` | day of month, 2-digit | `07` |
| `Do` | suffixed day of month | `2nd` |
| `dd` / `ddd` / `dddd` | weekday short / medium / long | `Mo` / `Mon` / `Monday` |
| `MM` | month number | `03` |
| `MMM` / `MMMM` | month name short / long | `Mar` / `March` |
| `YY` / `YYYY` | year 2- / 4-digit | `26` / `2026` |
| `HH` | hour, 24h | `14` |
| `hh` | hour, 12h | `02` |
| `mm` | **minutes** (lowercase!) | `05` |
| `A` / `a` | AM/PM / am-pm marker | `PM` / `pm` |

> **Drop a leading zero** by using the single-letter token instead of the double: `D` `M` `H` `h` `m`
> (instead of `DD` `MM` `HH` `hh` `mm`). Example: `D.M.YYYY H:mm` → `7.3.2026 9:05`.

> ⚠️ In moment, **`MM` is the month** and **`mm` is minutes**. Writing `HH:MM` by mistake is so
> common that the skin auto-corrects `:MM` → `:mm` for you - but getting it right is easy too.

> This is a subset of all the moment.js date format codes.  The full list can be found 
> [here](https://momentjscom.readthedocs.io/en/latest/moment/04-displaying/01-format/)

**Cards (Python strftime)** - examples: `%H:%M`, `%a %d %H:%M`, `%d.%m.%Y %H:%M`

| Token | Meaning | Example |
|---|---|---|
| `%d` | day of month | `07` |
| `%a` / `%A` | weekday short / long | `Mon` / `Monday` |
| `%m` | month number | `03` |
| `%b` / `%B` | month name short / long | `Mar` / `March` |
| `%y` / `%Y` | year 2- / 4-digit | `26` / `2026` |
| `%H` | hour, 24h | `14` |
| `%I` | hour, 12h | `02` |
| `%M` | **minutes** | `05` |
| `%p` | AM/PM marker | `PM` |

> **Drop a leading zero** on Linux/glibc (the usual WeeWX host) by prefixing the token with `-`:
> `%-d` `%-m` `%-H` `%-I` (instead of `%d` `%m` `%H` `%I`). Example: `%-d.%-m.%Y %-H:%M` → `7.3.2026 9:05`.
> This is a GNU `strftime` extension and may not work on non-Linux hosts. It also does **not** apply to
> the live-updated header fields (`date` / `time` / `datetime` / `datetime_updated`) when MQTT is
> enabled - the live formatter doesn't understand `%-`, so use the padded tokens there. Cards and all
> static (non-live) renders are fine.

> **A note on regional ordering.** The examples in this guide use the European convention - day-first
> and a 24-hour clock (`DD.MM.YYYY HH:mm` for charts, `%d.%m.%Y %H:%M` for cards). US-style is equally
> supported - just swap the tokens to month-first with a 12-hour clock and an AM/PM marker, e.g.
> `ddd MM/DD/YYYY hh:mm A` (chart) or `%a %m/%d/%Y %I:%M %p` (card). Pick whichever you prefer; the
> skin doesn't care which ordering you use.

> This is a subset of all the strftime format codes.  The full list can be found 
> [here](https://docs.python.org/3/library/datetime.html#strftime-and-strptime-format-codes)
---

## Step 1 - Define your Named Formats

Add these under `[Extras] → [[Formatting]]` (next to the existing `datetime_graph_*` keys):

```ini
[Extras]
    [[Formatting]]
        # ... existing datetime_graph_label / _tooltip keys stay as-is ...

        # Chart (moment.js) named formats
        datetime_custom_graph_dayonly = ddd DD
        datetime_custom_graph_full    = ddd DD.MM.YYYY HH:mm
        datetime_custom_graph_month   = MMM

        # Card (strftime) named formats
        datetime_custom_card_short = %H:%M
        datetime_custom_card_full  = %a %d.%m.%Y %H:%M
```

> Avoid commas inside a format value - the config parser treats a comma as a list separator.
> `%a, %d %b` will misbehave; use `%a %d %b` instead or put the format in quotation marks:
> `"%a, %d %b"`.

Defining names is optional - you can also write a literal format inline. Names pay off when you reuse a
format (see the next section). You can reference these in the examples below.

---

## Named constant vs. literal — which to use

Per-chart overrides (`datetime_label_format` / `datetime_tooltip_format`) and `CardPageFormats` values
each accept **either**:

- a **named constant** you defined in `[[Formatting]]` (e.g. `datetime_custom_graph_full`), or
- a **literal** format string written inline (e.g. `ddd DD.MM.YYYY HH:mm`).

The skin first looks the value up as a `[[Formatting]]` key; if it isn't one, the value is used as a
literal format.

**Prefer a named constant when you'll use the same format in more than one place** - e.g. the same
"full" timestamp on several charts, or one card format across pages. Define it once and reference it
everywhere: to restyle them all you edit a single line, the name documents intent, and the copies can't
drift apart or pick up a typo independently.

**A literal is fine for a true one-off** - a single chart with a unique format you won't reuse. No point
defining a constant you'll reference once.

> Because an unknown value is treated as a literal, a typo'd constant name (or one you forgot to define)
> renders as literal text on the chart/card rather than silently falling back - a loud, easy-to-spot
> mistake. Define the constant before you reference it.

This includes the per-page keyed defaults in Example 2 (`datetime_graph_label_week` etc.) - their
shipped values are literals, but they accept a named constant just the same.

---

## Example 1 - Change One Custom Chart's Dates

On any `customChart*` in `[[Appearance]]`, add either or both keys. The value can be a **named
constant** or a **direct (literal) format** - here's one of each (they're equivalent).

**Named constant:**

```ini
[Extras]
    [[Appearance]]
        [[[customChartOutTemp]]]
            title     = Outdoor Temperature
            charttype = area
            values    = outTemp, dewpoint
            column    = avg
            datetime_label_format   = datetime_custom_graph_dayonly   # x-axis labels
            datetime_tooltip_format = datetime_custom_graph_full      # hover tooltip
```

**Direct (literal) format:**

```ini
[Extras]
    [[Appearance]]
        [[[customChartOutTemp]]]
            title     = Outdoor Temperature
            charttype = area
            values    = outTemp, dewpoint
            column    = avg
            datetime_label_format   = dd DD                  # x-axis labels
            datetime_tooltip_format = ddd DD.MM.YYYY HH:mm   # hover tooltip
```

Set just one if you only want to change the axis *or* the tooltip; anything you leave out keeps the
page's normal format.

### Vary it per page

Inside a custom chart you can override per page using the page sub-sections you already use for
`column` / `values`:

```ini
        [[[customChartOutTemp]]]
            charttype = area
            values    = outTemp, dewpoint
            column    = avg
            datetime_label_format = datetime_custom_graph_dayonly      # named constant - default for all pages
            [[[[week]]]]
                outTemp  = min, max
                datetime_label_format = ddd DD.MM.YYYY HH:mm           # literal - full detail, Week page only
```

Valid per-page sub-sections: `[[[[current]]]]` (the index/"current" page), `[[[[yesterday]]]]`,
`[[[[week]]]]`, `[[[[month]]]]`, `[[[[year]]]]`. (These follow the chart's per-page config scope, so the
index page is `current` here - distinct from the `day` scope used for per-page labels/tooltips and cards
below.)

---

## Example 2 - Change **Every** Chart on a Page

The skin selects chart axis labels and tooltips by page automatically via keyed defaults in
`[[Formatting]]`. To change the format for every chart on a specific page, set the matching key - with a
**named constant** or a **direct (literal) format**, one of each shown here.

**Named constant:**

```ini
[Extras]
    [[Formatting]]
        datetime_graph_label_month   = datetime_custom_graph_dayonly   # x-axis, every chart on month.html
        datetime_graph_tooltip_month = datetime_custom_graph_full      # tooltip, every chart on month.html
```

**Direct (literal) format:**

```ini
[Extras]
    [[Formatting]]
        datetime_graph_label_month   = ddd DD                 # x-axis, every chart on month.html
        datetime_graph_tooltip_month = ddd DD.MM.YYYY HH:mm   # tooltip, every chart on month.html
```

> The values shipped in `skin.conf` are literals (`ddd`, `DD`, …); either form works.

**How the keyed defaults map to pages:**

| Key | Pages it applies to |
|---|---|
| `datetime_graph_label` | base / fallback (telemetry) |
| `datetime_graph_label_day` | day (index + yesterday) |
| `datetime_graph_label_week` | week |
| `datetime_graph_label_month` | month |
| `datetime_graph_label_year` | year |
| `datetime_graph_tooltip` | base / fallback (telemetry) |
| `datetime_graph_tooltip_day` | day (index + yesterday) |
| `datetime_graph_tooltip_week` | week |
| `datetime_graph_tooltip_month` | month |
| `datetime_graph_tooltip_year` | year |

- A per-chart format (Example 1) still wins over the keyed default for that one chart.
- index and yesterday share the **`day`** scope (their template `$page` is `day`), so
  `datetime_graph_label_day` / `datetime_graph_tooltip_day` cover both. The `telemetry` page's
  built-in battery charts use the base `datetime_graph_label` / `datetime_graph_tooltip`.

---

## Example 3 - Change the Card Times

The small "high 24.3° at **14:32**" times under each value card are controlled by
`[[[CardPageFormats]]]`. Values are **strftime** - either a `datetime_custom_card_*` named constant or a
literal strftime string written inline (e.g. `month = %a %d.%m.%Y %H:%M`).

**Named constant:**

```ini
[Extras]
    [[Formatting]]
        [[[CardPageFormats]]]
            month = datetime_custom_card_full      # every card's min/max time on month.html
            year  = datetime_custom_card_full
```

**Direct (literal) format:**

```ini
[Extras]
    [[Formatting]]
        [[[CardPageFormats]]]
            month = %a %d.%m.%Y %H:%M
            year  = %a %d.%m.%Y %H:%M
```

- Valid scopes: `day` (covers index + yesterday), `week`, `month`, `year`, `telemetry`.
- A value is a named `datetime_custom_card_*` constant **or** a literal strftime string (see
  *Named constant vs. literal*).
- Cards are page-level only - there's no per-individual-card override.
- When no `CardPageFormats` entry applies, cards fall back to the built-in defaults
  (see *Customizing the Built-in Defaults* below).

---

## How the Layers Combine (Precedence)

For a chart's label or tooltip, the skin picks the first of these that's set:

1. **Per-chart** key on that custom chart (Example 1) - most specific, wins.
2. **Per-page keyed default** (`datetime_graph_label_day` / `_week` / `_month` / `_year`) - Example 2.
3. The **global default** (`datetime_graph_label` / `datetime_graph_tooltip`) - final fallback.

Cards are simpler: **per-page `CardPageFormats`** → the page's built-in default.

**Do nothing and nothing changes.** If you add no `CardPageFormats` and no per-chart keys, every
chart and card renders exactly as it does today.

---

## Customizing the Built-in Defaults

The "defaults" above are the built-in keys that already live in the `skin.conf` under
`[Extras] → [[Formatting]]`. The `CardPageFormats` / per-chart layers sit *on top* of them; if you'd
rather change the **baseline for the whole site** (one house style, no per-page fiddling), just edit
these keys directly.

| Key | Dialect | Ships as | What it affects |
|---|---|---|---|
| `datetime_graph_label` | moment | `dd HH:mm` | Chart **x-axis** base/fallback (telemetry; any page without a more specific key) |
| `datetime_graph_label_day` | moment | `dd HH:mm` | Chart **x-axis** labels on the day pages (index, yesterday) |
| `datetime_graph_label_week` | moment | `ddd` | Chart **x-axis** labels on the week page |
| `datetime_graph_label_month` | moment | `DD` | Chart **x-axis** labels on the month page |
| `datetime_graph_label_year` | moment | `DD.MM` | Chart **x-axis** labels on the year page |
| `datetime_graph_tooltip` | moment | `dd DD. MMM YY HH:mm` | Chart **hover tooltip** base/fallback (telemetry) |
| `datetime_graph_tooltip_day` | moment | `dd DD. MMM YY HH:mm` | Chart **hover tooltip** on the day pages (index, yesterday) |
| `datetime_graph_tooltip_week` | moment | `dd DD. MMM YY HH:mm` | Chart **hover tooltip** on the week page |
| `datetime_graph_tooltip_month` | moment | `dd DD. MMM YY HH:mm` | Chart **hover tooltip** on the month page |
| `datetime_graph_tooltip_year` | moment | `dd DD. MMM YY HH:mm` | Chart **hover tooltip** on the year page |
| `datetime_today` | strftime | `%H:%M` | **Card** min/max times on the day pages (index, yesterday) and telemetry |
| `datetime` | strftime | `%a %d %H:%M` | **Card** min/max times on week and month |
| `datetime_archive` | strftime | `%d.%m. %H:%M` | **Card** min/max times on year |

How they fit the cascade:

- The `datetime_graph_label[_week/_month/_year]` and `datetime_graph_tooltip[_week/_month/_year]`
  keys are **tier 2/3** for charts - the fallback when no per-chart key applies. Editing one shifts
  the baseline for every chart on that page that hasn't been overridden.
- The three card keys (`datetime_today` / `datetime` / `datetime_archive`) are the per-page **card
  default** - the fallback when no `CardPageFormats` entry applies.

> ⚠️ **`datetime` does double duty.** Besides the week/month card times above, `datetime` also formats
> the header's **"last updated" timestamp** (refreshed live when MQTT is enabled). Two ways to keep
> them apart: use `CardPageFormats` (Example 3) to restyle only the cards and leave `datetime` alone,
> or set the optional **`datetime_updated`** key to give the timestamp its own format (it falls back
> to `datetime` when unset, so existing setups are unaffected).
>
> Example: `datetime_updated = %a %d %H:%M`
>
> Note: `%-d` / `%-H` (GNU strftime zero-strip) do **not** work in MQTT live renders; use the padded
> tokens (`%d`, `%H`) in `datetime_updated`.

What these keys do **not** control:

- **The header's date and time rows** use their own separate `date` and `time` keys (not in the table
  above). Edit those to restyle the header without touching cards or charts.
- **Sunrise / sunset** (and moon / twilight) times aren't formatted here at all - they come from
  WeeWX's almanac and are controlled in `weewx.conf` under `[Units] [[TimeFormats]]`.

**Rule of thumb:** edit the defaults for a single global look; use the per-page / per-chart layers
(Examples 1-3) for exceptions. The defaults are also your reference for *what a page currently does* -
copy the relevant default into a `datetime_custom_*` name as a starting point and tweak from there.

---

## A Complete Worked Example

Goal: short dates on the month page's charts, nicer card times on month and year - and one specific
custom chart (Outdoor Temperature) that keeps a bare `HH:mm` tooltip because you stare at it all day
and don't need the date repeated.

This one example uses **both** styles - named constants *and* literals mixed together:

```ini
[Extras]
    [[Formatting]]
        # NAMED constants - defined once, reused below
        datetime_custom_graph_time = HH:mm                    # moment (charts)
        datetime_custom_card_full  = %a %d.%m.%Y %H:%M         # strftime (cards)

        # Per-page keyed defaults - written as LITERALS
        datetime_graph_label_month   = ddd DD                 # every chart's x-axis on month.html
        datetime_graph_tooltip_month = ddd DD.MM.YYYY HH:mm   # every chart's tooltip on month.html

        # Per-page cards - a NAMED constant
        [[[CardPageFormats]]]
            month = datetime_custom_card_full
            year  = datetime_custom_card_full

    [[Appearance]]
        [[[customChartOutTemp]]]
            title     = Outdoor Temperature
            charttype = area
            column    = avg
            values    = outTemp, dewpoint
            datetime_tooltip_format = datetime_custom_graph_time   # NAMED - bare HH:mm, wins over the page tooltip
            [[[[year]]]]
                datetime_label_format = MMM                        # LITERAL - axis = "Mar", "Apr", … on year.html
```

Result, on `month.html`:

- **Every chart** has axes reading `Mon 07` and tooltips reading `Mon 07.03.2026 14:05` (from the
  keyed defaults `datetime_graph_label_month` / `datetime_graph_tooltip_month`)…
- **…except the Outdoor Temperature chart**, whose tooltip reads just `14:05` - its per-chart
  `datetime_tooltip_format` wins, while its axis still follows the page (`Mon 07`) because it didn't
  override the label.
- **Card** min/max times read `Mon 07.03.2026 14:05`.

On `year.html`, only the Outdoor Temperature chart changes (no keyed defaults are set for year in
this example), thanks to its per-chart sub-section:

- Its axis reads `Mar`, `Apr`, … (the `[[[[year]]]]` label override); tooltip still `14:05`.
- Every other chart on the year page keeps the global defaults.
- Cards read `Mon 07.03.2026 14:05` (from the `year` CardPageFormats entry).

Pages and charts you didn't touch render exactly as before.

---

## Troubleshooting

- **The whole label is literal text like `datetime_custom_graph_full`** - you referenced a name that
  doesn't exist in `[[Formatting]]`, or there's a typo. Names are case-sensitive.
- **A card shows `dd DD HH:mm` literally** - you used a *graph* (moment) name in `CardPageFormats`.
  Cards need a `datetime_custom_card_*` (strftime) name.
- **Minutes show the month** - you wrote `MM` instead of `mm` in a moment format somewhere other than
  after a colon (the auto-fix only covers `:MM`).
- **Nothing changed** - did you regenerate the report and hard-refresh? Is the chart actually
  listed in a chart section's `items`? Try deleting the html file and letting the report generator recreate it.
- **The wrong page scope is matching** - per-page label/tooltip and card scopes are: `day` (index +
  yesterday share it), `week`, `month`, `year`, `telemetry`. There are no archive scopes; archive pages
  (month-YYYY-MM, year-YYYY) use the `month` / `year` keys respectively. (Per-**chart** override
  sub-sections are separate and use `current`/`yesterday`/`week`/`month`/`year`.)
