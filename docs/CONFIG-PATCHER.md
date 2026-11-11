# Configuration Patcher Guide

The Configuration Patcher is a powerful tool that allows you to **automatically apply your custom settings** to `skin.conf` after updates, eliminating the need to manually reconfigure everything.

## 🎯 Why Use the Configuration Patcher?

### The Problem

When you update NeoWX Material to a new version, you face a dilemma:

❌ **Keep your old `skin.conf`** → Miss out on new features and improvements
❌ **Use the new `skin.conf`** → Lose all your custom settings and have to reconfigure manually

### The Solution

✅ **Use the Configuration Patcher** → Automatically merge your custom settings into the new configuration file!

### Benefits

- 🚀 **Automatic updates** - Apply your settings with one command
- 🔒 **Preserve customizations** - Never lose your personal configuration
- ⚡ **Fast** - No manual editing required
- 🎯 **Selective** - Only patch what you've changed
- 🔄 **Repeatable** - Use the same patch file after every update
- 📝 **Version control friendly** - Track your changes in a small patch file

---

## 🚀 Quick Start

### 1. Run the Patcher

```bash
# From the skin directory
cd /etc/weewx/skins/neowx-material

# Apply your patch
./config_patcher.py skin.conf skin.conf.patch

# Or with explicit Python interpreter
python3 config_patcher.py skin.conf skin.conf.patch
```

### 2. Restart WeeWX

```bash
sudo systemctl restart weewx
```

That's it! Your custom settings are now applied to the updated configuration file.

---

## 🔄 Updating the Skin

### Manual Update

When a new version of NeoWX Material is released, update it and automatically reapply your settings:

```bash
# 1. Install the latest version
weectl extension install https://github.com/seehase/neowx-material/archive/refs/heads/master.zip

# 2. Navigate to skin directory
cd /etc/weewx/skins/neowx-material

# 3. Apply your custom settings
./config_patcher.py skin.conf skin.conf.patch

# 4. Restart WeeWX
sudo systemctl restart weewx
```



---

## 📖 How It Works

### Concept

The patcher reads your **patch file** (e.g., `skin.conf.patch`) and applies only the differences to the target configuration file (`skin.conf`).

**Flow:**
```
┌─────────────────────┐
│  skin.conf.patch    │  Your custom settings
│  (your changes)     │
└──────────┬──────────┘
           │
           │ Apply patches
           ▼
┌─────────────────────┐
│    skin.conf        │  Base configuration
│  (from update)      │  (with new features)
└──────────┬──────────┘
           │
           │ Merge
           ▼
┌─────────────────────┐
│    skin.conf        │  Updated config with
│  (patched result)   │  your custom settings
└─────────────────────┘
```

### What Gets Patched?

- ✅ **Values** - Any setting in your patch file overwrites the target
- ✅ **New sections** - Sections only in patch file are added
- ✅ **Nested values** - Deep configuration structures are merged
- ✅ **Comments** - Preserved from original file
- ⚠️ **Deletions** - Settings not in patch file remain unchanged

> ⚠️ **`[[[sections]]]` is a section, not a value - it merges, it doesn't replace.**
> A `[[[[cards]]]]` block in your patch merges INTO the shipped `[[[[cards]]]]`
> block (so patching its `items` replaces that section's item list exactly,
> as you'd expect). But a section id that ISN'T one of the shipped ones is
> merged in as a brand-new, *additional* section - it does not take over an
> existing panel, and its items get de-duplicated away if a shipped section
> already lists them (items are first-occurrence-wins, in section order).
> Always reuse one of the nine shipped ids to override a panel:
> `card`, `external_sensors`, `chart`, `custom_charts`, `additional_charts`,
> `external_sensor_charts`, `other_charts`, `telemetry_cards`,
> `telemetry_charts`.
> `merge_configs` has no delete path, so a patch cannot remove a shipped
> section outright. You can still **suppress** one without touching
> `skin.conf`: override its `items` with an empty value, and the section is
> skipped because a section with no items never renders. To drop the
> `external_sensors` panel, patch it to nothing:
>
> ```ini
> [Extras]
>   [[Appearance]]
>     [[[sections]]]
>       [[[[external_sensors]]]]
>         items =
> ```
>
> The section node survives in the merged config but produces no output, and
> its former items are left for later sections to claim, since a skipped
> section never enters the de-duplication set.

---

## 📝 Creating a Patch File

### Basic Structure

A patch file follows the **same structure** as the file you want to patch, but contains **only the values you want to change**.

**Example `skin.conf.patch`:**

```ini
[Extras]
    color = indigo

    [[Header]]
        custom1_label = My Website
        custom1_url = https://example.com/
        google_tagmanager_enable = yes
        google_tagmanager_id = GTM-XXXXXXX

    [[Footer]]
        name = My Weather Station
        link1_label = Lightning Maps
        link1_url = https://www.lightningmaps.org
        update_check = patch

    [[Appearance]]
        enable_hover_effect = true
        defaultChartBehavior = pan
        # [[[sections]]] must stay the LAST thing under [[Appearance]] -
        # configobj reparents anything written after it into the last
        # [[[[...]]]] block instead of into [[Appearance]].
        [[[sections]]]
            [[[[cards]]]]
                items = outTemp, outHumidity, forecast, barometer, windSpeed, rain

    [[Forecast]]
        days = 7
        show_icon = yes
        variables = temperature, precipitation, wind, sun

    [[MQTT]]
        enabled = true
        host = mqtt.example.com
        port = 9001
        username = weewx-readonly
        password = mysecretpassword

[Labels]
    [[Generic]]
        extraTemp1 = Greenhouse Temperature
        extraTemp2 = Soil Temperature
```

### Key Principles

1. **Include only what you change** - Don't copy the entire file
2. **Maintain hierarchy** - Keep the section structure (`[Section]`, `[[Subsection]]`)
3. **Use exact keys** - Key names must match exactly (case-sensitive)
4. **One value per line** - Standard INI format

---

## 🔍 Understanding the Patch File Structure

### Section Hierarchy

The patch file uses the same hierarchical structure as `skin.conf`:

```ini
[TopLevelSection]           # Single brackets = top level
    key = value

    [[SubSection]]          # Double brackets = subsection
        key = value

        [[[DeepSection]]]   # Triple brackets = deeper level
            key = value
```

**Example:**
```ini
[Extras]                    # Top level
    color = blue

    [[Header]]              # Subsection under [Extras]
        custom1_label = Link

    [[MQTT]]                # Another subsection under [Extras]
        enabled = true

        [[[SensorMapping]]] # Subsection under [[MQTT]]
            outTemp = "outTemp_C", "°C", 1
```

### Value Types

The patcher supports all configuration value types:

**Strings:**
```ini
name = My Weather Station
color = indigo
```

**Booleans:**
```ini
enabled = true
show_icon = yes
debug = false
```

**Numbers:**
```ini
port = 9001
days = 7
update_interval = 1440
```

**Lists (comma-separated):**
```ini
items = outTemp, outHumidity, barometer, windSpeed
variables = temperature, precipitation, wind
```

**Quoted strings (URLs, paths, special characters):**
```ini
url = "https://example.com/path?param=value"
flash_color = "#00ff00"
```

---

## 📋 Step-by-Step: Creating Your Patch File

### Step 1: Start with Your Current Configuration

Open your working `skin.conf` and identify what you've customized.

### Step 2: Create the Patch File

```bash
cd /etc/weewx/skins/neowx-material
nano skin.conf.patch
```

### Step 3: Copy Section Headers

Start by copying the section structure for settings you've changed:

```ini
[Extras]
    [[Header]]
    [[Footer]]
    [[Appearance]]
    [[MQTT]]
```

### Step 4: Add Your Custom Values

Fill in only the values you've changed from defaults:

```ini
[Extras]
    color = teal

    [[Header]]
        custom1_label = My Link
        custom1_url = https://mysite.com
        google_analytics_enable = yes
        google_analytics_id = G-XXXXXXXXX

    [[Footer]]
        name = John's Weather Station
        link1_label = Weather Underground
        link1_url = https://wunderground.com
        support_weewx = yes
        support_skin = yes

    [[Appearance]]
        enable_hover_effect = false
        defaultChartBehavior = pan
        [[[sections]]]
            [[[[cards]]]]
                items = forecast, outTemp, outHumidity, barometer, windSpeed
            [[[[custom_charts]]]]
                items = outTemp, windSpeed, barometer, rain
                title = Custom Charts
                content = chart
```

### Step 5: Save and Test

```bash
# Backup your original config first!
cp skin.conf skin.conf.backup

# Apply the patch
./config_patcher.py skin.conf skin.conf.patch

# Check the result
cat skin.conf | grep "color ="
cat skin.conf | grep "custom1_label"
```

---

## 💡 Real-World Examples

### Example 1: Basic Customization

**Scenario:** You only want to change the color scheme, station name, and enable MQTT.

**`skin.conf.patch`:**
```ini
[Extras]
    color = purple

    [[Footer]]
        name = Mountain View Weather

    [[MQTT]]
        enabled = true
        host = mqtt.local
        port = 9001
```

### Example 2: Custom Sensor Labels

**Scenario:** You have extra temperature sensors with custom names.

**`skin.conf.patch`:**
```ini
[Labels]
    [[Generic]]
        extraTemp1 = Garage Temperature
        extraTemp2 = Attic Temperature
        extraTemp3 = Pool Temperature
        extraHumid1 = Garage Humidity
```

### Example 3: Forecast and Appearance

**Scenario:** Enable 7-day forecast and customize card order.

**`skin.conf.patch`:**
```ini
[Extras]
    [[Appearance]]
        [[[sections]]]
            [[[[cards]]]]
                items = forecast, outTemp, outHumidity, barometer, windSpeed, rain, UV
            [[[[custom_charts]]]]
                items = outTemp, barometer, windSpeed, rain, windDir
                title = Custom Charts
                content = chart

    [[Forecast]]
        days = 7
        show_icon = yes
        show_description = yes
        variables = temperature, precipitation, uv-sun, wind
        forecast_separator = yes
        show_hourly_icons = yes
```

### Example 4: Full MQTT Configuration

**Scenario:** Complete MQTT setup with sensor mapping.

**`skin.conf.patch`:**
```ini
[Extras]
    [[MQTT]]
        enabled = true
        debug = false
        host = mqtt.example.com
        port = 9001
        topic = "weewx/#"
        use_ssl = true
        use_websocket = true
        websocket_path = /mqtt
        username = weewx-readonly
        password = secretpass123
        flash_on_update = true
        flash_color = "#00ff00"

        [[[SensorMapping]]]
            outTemp = "outTemp_C", "°C", 1
            outHumidity = "outHumidity", "%", 0
            barometer = "barometer_mbar", " hPa", 1
            windSpeed = "windSpeed_kph", " km/h", 0
            windGust = "windGust_kph", " km/h", 1
            rain = "dayRain_cm", " mm", 1
            rainRate = "rainRate_cm_per_hour", " mm/h", 2
            UV = "UV", "", 1
```

### Example 5: Custom Navigation and Links

**Scenario:** Add custom navigation links and footer links.

**`skin.conf.patch`:**
```ini
[Extras]
    [[Header]]
        custom1_label = Solar Panels
        custom1_url = https://solar.example.com/
        custom2_label = Webcam
        custom2_url = https://webcam.example.com/
        google_tagmanager_enable = yes
        google_tagmanager_id = GTM-ABC1234

    [[Footer]]
        name = Smith Family Weather
        link1_label = RadarScope
        link1_url = https://radarscope.example.com
        link2_label = Windy
        link2_url = https://windy.com/48.123/-122.456
        link3_label = Weather.gov
        link3_url = https://weather.gov
```

---

## 🎨 Advanced Usage

### Combining Multiple Changes

You can patch multiple sections at once:

```ini
[Extras]
    color = indigo

    [[Header]]
        custom1_label = Solar
        custom1_url = https://solar.local

    [[Footer]]
        name = My Station

    [[Appearance]]
        enable_hover_effect = false
        [[[sections]]]
            [[[[cards]]]]
                items = forecast, outTemp, outHumidity

    [[Forecast]]
        days = 7
        show_hourly_icons = yes

    [[MQTT]]
        enabled = true
        host = mqtt.local

[Labels]
    [[Generic]]
        extraTemp1 = Custom Sensor 1
        extraTemp2 = Custom Sensor 2

[CheetahGenerator]
    search_list_extensions = user.historygenerator.MyXSearch, user.openmeteo.Forecast
```

### Selective Patching

Only patch specific subsections:

```ini
[Extras]
    # Don't include color or other top-level settings

    [[MQTT]]
        # Only patch MQTT settings
        enabled = true
        host = mqtt.local
        port = 9001
```

The patcher will leave all other settings untouched.

---

## 🔧 Workflow for Updates

### Recommended Update Process

1. **Backup your current patch file:**
   ```bash
   cp skin.conf.patch skin.conf.patch.backup
   ```

2. **Update NeoWX Material:**
   ```bash
   weectl extension install https://github.com/seehase/neowx-material/archive/refs/heads/master.zip
   ```

3. **Apply your patch:**
   ```bash
   cd /etc/weewx/skins/neowx-material
   ./config_patcher.py skin.conf skin.conf.patch
   ```

4. **Review changes:**
   ```bash
   # Check specific settings
   grep "color =" skin.conf
   grep "enabled =" skin.conf

   # Or use diff to see all changes
   diff skin.conf.backup skin.conf
   ```

5. **Restart WeeWX:**
   ```bash
   sudo systemctl restart weewx
   ```

### Version Control

Consider storing your patch file in version control:

```bash
# Initialize git repository
cd /etc/weewx/skins/neowx-material
git init
git add skin.conf.patch
git commit -m "Initial custom configuration"

# After making changes
git add skin.conf.patch
git commit -m "Updated MQTT settings"
```

---

## 🛠️ Troubleshooting

### Problem: Patch Not Applied

**Symptoms:** After running patcher, your settings don't appear in `skin.conf`.

**Solutions:**

1. **Check section hierarchy:**
   ```bash
   # View your patch file structure
   grep "^\[" skin.conf.patch

   # Compare with target file
   grep "^\[" skin.conf
   ```
   Ensure brackets match exactly.

2. **Verify key names:**
   ```bash
   # Check if key exists in target
   grep "custom1_label" skin.conf
   ```
   Key names are case-sensitive.

3. **Check indentation:**
   Subsections must be properly indented with spaces (not tabs).

### Problem: Syntax Errors

**Symptoms:** WeeWX fails to start after patching.

**Solutions:**

1. **Restore backup:**
   ```bash
   cp skin.conf.backup skin.conf
   ```

2. **Validate patch file:**
   ```bash
   # Check for common issues
   grep -n "= $" skin.conf.patch  # Empty values
   grep -n "^[^# \[]" skin.conf.patch  # Unindented keys
   ```

3. **Test incrementally:**
   Comment out sections in patch file to identify problematic line.

### Problem: Duplicate Entries

**Symptoms:** Same key appears multiple times in result.

**Solution:** The patcher replaces values, not duplicates them. If you see duplicates:

```bash
# Check for duplicates
grep -n "custom1_label" skin.conf
```

If found, manually remove duplicates or recreate patch file.

### Problem: Special Characters

**Symptoms:** Values with special characters not applied correctly.

**Solution:** Use quotes for special characters:

```ini
# Incorrect
flash_color = #00ff00  # The # starts a comment!

# Correct
flash_color = "#00ff00"

# URLs with special characters
custom1_url = "https://example.com/path?param=value&other=123"
```

---

## 📚 Complete Example

Here's a complete example patch file with common customizations:

**`skin.conf.patch`:**

```ini
[Extras]
    # Skin color scheme
    color = indigo

    [[Header]]
        # Custom navigation links
        custom1_label = Solar Panels
        custom1_url = https://solar.example.com/
        custom2_label = Webcam
        custom2_url = https://webcam.example.com/

        # Analytics
        google_analytics_enable = yes
        google_analytics_id = G-ABC123XYZ

        # Auto refresh
        auto_refresh_enable = yes
        auto_refresh_seconds = 300

    [[Footer]]
        # Station information
        name = Mountain View Weather

        # Custom footer links
        link1_label = Lightning Maps
        link1_url = https://www.lightningmaps.org
        link2_label = Windy
        link2_url = https://www.windy.com/48.123/-122.456
        link3_label = Weather Underground
        link3_url = https://wunderground.com

        # Credits
        support_weewx = yes
        support_skin = yes

        # Update check
        update_check = patch
        update_interval = 1440

    [[Appearance]]
        # Enable hover effects
        enable_hover_effect = true

        # Chart behavior
        defaultChartBehavior = pan

        # Custom value colors
        lo_value_color = "#03a9f4"
        hi_value_color = "#f44336"

        # Card and chart order (with forecast enabled). [[[sections]]] must
        # come after every other [[Appearance]] setting above, and each
        # [[[[...]]]] id below must be one of the shipped ids (here: card,
        # custom_charts) so it overrides that panel instead of being
        # appended as an unused extra section - see "What Gets Patched?".
        [[[sections]]]
            [[[[cards]]]]
                items = forecast, outTemp, outHumidity, barometer, windSpeed, rain, UV, radiation
            [[[[custom_charts]]]]
                items = outTemp, barometer, windSpeed, windDir, rain, UV, radiation, outHumidity
                title = Custom Charts
                content = chart

    [[Embedded]]
        # Custom embedded content
        [[[iFrame_windy]]]
            url = "https://embed.windy.com/embed.html?type=map&location=coordinates&zoom=8&overlay=wind&product=ecmwf&lat=48.123&lon=-122.456"
            title = Windy Map
            aspect_ratio = "16/9"

    [[Forecast]]
        # Forecast settings
        timezone = auto
        variables = temperature, precipitation, uv-sun, wind, evapotranspiration
        show_icon = yes
        show_description = yes
        days = 7
        forecast_separator = yes
        show_today_tomorrow = yes
        show_hourly_icons = yes
        hourly_icons_interval = 3

    [[MQTT]]
        # MQTT real-time updates
        enabled = true
        debug = false

        # Connection settings
        host = mqtt.example.com
        port = 9001
        topic = "weewx/#"
        use_ssl = true
        use_websocket = true
        websocket_path = /mqtt

        # Authentication
        username = weewx-readonly
        password = secretpassword123

        # Visual effects
        flash_on_update = true
        flash_color = "#00ff00"

        # Sensor mapping
        [[[SensorMapping]]]
            outTemp = "outTemp_C", "°C", 1
            outHumidity = "outHumidity", "%", 0
            barometer = "barometer_mbar", " hPa", 1
            windSpeed = "windSpeed_kph", " km/h", 0
            windGust = "windGust_kph", " km/h", 1
            rain = "dayRain_cm", " mm", 1
            rainRate = "rainRate_cm_per_hour", " mm/h", 2
            UV = "UV", "", 1

[Labels]
    # Custom sensor labels
    [[Generic]]
        extraTemp1 = Greenhouse Temperature
        extraHumid1 = Greenhouse Humidity
        extraTemp2 = Soil Temperature
        extraHumid2 = Soil Humidity
        extraTemp3 = Attic Temperature
```

**Usage:**

```bash
# Apply this patch
./config_patcher.py skin.conf skin.conf.patch

# Restart WeeWX
sudo systemctl restart weewx
```

---

## 📖 Reference

### Patch File Location

- **Recommended:** `/etc/weewx/skins/neowx-material/skin.conf.patch`
- **Alternative:** Store in your home directory and specify full path

### Patcher Script Location

- **Installed location:** `/etc/weewx/skins/neowx-material/config_patcher.py`
- **Executable:** Make sure it has execute permissions (`chmod +x config_patcher.py`)

### Command Syntax

```bash
# Basic syntax
./config_patcher.py <source_file> <patch_file>

# With Python interpreter
python3 config_patcher.py <source_file> <patch_file>

# Full paths
python3 /etc/weewx/skins/neowx-material/config_patcher.py \
    /etc/weewx/skins/neowx-material/skin.conf \
    /path/to/skin.conf.patch
```

### Common Sections to Patch

| Section | Purpose | Common Settings |
|---------|---------|-----------------|
| `[Extras]` → `color` | Skin theme | Color name |
| `[[Header]]` | Navigation | Custom links, analytics |
| `[[Footer]]` | Footer | Name, links, credits |
| `[[Appearance]]` | UI layout | Card order, hover effects |
| `[[Forecast]]` | Weather forecast | Days, variables, icons |
| `[[MQTT]]` | Real-time updates | Connection, sensors |
| `[Labels]` → `[[Generic]]` | Sensor names | Custom labels |

---

## 🎓 Best Practices

1. ✅ **Start small** - Begin with a few settings, expand gradually
2. ✅ **Comment your patch** - Add comments explaining non-obvious settings
3. ✅ **Version control** - Keep your patch file in git or backup
4. ✅ **Test after updates** - Always verify settings after patching
5. ✅ **Backup first** - Keep a backup of working `skin.conf`
6. ✅ **Document changes** - Note why you changed each setting
7. ✅ **Use quotes** - Quote values with special characters (#, &, ?, etc.)
8. ✅ **Consistent spacing** - Use spaces for indentation (not tabs)

---

## 🆘 Getting Help

If you encounter issues:

1. **Check syntax:** Ensure proper INI format and indentation
2. **Validate structure:** Section hierarchy must match target file
3. **Review examples:** Use provided examples as templates
4. **Test incrementally:** Comment out sections to isolate problems
5. **Ask for help:** [Open an issue on GitHub](https://github.com/seehase/neowx-material/issues)

---

## 📝 Summary

The Configuration Patcher enables **effortless updates** while preserving your customizations:

✅ Create a `skin.conf.patch` file with only your changes
✅ Run `./config_patcher.py skin.conf skin.conf.patch` after updates
✅ Restart WeeWX and enjoy your personalized configuration

**Never manually reconfigure your weather station again!** 🎉

---

**Made with ❤️ for the WeeWX community**
