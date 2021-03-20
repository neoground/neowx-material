# Changelog

## 1.7 (unreleased)

### Added

- Almanac in the header can be disabled
- Almanac can also be shown as link in navigation menu
- All navigation menu links can be enabled / disabled
- You can define 2 custom links in the navigation menu
- Catalan translation
- Missing battery status values to telemetry page

### Changed

- font-small text is now slightly smaller (0.85rem)
  to fix the full HD layout
- Better year archive layout with improved overview on Full HD screens,
  better visibility of month archives
- Improved the header + footer layout

### Fixed

- The almanac page won't throw an error anymore if the 
  pyephem package is not available


## 1.6 (2021-03-14)

### Added

- Finnish translation
- Timespans of graph data can now be adjusted in skin.conf

### Changed

- Apexcharts common config is now centralized in js.inc
- Graph config refactoring
- Structure of skin.conf: better subsections and comments


## 1.5 (2021-03-12)

### Added

- Italian translation
- Horizontal trend arrow if trend = 0

### Changed

- Month archive charts now show the same layout as year 
  charts in the archive (full date, better performance)
  
### Fixed

- Barometer trend can now also be None without throwing an error
- Max value of radiation is now shown correctly
- Partly missing data on a page will now result in a correct
  graph with "null" values instead of just ignoring missing data

### Removed

- Graph animations due to high amount of data / problems with 
  displaying "null" values which increases performance a lot


## 1.4 (2021-03-08)

### Added

- Unit labels to y-axis of charts
- Numbers in charts now have units
- Missing favicons
- Skin version
- Imprint / privacy links in the footer can be set in skin.conf
- Custom logo URL can be set in skin.conf

### Changed

- iOS Webapp appearance is now better and includes a splash screen
- Numbers in charts have the same decimals as the display values


## 1.3 (2021-03-07)

### Added

- Localization support for chart dates (moment.js)
- Charts color palette selection on skin.conf
- Wind direction indicator to all pages

### Changed

- Buttons / headings are now uniform
- Almanac header layout
- Chart numbers are now formatted as the display values

### Fixed

- The responsive layout on footer
- Rain rate / avg label
- Wind direction on archive pages


## 1.2 (2021-03-03)

### Added

- Translucent status bar on ios devices

### Fixed

- UTF8 encoding now set on template pages
- Chart labels can now contain an apostrophe


## 1.1 (2021-03-02)

### Added

- Telemetry page
- Telemetry page link in footer


## 1.0 (2021-03-01)

- Initial release
