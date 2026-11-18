# Antimeridian (International Date Line) Vector & Basemap Support Plan

## 1. Problem Statement & Root Cause Analysis

When a weather station is situated near the antimeridian / International Date Line (e.g., the Aleutian Islands in Alaska at ~176°W to 173°E, Fiji at ~178°E, or Chukotka at ~179°W), three distinct failure modes occur in Leaflet and GeoJSON processing:

1. **Leaflet Vector Marker Disconnect (Off-Screen Placement):**
   - In Leaflet, tile layers (`L.tileLayer`) support continuous horizontal wrapping across worlds (`wrap: true`).
   - Vector layers (`L.circleMarker`, `L.circle`, `L.geoJSON`, `L.polyline`), however, are rendered on an SVG canvas using literal, unwrapped coordinate math (`map.latLngToLayerPoint`).
   - If a station is at `-178°` (West) and an earthquake occurs 2° to the west at `+179°` (East), Leaflet plots the marker at `+179°` (357° to the right on the far side of the projection), rather than adjacent at `-181°`.

2. **Bounding Box Auto-Zoom Blowup:**
   - When building `eventBounds` via `L.latLngBounds` with coordinates spanning the date line (e.g., `[-179°]` and `[+179°]`), `bounds.extend()` computes a span of `358°` longitude instead of `2°`.
   - This causes `map.fitBounds(eventBounds)` to zoom out to level 0 or 1, displaying the entire world instead of the local 300 km regional cluster.

3. **Vector Basemap Void (Cut-off at ±180°):**
   - Natural Earth GeoJSON polygons (`ne_110m_land`) and USGS Quaternary Fault lines have coordinates strictly bounded within `[-180.0, +180.0]`.
   - When the map view is centered near the antimeridian, the viewport spans beyond `±180°` (e.g., `[-195°, -165°]`). Because no geometries exist in the `< -180°` or `> +180°` coordinate space, the map displays a blank, dark void on the wrapped side of the date line.

---

## 2. Proposed Architecture & Solution Strategy

### A. Relative Longitude Normalization Helper (`wrapLon`)
Introduce a standard coordinate unwrapper relative to a reference longitude (`refLon` = `stationLon`):

```javascript
function wrapLon(lon, refLon) {
  var diff = lon - refLon;
  while (diff > 180) { lon -= 360; diff -= 360; }
  while (diff < -180) { lon += 360; diff += 360; }
  return lon;
}
```

- When iterating through USGS earthquake features, compute `wrappedLon = wrapLon(qLon, opts.stationLon)`.
- Use `wrappedLon` for:
  - `eventBounds.extend([qLat, wrappedLon])`
  - `L.circleMarker([qLat, wrappedLon], ...)`
  - Distance calculation (or normalize `dLon` in `calculateDistance`)
  - List item pan/fly-to coordinates

### B. Dynamic World Clones for Vector Basemap & Faults
Rather than duplicating geometry data on disk (which would bloat `world-land.js` by 100%), dynamically generate a wrapped duplicate layer when the station or view is near the antimeridian (`Math.abs(opts.stationLon) > 50` or coverage radius crosses `±180°`):

1. **Shift Function:**
   A lightweight geometry transformer that deep-clones GeoJSON coordinates and shifts X by `+360` or `-360`:
   ```javascript
   function cloneAndShiftGeoJson(geojson, offsetLon) {
     // Recursively shifts coordinates[0] += offsetLon
   }
   ```
2. **Conditional Layer Addition:**
   - If `opts.stationLon > 50`: add an auxiliary layer shifted by `-360°` (covering longitudes `[-180° - 360°, 180° - 360°]` = `[-540°, -180°]`).
   - If `opts.stationLon < -50`: add an auxiliary layer shifted by `+360°` (covering longitudes `[180°, 540°]`).
   - Attach both the base and shifted layers to `basemapPane`.

### C. Quaternary Fault Coverage Extension & Antimeridian Query Splitting
1. **Extended Coverage Range (4x ~ 2,400 km / 1,500 mi):**
   - Extend fault coverage radius from ~300-500 km to `faultRadiusKm = Math.max(radiusKm * 4, 2400)` (~1,500 miles).
   - This provides comprehensive regional context across tectonic boundaries (e.g., the entire Denali, Fairweather, Queen Charlotte, and Aleutian fault arcs).
2. **Date Line Bounding Box Splitting:**
   - When `minLon < -180` or `maxLon > 180`, split the ArcGIS query into two valid geographic envelopes (`[-180, maxLon]` and `[minLon + 360, 180]`).
   - Merge the results so faults across both sides of the antimeridian are retrieved cleanly.
3. **Fault Coordinate Wrapping:**
   - Recursively wrap all fault line coordinates relative to `opts.stationLon` so they render seamlessly adjacent to the station without global wrap-around artifacts.

### D. Coverage Circle & Popup Verification
- Leaflet's `L.circle([lat, lon], radius)` projects radius in meters. When centered at `opts.stationLon`, the SVG circle renders cleanly across `-180°` / `+180°` without clipping.
- Station marker at `[opts.stationLat, opts.stationLon]` remains the anchor.

---

## 3. Verification Plan

1. **Unit / Coordinate Test:**
   - Test `wrapLon(-179, 179)` -> `181` (distance = 2°).
   - Test `wrapLon(179, -179)` -> `-181` (distance = 2°).
   - Verify `calculateDistance` produces identical results across the boundary.

2. **Visual Inspection:**
   - Test station at Adak, AK (`51.88, -176.65`) with synthetic or real Aleutian quakes at `+178.0` and `-178.0`.
   - Verify the basemap land polygons render seamlessly across the vertical line with no missing continents or clipped landmasses.
   - Verify `eventBounds` fits tightly around the cluster at zoom 7–8 rather than zooming out to 1.

3. **CI / Lint:**
   - Run `node -c skins/neowx-material/js/earthquakes.js`.
