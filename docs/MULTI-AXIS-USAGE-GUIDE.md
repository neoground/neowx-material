# Multi-Axis CustomCharts - Usage Guide

## Overview

The neowx-material skin now supports multiple Y-axes in custom charts, allowing you to display data series with different value ranges or units on separate, independently scaled axes.

This feature is particularly useful when:
- Combining measurements with vastly different scales (e.g., UV index 0-15 with solar radiation 0-1000 W/m²)
- Displaying different units on the same chart (e.g., temperature in °C and humidity in %)
- Creating more readable charts by avoiding scale compression

## Quick Start

### Basic Two-Axis Example

Here's a simple example showing UV index and solar radiation on separate axes:

```conf
[[[customChartRadiation]]]
    title = Radiation and UV
    charttype = area
    column = avg
    values = UV, radiation
    
    # Assign each field to an axis
    [[[[yaxis]]]]
        UV = y1
        radiation = y2
    
    # Configure each axis
    [[[[yaxis_config]]]]
        [[[[[y1]]]]]
            title = UV Index
            opposite = false
        [[[[[y2]]]]]
            title = Solar Radiation (W/m²)
            opposite = true
```

**Result:** UV Index appears on the left axis (y1), Solar Radiation on the right axis (y2).

## Configuration Structure

### Required Sections

1. **Basic Chart Configuration** (as before)
   ```conf
   [[[customChartName]]]
       title = My Chart Title
       charttype = area    # or bar, line
       column = avg        # or min, max, sum
       values = field1, field2, field3
   ```

2. **Axis Assignment** (new)
   ```conf
   [[[[yaxis]]]]
       field1 = y1
       field2 = y2
       field3 = y1  # multiple fields can share an axis
   ```

3. **Axis Configuration** (optional)
   ```conf
   [[[[yaxis_config]]]]
       [[[[[y1]]]]]
           title = Axis 1 Label
           opposite = false
           # ... more options
       [[[[[y2]]]]]
           title = Axis 2 Label
           opposite = true
   ```

### Axis Assignment (`[[[[yaxis]]]]`)

Maps each data field to a Y-axis identifier (y1, y2, y3, etc.).

**Syntax:**
```conf
[[[[yaxis]]]]
    fieldName = axisId
```

**Example:**
```conf
[[[[yaxis]]]]
    outTemp = y1
    outHumidity = y2
    dewpoint = y1      # shares y1 with outTemp
    inTemp = y1        # also shares y1
```

**Notes:**
- Axis IDs must be y1, y2, y3, etc. (in order)
- Multiple fields can share the same axis
- If a field is not assigned, it defaults to y1
- You can use up to 10 axes (y1 through y10), though 2-3 is typical

### Axis Configuration (`[[[[yaxis_config]]]]`)

Customizes the appearance and behavior of each axis.

**Available Options:**

| Option | Type | Description | Default |
|--------|------|-------------|---------|
| `title` | string | Axis label text | Empty |
| `opposite` | boolean | Position on right side? | `false` for y1, `true` for others |
| `decimals` | integer | Number of decimal places | Auto (from unit format) |
| `min` | number | Minimum value | Auto-calculated |
| `max` | number | Maximum value | Auto-calculated |

**Example:**
```conf
[[[[yaxis_config]]]]
    [[[[[y1]]]]]
        title = Temperature (°C)
        opposite = false
        decimals = 1
        min = -10
        max = 40
    [[[[[y2]]]]]
        title = Humidity (%)
        opposite = true
        decimals = 0
        min = 0
        max = 100
```

## Complete Examples

### Example 1: Temperature and Humidity

Perfect for comparing temperature and humidity trends:

```conf
[[[customChartTempHumid]]]
    title = Temperature & Humidity
    charttype = area
    values = outTemp, outHumidity
    
    [[[[yaxis]]]]
        outTemp = y1
        outHumidity = y2
    
    [[[[yaxis_config]]]]
        [[[[[y1]]]]]
            title = Temperature (°C)
            opposite = false
            decimals = 1
        [[[[[y2]]]]]
            title = Humidity (%)
            opposite = true
            decimals = 0
```

### Example 2: UV and Solar Radiation

UV index (0-15) and solar radiation (0-1000+ W/m²) on separate scales:

```conf
[[[customChartRadiation]]]
    title = UV & Solar Radiation
    charttype = area
    column = avg
    values = UV, radiation
    
    [[[[yaxis]]]]
        UV = y1
        radiation = y2
    
    [[[[yaxis_config]]]]
        [[[[[y1]]]]]
            title = UV Index
            opposite = false
            decimals = 1
            min = 0
            max = 15
        [[[[[y2]]]]]
            title = Solar Radiation (W/m²)
            opposite = true
            decimals = 0
```

### Example 3: Three Axes - Temperature, Humidity, and Pressure

Advanced example with three independent axes:

```conf
[[[customChartMulti]]]
    title = Temperature, Humidity & Pressure
    charttype = line
    values = outTemp, outHumidity, barometer
    
    [[[[yaxis]]]]
        outTemp = y1
        outHumidity = y2
        barometer = y3
    
    [[[[yaxis_config]]]]
        [[[[[y1]]]]]
            title = Temperature (°C)
            opposite = false
        [[[[[y2]]]]]
            title = Humidity (%)
            opposite = true
        [[[[[y3]]]]]
            title = Pressure (mbar)
            opposite = true
```

**Note:** When using 3+ axes, the chart may become crowded. Consider limiting to 2-3 axes for best readability.

### Example 4: Multiple Fields on Same Axis

Multiple series sharing one axis, one series on another:

```conf
[[[customChartTemperatures]]]
    title = All Temperatures
    charttype = area
    values = outTemp, inTemp, dewpoint, barometer
    
    [[[[yaxis]]]]
        outTemp = y1
        inTemp = y1
        dewpoint = y1
        barometer = y2    # pressure on separate axis
    
    [[[[yaxis_config]]]]
        [[[[[y1]]]]]
            title = Temperature (°C)
            opposite = false
        [[[[[y2]]]]]
            title = Pressure (mbar)
            opposite = true
```

### Example 5: Per-Page Configuration with Multi-Axis

Combine multi-axis with per-page column overrides:

```conf
[[[customChartOutTemp]]]
    title = Outdoor Temperature
    charttype = area
    values = outTemp, dewpoint
    column = avg
    
    # Default: both on y1
    [[[[yaxis]]]]
        outTemp = y1
        dewpoint = y1
    
    # Week page: show min/max for outTemp, avg for dewpoint
    # All still on y1 (same axis)
    [[[[week]]]]
        outTemp = min, max
        dewpoint = avg
    
    # Month page: separate axes for better visibility
    [[[[month]]]]
        outTemp = min, max
        dewpoint = avg
        
        # Override axis assignment for this page only
        yaxis = {
            "outTemp": "y1",
            "dewpoint": "y2"
        }
        yaxis_config = {
            "y1": {"title": "Outside Temperature (°C)"},
            "y2": {"title": "Dew Point (°C)", "opposite": true}
        }
```

**Note:** The per-page yaxis syntax using dictionaries may require Python-style quoting depending on your config parser.

## Backward Compatibility

### Existing Charts Continue to Work

All existing custom charts without multi-axis configuration will continue to work exactly as before:

```conf
# This still works - single axis, all fields share it
[[[customChart1]]]
    title = My Chart
    charttype = area
    column = avg
    values = outTemp, inTemp
```

### Automatic Defaults

If you don't specify axis assignments:
- All series are placed on `y1` (single axis behavior)
- Axis uses the first field's unit and format
- No axis titles are shown

## Best Practices

### 1. Use Multi-Axis When Scales Differ Significantly

✅ **Good use case:**
```conf
# UV (0-15) and Radiation (0-1000) - very different scales
values = UV, radiation
[[[[yaxis]]]]
    UV = y1
    radiation = y2
```

❌ **Not necessary:**
```conf
# OutTemp and InTemp - similar scales, same units
values = outTemp, inTemp
# Don't need multi-axis here, single axis works fine
```

### 2. Limit to 2-3 Axes

- 2 axes: Ideal for most use cases
- 3 axes: Acceptable if necessary, but chart gets busy
- 4+ axes: Avoid - chart becomes unreadable

### 3. Use Descriptive Axis Titles

✅ **Good:**
```conf
[[[[[y1]]]]]
    title = Temperature (°C)
[[[[[y2]]]]]
    title = Solar Radiation (W/m²)
```

❌ **Bad:**
```conf
[[[[[y1]]]]]
    title = Series 1
[[[[[y2]]]]]
    title = Data
```

### 4. Consistent Positioning

Convention:
- Primary/most important data: Left (y1, opposite = false)
- Secondary data: Right (y2, opposite = true)
- Additional data: Right (y3, y4, opposite = true)

### 5. Set Min/Max When Needed

For fields with known ranges, set explicit bounds:

```conf
[[[[[y1]]]]]
    title = UV Index
    min = 0
    max = 15        # UV never exceeds ~11-12, but 15 gives headroom

[[[[[y2]]]]]
    title = Humidity (%)
    min = 0
    max = 100       # Humidity is always 0-100%
```

## Troubleshooting

### Chart Not Displaying

**Problem:** Chart doesn't appear after adding multi-axis config.

**Solution:**
1. Check that all field names in `[[[[yaxis]]]]` match the fields in `values`
2. Verify axis IDs are y1, y2, y3 (lowercase, no spaces)
3. Check indentation in skin.conf (must be exact)

### Wrong Units on Axis

**Problem:** Axis shows wrong unit or format.

**Solution:**
- The axis uses the unit from the **first field** assigned to it
- Make sure the first field has the correct unit type
- Example: If y1 shows both temperate and humidity, it will use temperature's unit

### Axes Overlapping

**Problem:** Multiple axes render on top of each other.

**Solution:**
- Ensure `opposite = true` for y2, y3, etc.
- ApexCharts automatically adjusts spacing for up to 3-4 axes
- Beyond that, consider using multiple charts instead

### Data Not on Correct Axis

**Problem:** Series appears on wrong axis.

**Solution:**
- ApexCharts links series to axes via `seriesName`
- The system uses the **first field** on each axis as the seriesName
- If issues persist, check that field names are spelled correctly

## Technical Details

### How It Works

1. **Configuration Parsing** (Cheetah template)
   - Reads `yaxis` and `yaxis_config` sections from skin.conf
   - Builds `axis_map`: `{field_name: axis_id}`
   - Identifies unique axes used

2. **ApexCharts Configuration Generation**
   - If only one axis (y1): Uses single yaxis object (backward compatible)
   - If multiple axes: Generates yaxis array with one object per axis
   - Each axis object includes:
     - `seriesName`: Links to the first field on that axis
     - `title`: Axis label
     - `opposite`: Positioning (left/right)
     - `labels.formatter`: Unit formatting
     - `min/max`: Optional bounds

3. **Series Association**
   - ApexCharts matches each series to an axis by `seriesName`
   - Multiple series with the same seriesName share one axis

### Supported Templates

Multi-axis support is currently implemented in:
- ✅ `yesterday.html.tmpl` (Phase 1 - Complete)
- ⏳ `index.html.tmpl` (current day) - Coming soon
- ⏳ `week.html.tmpl` - Coming soon
- ⏳ `month.html.tmpl` - Coming soon
- ⏳ `year.html.tmpl` - Coming soon

### Configuration File Syntax Notes

The configuration uses nested sections in ConfigObj format:

```
[[[Level 3]]]           # customChart name
    [[[[Level 4]]]]     # yaxis or yaxis_config
        [[[[[Level 5]]]]]  # axis identifier (y1, y2, etc.)
```

**Indentation matters!** Use consistent spacing (4 spaces per level recommended).

## FAQ

### Q: Can I use more than 3 axes?

**A:** Technically yes (up to ~10), but practically not recommended. Charts with more than 3 axes become difficult to read. Consider splitting into multiple charts instead.

### Q: Can I have different axes on different pages?

**A:** Not directly, but you can work around it:
- Define axis config in page-specific section (`[[[[yesterday]]]]`, `[[[[week]]]]`, etc.)
- Currently requires separate chart definitions per page
- Future enhancement may support page-specific axis overrides

### Q: Do all chart types support multi-axis?

**A:** Yes! Multi-axis works with `area`, `bar`, and `line` chart types.

### Q: Can I use logarithmic scales?

**A:** Not yet. This is a planned future enhancement. See MULTI-AXIS-PLAN.md for roadmap.

### Q: Will this break my existing charts?

**A:** No! If you don't add multi-axis configuration, charts work exactly as before (single axis, all fields share it).

### Q: Can I change axis colors?

**A:** Not directly through the axis config. Axis colors follow the chart theme. A future enhancement may add axis color coordination with series colors.

## Additional Resources

- **ApexCharts Documentation:** https://apexcharts.com/docs/chart-types/multiple-yaxis-scales/
- **Implementation Plan:** See `MULTI-AXIS-PLAN.md` in the skin directory
- **Skin Configuration:** See `skin.conf` for example configurations

## Support

If you encounter issues:

1. Check this guide's Troubleshooting section
2. Verify your skin.conf syntax (indentation, spelling)
3. Check the WeeWX logs for template errors
4. Report issues on the project GitHub page

## Version History

- **v1.60.0**: Multi-axis support added for `yesterday.html.tmpl`
- **v1.59.0**: Initial custom chart implementation (single axis only)

---

**Last Updated:** 2026-05-28  
**Applies to:** neowx-material skin v1.60.0+

