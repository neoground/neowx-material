# Battery Field Mapping Configuration Guide

## Overview
This feature allows you to convert numeric battery status values (like 0, 1, 9) into human-readable custom text (like "OK", "Low", "Critical") and display them with custom charts.

---

## Step-by-Step Configuration

### Step 1: Find Your Station's Battery Values

First, you need to know what numeric values your weather station reports for battery status. 
If you know your station has a "battery sensor" but it's not showing under the Telemetry page edit the skin.conf to allow_zero_values = yes to plot it.
Or check your weewx database or reports to see what values appear.

**Common examples:**
- Simple stations: `0` and `1` 
- Ecowitt stations: `0` (Normal) and `9` (Low)
- Advanced stations: `0`, `1`, `2`, `3`, `4` (multiple levels)

---

### Step 2: Understanding the Configuration Structure

For each battery sensor, you need to configure 4 things:

```ini
[[Telemetry]]
allow_zero_values = yes #needed if the sensor output might be equal to 0
chart_days = 1
    [[[BatteryFields]]]
        [[[[sensorName]]]]
            enabled = yes
            
            # 1. TEXT LABELS: What text to display for each raw value
            0 = Normal
            9 = Low
            
            # 2. CHART POSITIONS: Where each value appears on the chart
            chart_position_0 = 1
            chart_position_9 = 0
            
            # 3. MAX POSITION: The highest position number used
            max_chart_position = 1
            
            # 4. FLIP: Whether to flip the chart upside-down
            flip_values = no
```

---

### Step 3: Configure Text Labels

**Rule:** The number must match what your station reports.

```ini
# Your station reports these raw values → Display this text
0 = Normal
9 = Low
```

**Examples:**

**Two-level battery (0 and 1):**
```ini
0 = OK
1 = Low
```

**Three-level battery (0, 1, and 2):**
```ini
0 = Good
1 = Fair  
2 = Low
```

**Five-level battery (0 through 4):**
```ini
0 = Full
1 = Good
2 = Fair
3 = Low
4 = Critical
```

---

### Step 4: Configure Chart Positions

**Rule:** `chart_position_X` where X is the RAW VALUE your station reports.

The position number controls where it appears on the chart:
- **Higher numbers = Top of chart** (good status)
- **Lower numbers = Bottom of chart** (bad status)

#### Example 1: Ecowitt (values 0 and 9)

```ini
# Station reports 0 for Normal → Show at top (position 1)
chart_position_0 = 1

# Station reports 9 for Low → Show at bottom (position 0)
chart_position_9 = 0

# Chart range is 0 to 1
max_chart_position = 1
```

**Visual result:**
```
Chart Y-axis:
1 (top)    ← Normal (green/good)
0 (bottom) ← Low (red/warning)
```

#### Example 2: Simple Battery (values 0 and 1)

```ini
# Station reports 0 for OK → Show at top (position 1)
chart_position_0 = 1

# Station reports 1 for Low → Show at bottom (position 0)
chart_position_1 = 0

# Chart range is 0 to 1
max_chart_position = 1
```

#### Example 3: Three-Level Battery (values 0, 1, and 2)

```ini
# Station reports 0 for Good → Show at top (position 2)
chart_position_0 = 2

# Station reports 1 for Fair → Show in middle (position 1)
chart_position_1 = 1

# Station reports 2 for Low → Show at bottom (position 0)
chart_position_2 = 0

# Chart range is 0 to 2
max_chart_position = 2
```

**Visual result:**
```
Chart Y-axis:
2 (top)    ← Good
1 (middle) ← Fair
0 (bottom) ← Low
```

---

### Step 5: Set Maximum Chart Position

**Rule:** Set this to the highest position number you used.

```ini
# If your positions are 0 and 1
max_chart_position = 1

# If your positions are 0, 1, and 2
max_chart_position = 2

# If your positions are 0, 1, 2, 3, and 4
max_chart_position = 4
```

---

### Step 6: Flip Values (Optional)

If your chart looks upside-down (bad status at top, good at bottom), use flip:

```ini
# Normal display (good at top)
flip_values = no

# Flipped display (inverts the chart)
flip_values = yes
```

---

## Common Mistakes

### ❌ WRONG: Using sequential numbers instead of raw values
```ini
0 = Normal
9 = Low
chart_position_0 = 1
chart_position_1 = 0  # WRONG! Station doesn't report value "1"
```

### ✅ CORRECT: Using the actual raw values
```ini
0 = Normal
9 = Low
chart_position_0 = 1
chart_position_9 = 0  # CORRECT! Matches the raw value "9"
```

---

## Complete Examples

### Example 1: Ecowitt Battery (0=Normal, 9=Low)

```ini
[[Telemetry]]
    allow_zero_values = yes
    chart_days = 1
    
    [[[BatteryFields]]]
        [[[[batteryStatus1]]]]
            enabled = yes
            0 = Normal
            9 = Low
            chart_position_0 = 1
            chart_position_9 = 0
            max_chart_position = 1
            flip_values = no
```

### Example 2: Simple Battery (0=OK, 1=Change)

```ini
[[Telemetry]]
    allow_zero_values = yes
    chart_days = 1
    
    [[[BatteryFields]]]
        [[[[outTempBatteryStatus]]]]
            enabled = yes
            0 = OK
            1 = Change
            chart_position_0 = 1
            chart_position_1 = 0
            max_chart_position = 1
            flip_values = no
```

### Example 3: Multi-Level Battery (0-4)

```ini
[[Telemetry]]
    allow_zero_values = yes
    chart_days = 7
    
    [[[BatteryFields]]]
        [[[[extraBatteryStatus1]]]]
            enabled = yes
            0 = Full
            1 = Good
            2 = Fair
            3 = Low
            4 = Critical
            chart_position_0 = 4
            chart_position_1 = 3
            chart_position_2 = 2
            chart_position_3 = 1
            chart_position_4 = 0
            max_chart_position = 4
            flip_values = no
```

---

## Quick Reference

| Setting | Purpose | Example |
|---------|---------|---------|
| `enabled` | Turn mapping on/off | `yes` or `no` |
| `0 = Text` | Label for raw value 0 | `0 = Normal` |
| `chart_position_X` | Y-axis position for value X | `chart_position_0 = 1` |
| `max_chart_position` | Highest position used | `1` for 2 levels, `2` for 3 levels, etc. |
| `flip_values` | Invert chart display | `yes` or `no` |

---

## Troubleshooting
**Problem:** The sensor is not even showing under telemetry
- **Check:** Make sure `allow_zero_values = yes` so the sensor gets plotted even if is reporting 0

**Problem:** Chart shows numbers instead of text labels
- **Check:** Make sure `chart_position_X` uses the actual raw values your station reports

**Problem:** Chart looks upside-down (bad at top, good at bottom)
- **Solution:** Set `flip_values = yes`

**Problem:** Values show as "1", "2" instead of your labels
- **Check:** Your `chart_position_X` numbers must match the raw values from your station

**Problem:** Page breaks when sensor doesn't exist
- **Solution:** Only configure sensors that actually exist, or set `enabled = no`

---

## Testing Your Configuration

1. Add the configuration to `skin.conf`
2. Restart weewx: `sudo systemctl restart weewx`
3. Check the telemetry page
4. Look at the chart - "good" status should be at the top
5. If upside-down, set `flip_values = yes`
