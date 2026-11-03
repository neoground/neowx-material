# AI Assistant & Pair Programming Instructions: neowx-material

This document provides architectural context, template rules, and development guidelines for the `neowx-material` WeeWX skin codebase.

---

## 1. Project Overview & Architecture

`neowx-material` is a modern, responsive Material Design skin and extension for the **WeeWX** weather station software.

### Technology Stack
- **Template Engine**: Cheetah 3 Python templating (`*.html.tmpl`, `*.json.tmpl`, `*.inc`).
- **Python Backend**: WeeWX search-list extensions and custom report generators located in `bin/user/`.
- **Styles**: SCSS / Sass (version `1.105.1`), compiled to `skins/neowx-material/css/style.css` and `style.min.css` via Yarn/npm.
- **Frontend / Client-Side**: Vanilla JavaScript + ApexCharts (`js/vendor/apexcharts/`), Material Design Bootstrap (MDB), and WebSockets/MQTT live updates (`js/weewx-mqtt.js`, `js/neowx-live.js`).

### Directory Structure
```
neowx-material/
├── AGENTS.md                          # AI context & contributor guidelines (this file)
├── install.py                         # WeeWX extension installer manifest (files_to_install)
├── update_version.sh                  # Project version bump script
├── bin/user/                          # Python extensions for WeeWX
│   ├── historygenerator.py           # Historical table generator
│   ├── neowx_archive.py              # Archive calculation helpers
│   ├── panelorder.py                 # Panel sorting and dynamic layout
│   ├── openmeteo.py                  # Forecast integration
│   ├── updatecheck.py / onlinecheck.py
├── skins/neowx-material/              # Skin root
│   ├── skin.conf                      # Skin configuration (Extras, Generators, Copy)
│   ├── skin.conf.patch.sample         # Sample configuration patch
│   ├── package.json                   # Build scripts for Sass / SCSS
│   ├── *.html.tmpl                    # Main entrypoint page templates (index, almanac, history, telemetry, day/week/month/year)
│   ├── *.json.tmpl                    # JSON endpoints (manifest.json.tmpl, live.json.tmpl)
│   ├── inc/                           # Dedicated partials folder (23 include files)
│   │   ├── head.inc, header.inc, footer.inc, js.inc, card_datetime.inc
│   │   ├── forecast.inc, telemetry.inc, panel_header.inc, embedded_region.inc, trend_indicator.inc
│   │   └── chart_*.inc, graph_*.inc  # 11 internal chart/graph config partials
│   ├── css/                           # Compiled CSS bundles
│   ├── scss/                          # Source styles (_neowx-material.scss, variables, themes)
│   ├── js/                            # Client-side scripts (app.js, weewx-mqtt.js, neowx-live.js)
│   ├── lang/                          # Localization dictionary files (*.conf)
│   └── archive/NOAA/                  # NOAA text summary templates
```

---

## 2. Development Guidelines & Constraints

### Template Partials & Cheetah Directives
- **All template partials live in `skins/neowx-material/inc/`**.
- When including a partial from any `.tmpl` or nested `.inc` file, **always** prefix with `inc/`:
  ```cheetah
  #include "inc/header.inc"
  #include "inc/graph_area_config.inc"
  ```
- Any new template or asset added to the skin **must** also be declared in `install.py` in `files=[("skins/neowx-material", [...])]`.

### Line Endings (CRLF vs LF)
- Some legacy archive templates and partials (`month-%Y-%m.html.tmpl`, `week-%Y-%V.html.tmpl`, `year-%Y.html.tmpl`, and several `graph_*.inc` files) historically use **CRLF** line endings.
- Always preserve each file's native line ending style (e.g., use `newline=""` in Python file operations) to prevent massive git diff churn.

### Styles & Asset Compilation
- Source stylesheets are located in `skins/neowx-material/scss/`.
- Never edit `css/style.css` or `css/style.min.css` directly. Edit SCSS and compile with:
  ```bash
  cd skins/neowx-material && yarn run build
  ```

### Code Formatting Caution
- Do **not** run `cheetah_template_beautifier.py` automatically across existing templates or partials without review; it alters Cheetah directive indentation.

### Version Management
- when changeing exteinsion in /bin/user always update the versions of the extenstions in the corresponding *.py files
- Versions are bumped using `./update_version.sh` (which synchronizes `skin.conf`, `install.py`, and `package.json`).

### Create plan first
- before implementing any changes create a plan.md file in the docs/plans/ folder and describe what you are going to do.  get my approval before implementing it.
