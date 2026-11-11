# Multi-Axis CustomCharts Quick Reference

## Basic Syntax

```conf
[[[customChartName]]]
    title = Chart Title
    charttype = area          # or bar, line
    column = avg              # or min, max, sum
    values = field1, field2
    
    [[[[yaxis]]]]
        field1 = y1
        field2 = y2
    
    [[[[yaxis_config]]]]
        [[[[[y1]]]]]
            title = Axis 1 Label
            opposite = false
        [[[[[y2]]]]]
            title = Axis 2 Label
            opposite = true
```

## Configuration Options

### [[[[yaxis]]]] - Axis Assignment
Maps each field to an axis ID (y1, y2, y3, etc.)

```conf
[[[[yaxis]]]]
    fieldName = y1
    otherField = y2
```

### [[[[yaxis_config]]]] - Axis Settings
Configure appearance and behavior of each axis.

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `title` | string | Axis label | (empty) |
| `opposite` | boolean | Position on right? | false for y1, true for others |
| `decimals` | integer | Decimal places | Auto |
| `min` | number | Minimum value | Auto |
| `max` | number | Maximum value | Auto |

## Quick Examples

### Two Axes
```conf
[[[customChart1]]]
    title = UV & Radiation
    values = UV, radiation
    [[[[yaxis]]]]
        UV = y1
        radiation = y2
```

### Three Axes
```conf
[[[customChart2]]]
    title = Temp, Humidity & Pressure
    values = outTemp, outHumidity, barometer
    [[[[yaxis]]]]
        outTemp = y1
        outHumidity = y2
        barometer = y3
```

### Shared Axis
```conf
[[[customChart3]]]
    title = Temperatures + Pressure
    values = outTemp, inTemp, dewpoint, barometer
    [[[[yaxis]]]]
        outTemp = y1
        inTemp = y1
        dewpoint = y1
        barometer = y2
```

## Common Patterns

### Pattern 1: Different Units
```conf
values = temperature, humidity
[[[[yaxis]]]]
    temperature = y1
    humidity = y2
[[[[yaxis_config]]]]
    [[[[[y1]]]]]
        title = Temperature (°C)
    [[[[[y2]]]]]
        title = Humidity (%)
        opposite = true
```

### Pattern 2: Different Scales
```conf
values = UV, radiation
[[[[yaxis]]]]
    UV = y1
    radiation = y2
[[[[yaxis_config]]]]
    [[[[[y1]]]]]
        title = UV Index
        min = 0
        max = 15
    [[[[[y2]]]]]
        title = Radiation (W/m²)
        opposite = true
```

### Pattern 3: Multiple on Same Axis
```conf
values = outTemp, inTemp, dewpoint
# No yaxis config = all on y1 by default
```

## Cheat Sheet

### Axis IDs
- Must be: y1, y2, y3, y4, etc.
- Sequential starting from y1
- Up to 10 supported (y1-y10)
- Recommended: 2-3 maximum

### Positioning
- `opposite = false` → Left side
- `opposite = true` → Right side
- Default: y1 left, others right

### Chart Types
- `area` - Filled area chart
- `bar` - Vertical bars
- `line` - Line chart only

### Columns
- `avg` - Average value
- `min` - Minimum value
- `max` - Maximum value
- `sum` - Total (for rain, ET, etc.)
- `vecdir` - Vector direction (wind)

## Indentation Guide

```conf
[[Appearance]]                     ← Level 2
    [[[customChartName]]]          ← Level 3
        title = ...                ← Level 3 property
        [[[[yaxis]]]]              ← Level 4
            field1 = y1            ← Level 4 property
        [[[[yaxis_config]]]]       ← Level 4
            [[[[[y1]]]]]           ← Level 5
                title = ...        ← Level 5 property
```

**Important:** Use 4 spaces per indentation level!

## Common Fields

### Temperature
- `outTemp` - Outside temperature
- `inTemp` - Inside temperature
- `dewpoint` - Dew point
- `windchill` - Wind chill
- `heatindex` - Heat index
- `appTemp` - Apparent temperature

### Humidity
- `outHumidity` - Outside humidity
- `inHumidity` - Inside humidity

### Pressure
- `barometer` - Barometric pressure
- `altimeter` - Altimeter pressure
- `pressure` - Station pressure

### Solar
- `UV` - UV index
- `radiation` - Solar radiation

### Wind
- `windSpeed` - Wind speed
- `windGust` - Wind gust
- `windDir` - Wind direction
- `wind` - Wind (for vecdir)

### Rain
- `rain` - Rainfall
- `rainRate` - Rain rate
- `ET` - Evapotranspiration

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Chart not showing | Check field names, verify data exists |
| Wrong units | Check first field on axis |
| Axes overlap | Set `opposite = true` for y2+ |
| Syntax error | Check indentation (4 spaces) |
| Template error | Verify bracket nesting |

## Testing Steps

1. Add chart config to `[[Appearance]]` section
2. Add chart name to a chart section's `items`
3. Restart WeeWX: `sudo systemctl restart weewx`
4. Navigate to Yesterday page
5. Check chart appears with multiple axes

## Resources

- **Full Guide:** MULTI-AXIS-USAGE-GUIDE.md
- **Test Configs:** MULTI-AXIS-TEST-CONFIG.conf
- **Summary:** MULTI-AXIS-IMPLEMENTATION-SUMMARY.md

---

**Quick Start:**
1. Copy example above
2. Replace field names with your sensors
3. Add to skin.conf [[Appearance]]
4. Add to a chart section's `items`
5. Restart WeeWX

