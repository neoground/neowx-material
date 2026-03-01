# NeoWX Material

**The most modern and feature-rich skin for WeeWX weather stations**

[![Live Demo](https://img.shields.io/badge/Live-Demo-blue?style=flat-square)](https://weewx.seehausen.org/)
[![GitHub Issues](https://img.shields.io/github/issues/seehase/neowx-material?style=flat-square)](https://github.com/seehase/neowx-material/issues)
[![License](https://img.shields.io/github/license/seehase/neowx-material?style=flat-square)](LICENSE)

This actively maintained fork brings NeoWX Material into the modern era with **real-time MQTT updates**, **weather forecasting**, **comprehensive multi-language support**, and many more improvements.

> **Live Demo:** [weewx.seehausen.org](https://weewx.seehausen.org/)

---

## 🚀 Why This Fork?

This is an **actively maintained** continuation of the NeoWX Material skin. The original repository has not been maintained for years, so this fork provides:

✅ **Active development and bug fixes**
✅ **New major features** (MQTT real-time updates, weather forecasts, telemetry)
✅ **Complete multi-language support** (11 languages, all keys translated and sorted)
✅ **Modern features** (configurable UI, improved charts, better mobile experience)
✅ **Community-driven improvements**

---

## ✨ Major Features

### 🔴 Real-Time MQTT Updates
- **Live data updates** without page refresh
- Configurable flash effects on value changes
- Connection status indicator
- WebSocket support for instant updates

### 🌤️ Weather Forecast Integration
- **7-day forecast** powered by [Open-Meteo](https://open-meteo.com)
- Hourly weather icons and conditions
- Temperature, precipitation, wind, and sunshine predictions
- Fully customizable display

### 🌍 Complete Multi-Language Support
- **11 languages**: Catalan, Dutch, English, Finnish, French, German, Italian, Polish, Slovak, Spanish, Swedish
- All keys translated and professionally localized
- Easy to extend with additional languages

### 📊 Advanced Telemetry & Battery Monitoring
- Dedicated telemetry page for station health
- Battery status tracking for all sensors
- Historical battery trend charts
- Signal quality monitoring

### 🎨 Beautiful Material Design
- Modern, clean interface
- 20+ color schemes to choose from
- Auto dark mode (follows system settings)
- Responsive design for all devices

### 📈 Interactive Charts
- Zoomable and pannable charts powered by ApexCharts
- Configurable time ranges and data intervals
- Wind rose visualization
- Customizable colors and appearance

### 🔧 Highly Customizable
- Extensive configuration options in `skin.conf`
- Reorderable cards and charts
- Custom embedded iFrames and images
- Configurable hover effects and animations

### 📦 Additional Features
- Historical data archive (HTML + NOAA TXT)
- Update notifications
- Google Analytics / Tag Manager support
- Almanac with celestial body tracking
- Support for all WeeWX sensors

---

## 📚 Documentation

Comprehensive guides are available for advanced features:

### 🔴 [MQTT Real-Time Updates Guide](MQTT.md)
Set up live data updates without page refresh. Complete guide covering:
- MQTT broker installation (Mosquitto with Docker)
- WeeWX MQTT extension configuration
- WebSocket setup for browser connections
- Sensor mapping and troubleshooting

### 🔧 [Configuration Patcher Guide](CONFIG-PATCHER.md)
Never lose your settings again! Automate configuration updates with:
- Automatic settings preservation after skin updates
- Patch file creation and usage
- Automated weekly updates with cron jobs
- Real-world examples and best practices

### 🔋 [Battery Configuration Guide](battery-config-guide.md)
Monitor all your weather station sensors:
- Battery status tracking for multiple sensors
- Telemetry page setup
- Custom sensor configuration
- Low battery alerts and monitoring

**Quick Links:**
- [Installation](#-installation) • [Configuration](#️-configuration) • [Troubleshooting](#-contribution--support) • [Contributing](#-contribution--support)

---

## 📥 Installation

### Quick Install

1. **Install the extension:**
   ```bash
   weectl extension install https://github.com/seehase/neowx-material/archive/refs/heads/master.zip
   ```

2. **Restart WeeWX:**
   ```bash
   sudo systemctl restart weewx
   ```

3. **Done!** Your new skin should be active.

### Manual Skin Selection

If your skin doesn't change automatically, edit `weewx.conf` and set:
```ini
[[neowx-material]]
   skin = neowx-material
   enable = true
   HTML_ROOT = /var/www/html
```

Then reload WeeWX:
```bash
sudo systemctl reload weewx
```

### Optional: Enhanced Almanac Features

For advanced almanac features (moon phases, celestial body tracking, etc.), install the **weewx-skyfield-almanac** extension:

```bash
weectl extension install https://github.com/roe-dl/weewx-skyfield-almanac/archive/master.zip
```

This is a modern replacement for the deprecated PyEphem and provides:
- Accurate moon phase calculations
- Rise/set times for sun, moon, and planets
- Celestial body positions and visibility
- Eclipse predictions

**Note:** This extension is optional and not part of the skin itself. The skin works without it, but the almanac page will have limited functionality.

For more information, see: [weewx-skyfield-almanac](https://github.com/roe-dl/weewx-skyfield-almanac)

---

## 🌍 Localization

### Language Configuration

Set your preferred language in `weewx.conf`:
```ini
[StdReport]
    [[NeowxMaterial]]]
        lang = de  # Options: ca, de, en, es, fi, fr, it, nl, pl, se, sk
```

**Available Languages:**
- 🇬🇧 English (`en`)
- 🇩🇪 German (`de`)
- 🇪🇸 Spanish (`es`)
- 🇫🇷 French (`fr`)
- 🇮🇹 Italian (`it`)
- 🇳🇱 Dutch (`nl`)
- 🇵🇱 Polish (`pl`)
- 🇸🇪 Swedish (`se`)
- 🇸🇰 Slovak (`sk`)
- 🇫🇮 Finnish (`fi`)
- Plus Catalan (`ca`) for regional support

### Time & Date Formats

For localized time and date formats, ensure your locale is installed:

1. **Check installed locales:**
   ```bash
   locale -a
   ```

2. **Install your locale if missing:**
   ```bash
   # Debian/Ubuntu
   sudo dpkg-reconfigure locales

   # RHEL/CentOS
   sudo locale-gen <locale_name>
   ```

3. **Configure WeeWX systemd service:**
   Edit `/etc/systemd/system/weewx.service` and add under `[Service]`:
   ```ini
   [Service]
   Environment="LANG=de_DE.UTF-8"
   ```

   Then reload:
   ```bash
   sudo systemctl daemon-reload
   sudo systemctl restart weewx
   ```

---

## ⚙️ Configuration

All configuration is done in `skin.conf` under `[Extras]`. Here are the key areas:

### 🔧 Easy Configuration Updates

**Never lose your settings again!** Use the Configuration Patcher to automatically apply your customizations after updates:

```bash
# Apply your custom settings
./config_patcher.py skin.conf.patch skin.conf
```

📖 **For complete instructions**, see the [Configuration Patcher Guide](CONFIG-PATCHER.md)

### Basic Settings
- **Color scheme**: Choose from 20+ Material Design colors
- **Navigation links**: Show/hide pages in the menu
- **Footer content**: Customize name, links, and credits

### MQTT Real-Time Updates
Enable live updates without page refresh:
```ini
[Extras]
    [[MQTT]]
        enabled = true
        host = your-mqtt-broker.com
        port = 9001
        topic = "weewx/#"
        flash_on_update = true
        flash_color = "#00ff00"
```

📖 **For complete MQTT setup instructions**, see the [MQTT Setup Guide](MQTT.md)

### Weather Forecast
Get 7-day forecasts from Open-Meteo:
```ini
[Extras]
    [[Appearance]]
        # Add "forecast" to this list:
        values_order = forecast, outTemp, outHumidity, ...

    [[Forecast]]
        # Optional: Override coordinates for forecast
        # By default, uses station coordinates from weewx.conf [Station] section
        # Useful if you want forecast for a different location
        latitude =
        longitude =
        
        days = 7
        show_icon = yes
        variables = temperature, precipitation, wind, sun
```

**Coordinate Override:**
- By default, forecast uses your weather station's coordinates from `weewx.conf`
- Optionally specify different coordinates in `skin.conf` for forecast at another location
- Leave empty to use station coordinates (recommended for most users)

### Charts & Appearance
- Customize chart colors, time spans, and behavior
- Reorder cards and charts
- Configure hover effects and animations
- Add custom embedded content (iFrames, images)

For complete configuration options, see the comments in `skin.conf`.

---

## 🤝 Contribution & Support

### Reporting Issues

Found a bug or have a feature request?
👉 [Open an issue on GitHub](https://github.com/seehase/neowx-material/issues)

### Contributing

We welcome contributions! Here's how you can help:

1. **Translations**: Help translate to additional languages
2. **Bug fixes**: Submit pull requests with fixes
3. **Features**: Propose and implement new features
4. **Documentation**: Improve this README or add guides

**To contribute:**
1. Fork this repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request to `https://github.com/seehase/neowx-material`

### Adding a New Language

If your language isn't supported yet:
1. Copy an existing language file from `/lang/`
2. Translate all keys
3. Submit a pull request
4. See [WeeWX Localization Guide](https://weewx.com/docs/5.1/custom/localization/) for details

---

## 👨‍💻 Development

For developers who want to contribute or customize the skin:

### Setup

1. **Clone the repository:**
   ```bash
   git clone https://github.com/seehase/neowx-material.git
   cd neowx-material
   ```

2. **Install dependencies:**
   ```bash
   npm install
   # or
   yarn install
   ```

3. **Create symlink for testing:**
   ```bash
   ln -s $(pwd)/skins/neowx-material /etc/weewx/skins/neowx-material
   ```

### Build Scripts

| Script | Description |
|--------|-------------|
| `yarn run build-css` | Compile SCSS to CSS |
| `yarn run build-minify-css` | Create minified CSS |
| `yarn run build` | Full build (CSS + minified) |

### Technology Stack

- **Styling**: SCSS → CSS (Material Design for Bootstrap)
- **Charts**: [ApexCharts](https://apexcharts.com) (MIT)
- **Icons**: [Weather Icons by Erik Flowers](https://erikflowers.github.io/weather-icons/) (MIT/SIL OFL 1.1)
- **Date/Time**: [MomentJS](https://momentjs.com) (MIT)
- **Forecast**: [Open-Meteo API](https://open-meteo.com) (CC BY 4.0)
- **MQTT**: Paho MQTT JavaScript client

---

## 📜 Credits & License

### Original Work
This skin is based on the original [NeoWX Material](https://github.com/neoground/neowx-material) by Neoground GmbH, which itself was inspired by the NeoWX skin (based on Sofaskin).

### This Fork
Maintained by [@seehase](https://github.com/seehase) and contributors.

### Acknowledgments

- **Tom Keffer** and the WeeWX contributors for the excellent WeeWX platform
- **Material Design for Bootstrap (MDB)** for the design framework
- All the contributors who have helped improve this skin

### Third-Party Libraries

- [ApexCharts](https://github.com/apexcharts/apexcharts.js) - MIT License
- [MomentJS](https://github.com/moment/moment) - MIT License
- [Weather Icons](https://github.com/erikflowers/weather-icons) - MIT / SIL OFL 1.1
- [Open-Meteo](https://open-meteo.com) - CC BY 4.0

### License

This project maintains the original license from NeoWX Material.

---

## 📞 Contact & Links

- **Live Demo**: [weewx.seehausen.org](https://weewx.seehausen.org/)
- **Issues**: [GitHub Issues](https://github.com/seehase/neowx-material/issues)
- **Original Project**: [neoground/neowx-material](https://github.com/neoground/neowx-material) (not maintained)
- **WeeWX**: [weewx.com](https://weewx.com)

---

Made with ❤️ for the WeeWX community
