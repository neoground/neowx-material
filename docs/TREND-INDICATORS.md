# Trend Indicators – Usage Guide

## Overview

On the **current page**, selected observations can show a small **trend arrow** next
to their value, with a tooltip describing how much the reading has changed. Barometer
trends in particular are a classic forecasting aid — a falling barometer often
precedes unsettled weather.

neowx-material's trend indicators are **unit-aware**: the direction is always decided
in a single canonical unit per measurement family, so a station running imperial units
behaves exactly like one running metric (or any other supported unit). You can also
choose how sensitive the thresholds are with the `trend_type` setting.

This feature is useful when you want to:

- See at a glance whether temperature, humidity, pressure, etc. are rising or falling
- Get a graded barometric **tendency** (slowly / rising / rapidly) like a dedicated console
- Match the behavior of a specific weather station brand (e.g. Davis Vantage)

## Quick Start

Two settings under `[[Appearance]]` in `skin.conf` control everything:

```conf
[[Appearance]]
    # Which observations get a trend arrow on the current page
    show_trend_on = barometer, outTemp, outHumidity, inTemp, inHumidity, UV, co2, pm2_5, ET

    # How the thresholds behave: multi (default) | davis | strict
    trend_type = multi
```

**Result:** each observation listed in `show_trend_on` shows a ↗ / → / ↘ arrow (and,
for the barometer in `multi`/`davis`, a graded tendency) with a tooltip such as
`Rising Slowly: +0.009 inHg (over 3 h)`.

## How a trend is calculated

For each observation the skin compares:

- the **current** reading, against
- the **oldest reading in the last 3 hours** (`span(hour_delta=3).<obs>.first`)

The 3-hour window is the meteorological standard for pressure tendency and works well
for the other observations too.

> **Why not WeeWX `$trend`?** `$trend` only returns a value when an archive record
> lands within a small grace window of *exactly* `now − 3h`; on many stations that
> made the arrows disappear intermittently. The skin computes the change itself, so an
> arrow appears whenever there is enough recent data.

The change is evaluated in a **canonical unit** (see
[Unit independence](#unit-independence)) to decide the direction and tier, while the
**tooltip** always shows the change in *your* display units.

**Short windows (restarts, new installs, data gaps).** The skin uses the *actual*
span between the first available reading and now, not a fixed 3 hours:

- If less than **1 hour** of recent history exists, the trend is **hidden** — there
  isn't enough data for a meaningful tendency.
- Between 1 and 3 hours, the pressure thresholds are **scaled to the real span** (so a
  rate that would be "rising" over 3 h is still classified correctly over a shorter
  window), and the tooltip shows the true interval, e.g. `(over 1.5 h)`. A span within
  **5 minutes** of the full 3 hours is treated as a complete window and shown as `3 h`.

## Threshold styles (`trend_type`)

All three styles share the same unit-canonical engine; they differ only in the barometer
thresholds and how many tiers are shown. Non-pressure observations use a simple
up / steady / down arrow in every style.

In the tables below, **Δ** is the pressure change over the window. The ranges are
half-open so every value maps to exactly one tier (a value exactly on a boundary falls
into the *faster* tier — e.g. Δ = +2.0 hPa is "Rising", not "Rising Slowly").

### `multi` (default) — WMO/NWS-style multi-tier

The barometer arrow steepens as the change grows, and doubles for the rapid tier.

| Change (Δ) | inHg equiv. | Arrow | Tooltip word |
|---|---|---|---|
| Δ ≥ +5.0 hPa | ≥ +0.148 | ↑↑ | Rising Rapidly |
| +2.0 ≤ Δ < +5.0 hPa | +0.059 … +0.148 | ↑ | Rising |
| +0.7 ≤ Δ < +2.0 hPa | +0.021 … +0.059 | ↗ | Rising Slowly |
| −0.7 < Δ < +0.7 hPa | within ±0.021 | → | Steady |
| −2.0 < Δ ≤ −0.7 hPa | −0.021 … −0.059 | ↘ | Falling Slowly |
| −5.0 < Δ ≤ −2.0 hPa | −0.059 … −0.148 | ↓ | Falling |
| Δ ≤ −5.0 hPa | ≤ −0.148 | ↓↓ | Falling Rapidly |

### `davis` — Davis Vantage thresholds

Mirrors the 3-level rate descriptions used by Davis Vantage consoles.

| Change (Δ) | inHg equiv. | Arrow | Tooltip word |
|---|---|---|---|
| Δ ≥ +2.0 hPa | ≥ +0.059 | ↑ | Rising Rapidly |
| +0.7 ≤ Δ < +2.0 hPa | +0.021 … +0.059 | ↗ | Rising Slowly |
| −0.7 < Δ < +0.7 hPa | within ±0.021 | → | Steady |
| −2.0 < Δ ≤ −0.7 hPa | −0.021 … −0.059 | ↘ | Falling Slowly |
| Δ ≤ −2.0 hPa | ≤ −0.059 | ↓ | Falling Rapidly |

These hPa cut-points reproduce Davis's published inHg table (≈ 0.02 inHg and 0.06 inHg).

### `strict` — simple up / steady / down

Every observation, including the barometer, uses a single steady band and a simple
rising / steady / falling arrow — no rapid tier, no doubled arrows.

| Change (Δ, barometer) | Arrow | Tooltip word |
|---|---|---|
| Δ > +0.5 hPa | ↗ | Rising |
| −0.5 ≤ Δ ≤ +0.5 hPa | → | Steady |
| Δ < −0.5 hPa | ↘ | Falling |

The `0.5 hPa` band is the canonical pressure dead-band; the same single-band logic
applies to all other observations (see the table below).

> **Note:** an unrecognized `trend_type` value falls back to `multi`.

## Unit independence

The direction (and, for the barometer, the tier) is decided after converting the change
into a **canonical unit** for that measurement family. This is what makes a trend behave
identically regardless of the units your station reports in.

| Family | Canonical unit | Steady dead-band (±) | Display units that map to it |
|---|---|---|---|
| Pressure | mbar (hPa) | 0.5 | inHg, mmHg, kPa, hPa, mbar |
| Temperature | °C | 0.5 | °F, °K, °C |
| Rain / ET | mm | 0.2 | inch, cm, mm |
| Rain rate | mm/h | 0.2 | inch/h, cm/h, mm/h |
| Wind speed | km/h | 1.0 | mph, knot, m/s, beaufort, km/h |
| Distance (short) | meter | 10 | foot, meter |
| Distance (long) | km | 0.5 | mile, km |
| Unit-agnostic | — (compared directly) | 0.5 | %, UV index, ppm, µg/m³, W/m² |

The dead-band is the "steady" threshold for `strict` and for all non-pressure
observations in `multi`/`davis`. Because it is applied in the canonical unit, a small
imperial change and the equivalent metric change produce the **same** result.

**Example:** a **+0.8 °F** rise equals **+0.44 °C**, which is below the 0.5 °C dead-band,
so it correctly shows **Steady** — exactly as a +0.44 °C reading would. A naïve raw
threshold would treat °F and °C differently.

## Tooltips

The tooltip names the category and shows the signed change in your display units over the
window actually covered by the data, e.g. `Rising Rapidly: +0.118 inHg (over 3 h)` (or
`(over 1.5 h)` shortly after a restart — see [How a trend is calculated](#how-a-trend-is-calculated)).

> **Localization.** The category words (`Rising`, `Falling`, `Steady`, `Rising Slowly`,
> `Rising Rapidly`, `Falling Slowly`, `Falling Rapidly`) and the word `over` are pulled
> from the active language file via `[Texts] → [[Trend]]` in `skins/neowx-material/lang/*.conf`.
> The `davis` style reuses the same phrase keys, so its tooltips read identically to `multi`.

Pressure is rounded to a unit-appropriate precision so small changes remain visible:

| Pressure unit | Decimal places |
|---|---|
| inHg | 3 |
| kPa | 2 |
| mmHg | 2 |
| hPa / mbar | 1 |

Non-pressure observations are rounded to 2 decimal places.

## Arrow reference

Arrows are drawn with the bundled [Weather Icons](https://erikflowers.github.io/weather-icons/) font:

| Glyph | Class | Meaning |
|---|---|---|
| → | `wi-direction-right` | Steady |
| ↗ / ↘ | `wi-direction-up-right` / `wi-direction-down-right` | Rising / falling (or "slowly") |
| ↑ / ↓ | `wi-direction-up` / `wi-direction-down` | Rising / falling faster |
| ↑↑ / ↓↓ | two `wi-direction-up` / `wi-direction-down` | `multi` rapid tier |

## Choosing a style

- **`multi`** — the richest readout; good if you like a graded barometric tendency and
  want the most information at a glance. *(Default.)*
- **`davis`** — pick this if you're used to a Davis Vantage console and want the arrows
  to match its rise/fall rate descriptions.
- **`strict`** — the simplest, least cluttered option: just up / steady / down for
  everything, with no rapid tier.

## Configuration reference

| Setting | Section | Values | Default |
|---|---|---|---|
| `show_trend_on` | `[[Appearance]]` | comma-separated observation names | `barometer, outTemp, outHumidity, inTemp, inHumidity, UV, co2, pm2_5, ET` |
| `trend_type` | `[[Appearance]]` | `multi`, `davis`, `strict` | `multi` |

## Troubleshooting

**No arrow appears for an observation**
- Confirm the observation name is listed in `show_trend_on` (names are case-sensitive and
  must match the WeeWX field, e.g. `outTemp`, not `Outside Temperature`).
- The observation needs archive records within the last 3 hours.

**No arrow right after a restart, new install, or data gap**
- The trend is hidden until at least **1 hour** of recent history is available, so the
  tendency isn't computed from just a few minutes of data. It returns automatically once
  the window is wide enough.

**The barometer almost never leaves "steady" (or flickers constantly)**
- That's the threshold style. The steady band is ±0.5 hPa in `strict` and ±0.7 hPa in
  `multi`/`davis`; beyond it, `multi` adds graded rising/rapid tiers while `strict` shows
  a single rising/falling arrow. Switch `trend_type` to taste.

**The tooltip value looks rounded/coarse for pressure**
- inHg/kPa changes are tiny, so they use extra decimal places (see
  [Tooltips](#tooltips)). If you display in hPa/mbar the change is larger and shown to 1
  decimal place.
