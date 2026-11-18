# Center Earthquake Map on Station Plan

## 1. Problem Statement & Root Cause
In `skins/neowx-material/js/earthquakes.js`, `populateMapAndList` previously executed:
```javascript
map.fitBounds(eventBounds, { padding: isEmbed ? [16, 16] : [28, 28], maxZoom: 8.5 });
```
Where `eventBounds` was computed by extending a bounding box around all received earthquake coordinates (`eventBounds.extend([qLat, qLon])`).

When a weather station is situated in a region where earthquake activity is geographically asymmetric (e.g., in Northern/Central Europe where quakes are concentrated far to the south in the Alps, Italy, and Balkans; or along oceanic coasts), `eventBounds` centers on the midpoint between the station and the far-off quakes.

As reported by `@seehausen` on his live station (https://weewx.seehausen.org/) with `radius = 1200`:
> *"There I see a strange behavior: set it to 1200, zoom changed, but it is not centered. I would expect to see 'my place' in the middle?"*

The station pin is pushed to the top or corner edge of the card instead of remaining the central focal point of the station's monitoring radius.

## 2. Proposed Solution
1. **Anchor Viewport to Coverage Circle:**
   The station's observation perimeter is defined by `opts.radiusKm` via `coverageCircle = addCoverageCircle(map, cachedOptions)`.
   Because `coverageCircle` is an `L.circle` centered strictly at `[opts.stationLat, opts.stationLon]`, its geographic bounding box (`coverageCircle.getBounds()`) is mathematically symmetric around the station pin.
2. **Preserve Center on Data Load:**
   Store `map._coverageCircle = coverageCircle;` on the map instance.
   In `populateMapAndList`, fit `map._coverageCircle.getBounds()` (with padding) instead of the asymmetric `eventBounds`.
   If the user has manually panned or zoomed (`map._hasUserPanned`), preserve their viewport across background data updates.
3. **Modal Interactive Selection:**
   Clicking any event in the list still flies directly to that earthquake's epicenter (`map.flyTo([lat, lon], 8)`).

## 3. Verification
1. Validate syntax with `node -c skins/neowx-material/js/earthquakes.js`.
2. Test both small radii (`radius = 300`) and large radii (`radius = 1200`), verifying the station pin remains dead center with symmetric circle padding on the card.
