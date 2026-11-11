# Telemetry Gauge Update: Voltage, Signal & Status Batteries

> **Note — config format changed:** The examples in this document use the current
> `[[Telemetry]] [[[<field>]]]` + `sensor_type` format. If your existing config
> uses a different layout (sub-sections, `enabled` keys), see the migration guide
> in **`docs/TELEMETRY-CONFIG-GUIDE.md`** for how to convert it, as well as the
> full option reference.

## What Changed?

The telemetry page can now show a meaningful, color-coded gauge for **every** kind
of battery or signal sensor — not just voltage sensors.

### Before:
- Voltage sensors: percentage gauge (green/red)
- Status sensors (0=OK, 1=Low): gauge always showed 100% green
- Signal sensors (rxCheckPercent): gauge always showed 100% green

### After:
- **Voltage sensors:** unchanged — percentage gauge from your min/max voltage
- **Status sensors:** the bar now fills based on which state the battery is in,
  can turn red on a "low" state, and shows the raw station value under the bar
- **Signal sensors:** a brand new WiFi-style "cone" indicator that fills from the
  center outward with the signal percentage

All three turn **red** when the sensor crosses the "low" point you configure.

---

## The Three Modes at a Glance

| Your sensor reports... | You set... | You get... |
|---|---|---|
| A voltage (4.5V, 12.6V, ...) | `sensor_type = voltage` + `max_voltage` | Battery bar filled by percentage, value at `value_position` |
| Signal strength (0–100%) | `sensor_type = signal` | WiFi-style cone filled by percentage, value at `value_position` |
| Status codes (0, 1, 9, ...) | `sensor_type = status` + text labels + chart positions | Battery bar filled by state, raw value at `value_position` |

---

## Mode 1: Voltage Battery Gauge

For sensors that report an actual voltage. You tell the skin what voltage means
"full" and what means "empty", and it works out the percentage.

```ini
[[[consBatteryVoltage]]]
    sensor_type = voltage
    max_voltage = 4.5      # this voltage = 100%
    min_voltage = 0.0      # this voltage = 0%
    low_threshold = 50     # below 50%, the bar turns red
```

**Results:**
- 4.5V → 100% green gauge
- 3.6V → 80% green gauge
- 2.0V → 44% red gauge (below 50%)

A 12V system works the same way — just use its real range:

```ini
[[[supplyVoltage]]]
    sensor_type = voltage
    max_voltage = 14.4    # fully charged
    min_voltage = 10.5    # discharged
    low_threshold = 30
```

If `low_threshold` is left out it defaults to 20 (red below 20%).

---

## Mode 2: Signal Indicator (new)

For sensors that report signal strength, like `rxCheckPercent` (the percentage of
radio packets your console actually received). Instead of a battery bar, the card
draws a WiFi-style cone — two curved wings around a center dot — that fills from
the center outward as the signal gets stronger.

```ini
[[[rxCheckPercent]]]
    sensor_type = signal
    max_signal = 100      # raw value that means 100%
    min_signal = 0        # raw value that means 0% (optional, defaults to 0)
    low_threshold = 30    # below 30%, the cone turns red
```

**Results:**
- 100% → full green cone
- 70% → green cone filled about 70% of the way out
- 20% → mostly empty red cone (below the 30% threshold)

`rxCheckPercent` already reports 0–100, so `max_signal = 100` maps it one-to-one.
If your sensor uses another scale (say 0–255 RSSI), set `max_signal` to that
full-scale value instead and the skin converts it to a percentage for you.

If you omit `max_signal` entirely, the raw value is used directly as the
percentage (clamped to 0–100).

---

## Mode 3: Status Batteries (improved)

For sensors that report discrete codes — `0 = OK, 1 = Low`, Ecowitt's
`0 = Normal, 9 = Low`, or multi-level `0..4` scales. Two things are new here:

**1. The bar now fills based on the state.** No configuration needed beyond the
chart positions you already have. The best state fills the bar completely; each
step down fills it less. With two states OK = 100% and Low = 50%; with five
states you get 100 / 80 / 60 / 40 / 20%. The raw station value (`0`, `1`, `9`...)
is shown under the bar — for a status sensor that number is usually more useful
than a percentage. If the station value has no matching label, the raw value is
shown as-is.

**2. The bar can turn red when the battery is low.** Tell the skin which raw
value means "low" and in which direction to compare:

```ini
[[[outTempBatteryStatus]]]
    sensor_type = status
    0 = OK
    1 = Low
    chart_position_0 = 1
    chart_position_1 = 0
    max_chart_position = 1
    low_state = 1             # the station reports 1 when the battery is low
    low_when = at_or_above    # red when the value is 1 or higher (default)
```

**Results:**
- Station reports 0 → card says "OK", green bar, "0" below
- Station reports 1 → card says "Low", **the entire bar turns red**, "1" below

Why a direction setting? Stations disagree about which way is "low":

- Most stations: a **higher** number means a weaker battery (`1 = Low`, Ecowitt
  `9 = Low`) → use `low_when = at_or_above` (the default, so you can leave it out)
- A few stations: a **lower** number means a weaker battery (`1 = OK, 0 = Low`)
  → use `low_when = at_or_below` with `low_state = 0`

For two-state sensors a low battery fills the **whole** bar red, so you can spot
it from across the room. Sensors with three or more states keep their partial
fill and just change color — e.g. a 5-state sensor at "Low" shows a 40% red bar.

If you don't set `low_state`, the bar fills by state but never turns red — same
as not setting `low_threshold` on a voltage sensor.

---

## What the Cards Look Like

```
┌───────────────────────────┐   ┌───────────────────────────┐   ┌───────────────────────────┐
│      Console Battery      │   │      Signal Quality       │   │    Transmitter Battery    │
│                           │   │                           │   │                           │
│  2.2V     12.6V    13.1V  │   │  98%      100%      100%  │   │  OK        OK         OK  │
│                           │   │                           │   │                           │
│   [█████████████░░░]      │   │      ((( ( · ) )))        │   │   [█████████████████]     │
│         91%               │   │         100%              │   │          0                │
└───────────────────────────┘   └───────────────────────────┘   └───────────────────────────┘
       voltage mode                   signal mode                     status mode
   (percentage below bar)         (cone, % below)              (raw station value below bar)
```

| Condition | Color |
|-----------|-------|
| OK / above threshold | Green (`#4caf50`) |
| Low / below threshold | Red (`#f44336`) |

---

## Complete Example Configuration

All three modes side by side:

```ini
[Extras]
    [[Telemetry]]
        allow_zero_values = yes   # needed when a healthy sensor reports 0
        chart_days = 30

        # Signal strength -> WiFi-style cone
        [[[rxCheckPercent]]]
            sensor_type = signal
            max_signal = 100
            min_signal = 0
            low_threshold = 30

        # Voltage sensor -> percentage battery bar
        [[[consBatteryVoltage]]]
            sensor_type = voltage
            max_voltage = 13.8
            min_voltage = 0.0
            low_threshold = 25

        # Status sensor -> state-based bar, red when the station says low
        [[[outTempBatteryStatus]]]
            sensor_type = status
            0 = OK
            1 = Low
            chart_position_0 = 1
            chart_position_1 = 0
            max_chart_position = 1
            low_state = 1
            low_when = at_or_above
```

---

## Testing

1. Add the configuration to your `skin.conf`
2. Restart weewx (`sudo systemctl restart weewx`) or regenerate now with
   `sudo weectl report run`
3. Open the telemetry page and check each card shows the right gauge and color

---

## Troubleshooting

**Voltage gauge still shows 100%?**
- Make sure `max_voltage` matches your battery's real maximum
- Check the sensor reports a voltage, not a status code
- Restart WeeWX after configuration changes
- If neither `max_voltage` nor `min_voltage` is set, the card now shows the raw
  value with no gauge (instead of a full bar), so a missing range is obvious

**Status bar stays green even though the station says Low?**
- Add `low_state` (and `low_when` if your station counts the other way)
- Without `low_state` the bar fills by state but never turns red
- If a reported value has no matching `chart_position_<value>`, the card shows the
  raw value with no gauge (instead of a green bar) so the gap is visible

**Signal cone never shows?**
- The cone only appears when `sensor_type = signal` is set for that sensor
- Don't set `sensor_type = signal` and `sensor_type = voltage` on the same sensor

**Card missing entirely?**
- Set `allow_zero_values = yes` — many healthy sensors report 0 all day, and
  without this the card is hidden
- The sensor must also be listed in a telemetry section's `items` under `[[Appearance]]`

**Values shown with commas (e.g., "4,5 V")?**
- Handled automatically — units are stripped and both comma and period decimal
  separators work

---

## See Also

- `TELEMETRY-CONFIG-GUIDE.md` — the complete configuration guide, including chart
  positions, flip, multi-level states, and display ordering
- `skin.conf` — your main configuration file
