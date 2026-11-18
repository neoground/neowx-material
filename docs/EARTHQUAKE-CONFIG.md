# 🌍 Regional Earthquakes Map & USGS Feed Guide

NeoWX Material includes a built-in, privacy-first **Regional Earthquakes Map** widget powered by the public **USGS Earthquake API**, **USGS Quaternary Faults**, and **Leaflet 1.9.4**.

The earthquake map is delivered as an interactive dashboard card (`earthquake_map`) that expands into a full-screen modal dialog showing a side-by-side map, seismic summary metrics, and a clickable event list.

---

## ✨ Key Features

- **📍 Dashboard Card & Full-Screen Modal Expansion**:
  - Place `earthquake_map` inside any section in `[[[sections]]]` (included by default in `[[[[embedded_items]]]]`).
  - Clicking the card's expand button (`↗`) opens a wide responsive modal dialog (`modal-xl`) with an interactive Leaflet map on the left (`col-lg-7`) and summary KPIs (Event Count, Max Magnitude, Latest Event) + a scrollable earthquake list on the right (`col-lg-5`).
  - Clicking any earthquake in the list smoothly pans/flies the map to the epicenter and opens its detail popup (magnitude, depth, distance from station, timestamp, and direct USGS event link).
- **🔒 Privacy-First & 100% Offline Vector Basemap**:
  - Zero external CDN dependencies or tracking scripts — **Leaflet 1.9.4** and a bundled **Natural Earth** vector landmass dataset (`world-land.js`) are hosted locally with the skin.
  - Defaults to `basemap = vector` (100% offline vector landmass polygons with seamless antimeridian/date-line wrapping), or optionally switch to `basemap = osm` for **OpenStreetMap** raster tiles (or a custom tile server).
- **⛰️ USGS Quaternary Fault Lines Overlay**:
  - Automatically queries and overlays tectonic fault traces from the **USGS Quaternary Fault and Fold Database** within a `4×` extended regional envelope around your station.
  - Distinguishes well-constrained fault lines (solid) from inferred fault traces (dashed), complete with hover highlighting, fault metadata popups (name, section, age, slip rate, sense), and a top-right layer toggle control.
- **🎨 Theme & Auto Dark Mode Integration**:
  - Respects your skin's primary Material color (`$Extras.color`) for the station pin, coverage radius circle, and modal KPI badges.
  - Automatically adapts to `light`, `dark`, and `auto` (`prefers-color-scheme: dark`) themes, including a dark slate filter when using OpenStreetMap tiles at night.
- **⚡ Smart Client-Side Caching & Zero Report Overhead**:
  - Runs entirely client-side with zero WeeWX report generation overhead.
  - CSS and JS assets are loaded **only** on pages whose layout actually includes an `earthquake*` item.
  - Earthquake feeds are cached in `localStorage` for **15 minutes**, and fault line geometry is cached for **7 days** for instant repeat page loads.

---

## ⚙️ Quick Start Configuration (`skin.conf`)

Everything is configured in `skins/neowx-material/skin.conf` under `[Extras]`:

```ini
[Extras]
    [[Appearance]]
        [[[pages]]]
            [[[[today]]]]
                sections = cards_with_forecast, additional_cards, external_sensors_cards, charts, additional_charts, external_sensor_charts, embedded_items

        [[[sections]]]
            [[[[embedded_items]]]]
                title = Embedded Items
                items = earthquake_map, iFrame_lightnings, iFrame_windy, iFrame_rainviewer
                column = full

    [[Embedded]]
        [[[earthquake_map]]]
            title = Regional Earthquakes
            aspect_ratio = "4/3"
            latitude = {lat}
            longitude = {lon}

    [[Earthquakes]]
        # Search radius around your station coordinates (in kilometers)
        radius_km = 300

        # Minimum earthquake magnitude to display (e.g. 0.5, 1.0, 2.5, 4.0)
        min_magnitude = 2.5

        # Rolling time window in days (e.g. 1, 7, or 30 days)
        timespan_days = 7

        # Basemap type: "vector" (offline Natural Earth GeoJSON) or "osm" (OpenStreetMap tiles)
        basemap = vector

        # Optional custom raster tile URL template when basemap = osm
        # (e.g. https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png)
        custom_tile_url =

        # Show major tectonic plate boundaries / fault lines on the map? yes/no
        show_faults = yes
```

---

## 🧩 Configuration Reference

### 1. Global Settings (`[Extras] → [[Earthquakes]]`)

These settings provide the default behavior for the earthquake map across the skin:

| Option | Default | Description |
| :--- | :--- | :--- |
| `radius_km` | `300` | Search radius around the target coordinates in kilometers (`1` – `20000`). Determines both the USGS API query radius and the dashed coverage circle drawn on the map. |
| `min_magnitude` | `2.5` | Minimum Richter/moment magnitude filter (`0.0` or higher, e.g., `0.5`, `1.0`, `2.5`). Supports `0` to include all micro-earthquakes. |
| `timespan_days` | `7` | Number of past days to query from the USGS FDSN Event API (e.g., `1`, `7`, `14`, `30`). |
| `basemap` | `vector` | Map rendering mode:<br>• `vector` — 100% offline bundled Natural Earth GeoJSON landmass.<br>• `osm` — OpenStreetMap raster tiles (automatically styled for light/dark themes). |
| `custom_tile_url` | *(empty)* | Optional custom XYZ raster tile URL template used when `basemap = osm` (defaults to `https://tile.openstreetmap.org/{z}/{x}/{y}.png` if left empty). |
| `show_faults` | `yes` | `yes` / `no` — Whether to fetch and display the USGS Quaternary Faults overlay and layer toggle button. |

---

### 2. Card & Location Overrides (`[Extras] → [[Embedded]] → [[[earthquake_map]]]`)

You can customize the card appearance, override the center coordinates, or override any of the `[[Earthquakes]]` parameters directly inside the `[[[earthquake_map]]]` block:

| Option | Default | Description |
| :--- | :--- | :--- |
| `title` | `Regional Earthquakes` | Heading displayed at the top of the card and in the expanded popup modal. Leave empty (`title =`) to falls back to the localized `$gettext("earthquakes")` label. |
| `aspect_ratio` | `"4/3"` | CSS aspect ratio of the embedded card map (`"4/3"`, `"16/9"`, `"1/1"`). Quotes are recommended. |
| `latitude` | `{lat}` | Target latitude for the map center and USGS search. `{lat}` or `{latitude}` dynamically resolves to your WeeWX station's `$station.latitude_f`. Set an explicit number (e.g. `47.3769`) to monitor a different region. |
| `longitude` | `{lon}` | Target longitude for the map center and USGS search. `{lon}` or `{longitude}` dynamically resolves to your WeeWX station's `$station.longitude_f`. Set an explicit number (e.g. `8.5417`) to monitor a different region. |
| `items_title_align` | *(inherits)* | Optional per-card heading alignment override: `center`, `left`, or `center-adjusted`. |
| `radius_km`, `min_magnitude`, `timespan_days`, `basemap`, `custom_tile_url`, `show_faults` | *(inherits from `[[Earthquakes]]`)* | Any setting from `[[Earthquakes]]` can also be placed directly inside `[[[earthquake_map]]]` (useful when managing overrides in `skin.conf.patch`). |

---

## 💡 Example Setups

### Example A: Low-Seismicity Region (Central Europe) with Wider Radius & Micro-Quakes
In regions with lower seismic activity, increase `radius_km` and lower `min_magnitude` to capture smaller regional events, and use OpenStreetMap tiles for detailed street/terrain context:

```ini
    [[Embedded]]
        [[[earthquake_map]]]
            title = Europa Earthquakes
            aspect_ratio = "4/3"
            latitude = {lat}
            longitude = {lon}
            radius_km = 1200
            min_magnitude = 0.5
            timespan_days = 7
            basemap = osm
```

### Example B: Monitoring a Specific Seismic Zone (Custom Coordinates)
Even if your weather station is located elsewhere, you can point the earthquake map at a specific seismic region (for example, Iceland, Italy, or Alaska) by setting explicit `latitude` and `longitude` coordinates:

```ini
    [[Embedded]]
        [[[earthquake_map]]]
            title = Iceland Seismic Activity
            aspect_ratio = "16/9"
            latitude = 64.9631
            longitude = -19.0208
            radius_km = 400
            min_magnitude = 1.5
            timespan_days = 14
```

### Example C: Mixing the Earthquake Card into a Charts or Cards Section
Like any `[[Embedded]]` item, `earthquake_map` can be placed in **any** section (`content = card` or `content = chart`), or in a full-width bottom panel (`column = full`):

```ini
        [[[sections]]]
            [[[[embedded_items]]]]
                title = Weather & Seismic Maps
                items = earthquake_map, iFrame_windy, iFrame_rainviewer, iFrame_lightnings
                column = full
```

---

## 🎨 Magnitude Color Scale

Earthquake markers on the map and badges in the modal list are color-coded by magnitude and scaled proportionally:

| Magnitude | Color | Hex |
| :--- | :--- | :--- |
| **M < 3.0** | 🟢 Green | `#388e3c` |
| **M 3.0 – 3.9** | 🟡 Amber | `#ffa000` |
| **M 4.0 – 4.9** | 🟠 Dark Orange | `#f57c00` |
| **M ≥ 5.0** | 🔴 Red | `#d32f2f` |

---

## 📜 Data Sources & Attribution

- **Seismic Events:** [USGS Earthquake Hazards Program (FDSN Event Web Service)](https://earthquake.usgs.gov/fdsnws/event/1/) — U.S. Public Domain.
- **Tectonic Fault Lines:** [USGS Quaternary Fault and Fold Database (ArcGIS REST Service)](https://earthquake.usgs.gov/cfusion/qfault/) — U.S. Public Domain.
- **Interactive Map Engine:** [Leaflet 1.9.4](https://leafletjs.com/) — BSD-2-Clause License.
- **Vector Basemap:** [Natural Earth](https://www.naturalearthdata.com/) — Public Domain.
- **Raster Basemap (when `basemap = osm`):** © [OpenStreetMap](https://www.openstreetmap.org/copyright) contributors — ODbL.
