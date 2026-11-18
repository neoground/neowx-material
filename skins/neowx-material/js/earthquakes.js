(function () {
  'use strict';

  var cachedData = null;
  var cachedFaultsData = null;
  var cachedOptions = null;

  var themeColors = {
    'red': '#f44336', 'pink': '#e91e63', 'purple': '#9c27b0', 'deep-purple': '#673ab7',
    'indigo': '#3f51b5', 'blue': '#2196f3', 'light-blue': '#03a9f4', 'cyan': '#00bcd4',
    'teal': '#009688', 'green': '#4caf50', 'light-green': '#8bc34a', 'lime': '#cddc39',
    'yellow': '#ffeb3b', 'amber': '#ffc107', 'orange': '#ff9800', 'deep-orange': '#ff5722',
    'brown': '#795548', 'grey': '#9e9e9e', 'blue-grey': '#607d8b'
  };

  function getMagColor(mag) {
    if (mag >= 5.0) return '#d32f2f'; // red
    if (mag >= 4.0) return '#f57c00'; // dark orange
    if (mag >= 3.0) return '#ffa000'; // amber
    return '#388e3c';                // green
  }

  function wrapLon(lon, refLon) {
    var diff = lon - refLon;
    while (diff > 180) { lon -= 360; diff -= 360; }
    while (diff < -180) { lon += 360; diff += 360; }
    return lon;
  }

  function calculateDistance(lat1, lon1, lat2, lon2) {
    var R = 6371; // km
    var dLat = (lat2 - lat1) * Math.PI / 180;
    var wLon2 = wrapLon(lon2, lon1);
    var dLon = (wLon2 - lon1) * Math.PI / 180;
    var a = Math.sin(dLat / 2) * Math.sin(dLat / 2) +
      Math.cos(lat1 * Math.PI / 180) * Math.cos(lat2 * Math.PI / 180) *
      Math.sin(dLon / 2) * Math.sin(dLon / 2);
    var c = 2 * Math.atan2(Math.sqrt(a), Math.sqrt(1 - a));
    return R * c;
  }

  function cloneAndShiftGeoJson(geojson, offsetLon) {
    if (!geojson || !geojson.features) return null;
    function shiftCoords(coords) {
      if (typeof coords[0] === 'number') {
        return [coords[0] + offsetLon, coords[1]];
      }
      return coords.map(shiftCoords);
    }
    return {
      type: 'FeatureCollection',
      name: geojson.name,
      crs: geojson.crs,
      features: geojson.features.map(function (f) {
        return {
          type: 'Feature',
          properties: f.properties,
          geometry: {
            type: f.geometry.type,
            coordinates: shiftCoords(f.geometry.coordinates)
          }
        };
      })
    };
  }

  function wrapGeoJsonCoords(coords, refLon) {
    if (typeof coords[0] === 'number') {
      return [wrapLon(coords[0], refLon), coords[1]];
    }
    return coords.map(function (c) {
      return wrapGeoJsonCoords(c, refLon);
    });
  }

  function setupBasemap(map, opts) {
    if (opts.basemap === 'vector') {
      if (window.NEOWX_WORLD_LAND) {
        if (!map.getPane('basemapPane')) {
          map.createPane('basemapPane');
          map.getPane('basemapPane').style.zIndex = 200;
        }
        var styleFn = function () {
          return {
            fillColor: opts.isDark ? '#2e3951' : '#e0e4eb',
            fillOpacity: 1,
            color: opts.isDark ? '#3f4f6e' : '#b0b8c5',
            weight: 1.2
          };
        };
        L.geoJSON(window.NEOWX_WORLD_LAND, {
          pane: 'basemapPane',
          attribution: '&copy; <a href="https://www.naturalearthdata.com/" target="_blank" rel="noopener">Natural Earth</a>',
          style: styleFn
        }).addTo(map);

        // Seamless antimeridian wrap: replicate world land across the date line
        if (opts.stationLon < 0) {
          var shiftedWest = cloneAndShiftGeoJson(window.NEOWX_WORLD_LAND, -360);
          if (shiftedWest) {
            L.geoJSON(shiftedWest, { pane: 'basemapPane', style: styleFn }).addTo(map);
          }
        }
        if (opts.stationLon >= 0 || opts.stationLon < -100) {
          var shiftedEast = cloneAndShiftGeoJson(window.NEOWX_WORLD_LAND, 360);
          if (shiftedEast) {
            L.geoJSON(shiftedEast, { pane: 'basemapPane', style: styleFn }).addTo(map);
          }
        }
      }
    } else {
      var tileUrl = opts.customTileUrl || 'https://tile.openstreetmap.org/{z}/{x}/{y}.png';
      L.tileLayer(tileUrl, {
        attribution: '&copy; <a href="https://www.openstreetmap.org/copyright" target="_blank" rel="noopener">OpenStreetMap</a> contributors',
        maxZoom: 18
      }).addTo(map);
    }
  }

  function addStationMarker(map, opts) {
    var stationIcon = L.divIcon({
      className: 'station-pin-marker',
      html: '<div style="background-color:' + opts.primaryHex + ';color:#fff;border-radius:50%;width:24px;height:24px;display:flex;align-items:center;justify-content:center;box-shadow:0 2px 5px rgba(0,0,0,0.4);border:2px solid #fff;"><i class="fas fa-broadcast-tower" style="font-size:11px;"></i></div>',
      iconSize: [24, 24],
      iconAnchor: [12, 12]
    });

    return L.marker([opts.stationLat, opts.stationLon], { icon: stationIcon })
      .addTo(map)
      .bindPopup('<strong>' + opts.stationName + '</strong><br>Lat: ' + opts.stationLat.toFixed(4) + ', Lon: ' + opts.stationLon.toFixed(4));
  }

  function addCoverageCircle(map, opts) {
    return L.circle([opts.stationLat, opts.stationLon], {
      radius: opts.radiusKm * 1000,
      color: opts.primaryHex,
      weight: 1.5,
      dashArray: '4, 4',
      fillColor: opts.primaryHex,
      fillOpacity: 0.05
    }).addTo(map);
  }

  function renderFaultsOnMap(map, geoData, opts) {
    if (!geoData || !geoData.features || geoData.features.length === 0) return;

    if (!map.getPane('faultsPane')) {
      map.createPane('faultsPane');
      map.getPane('faultsPane').style.zIndex = 350;
    }

    var faultColor = opts.isDark ? '#ff7043' : '#d84315';

    var normalizedFeatures = geoData.features.map(function (f) {
      if (!f.geometry || !f.geometry.coordinates) return f;
      return {
        type: 'Feature',
        properties: f.properties,
        geometry: {
          type: f.geometry.type,
          coordinates: wrapGeoJsonCoords(f.geometry.coordinates, opts.stationLon)
        }
      };
    });

    var faultsLayer = L.geoJSON({ type: 'FeatureCollection', features: normalizedFeatures }, {
      pane: 'faultsPane',
      style: function (feature) {
        var linetype = (feature.properties && feature.properties.linetype) || '';
        var isInferred = linetype.toLowerCase().indexOf('inferred') !== -1;
        return {
          color: faultColor,
          weight: 1.8,
          opacity: 0.8,
          dashArray: isInferred ? '5, 5' : null
        };
      },
      onEachFeature: function (feature, layer) {
        var props = feature.properties || {};
        var faultName = props.fault_name || 'Unnamed Fault';
        var sectionName = props.section_name || '';
        var age = props.age || '';
        var slipRate = props.slip_rate || '';
        var slipSense = props.slip_sense || '';
        var linetype = props.linetype || '';

        var popupContent = '<div class="font-small">' +
          '<strong><i class="fas fa-wave-square mr-1"></i>' + faultName + '</strong>' +
          (sectionName ? '<br><span class="text-muted">Section:</span> ' + sectionName : '') +
          (age ? '<br><span class="text-muted">Age:</span> ' + age : '') +
          (slipRate ? '<br><span class="text-muted">Slip Rate:</span> ' + slipRate : '') +
          (slipSense ? '<br><span class="text-muted">Sense:</span> ' + slipSense : '') +
          (linetype ? '<br><span class="text-muted">Mapping:</span> ' + linetype : '') +
          '<div class="text-muted font-italic mt-1" style="font-size:0.75rem;">USGS Quaternary Faults</div>' +
          '</div>';

        layer.bindPopup(popupContent);

        layer.on('mouseover', function () {
          this.setStyle({
            weight: 3.5,
            opacity: 1.0
          });
        });
        layer.on('mouseout', function () {
          faultsLayer.resetStyle(this);
        });
      }
    });

    faultsLayer.addTo(map);

    var overlays = {};
    var faultLabel = '<span style="font-size:0.85rem;"><span style="display:inline-block;width:12px;height:2px;background:' + faultColor + ';vertical-align:middle;margin-right:4px;"></span> Fault Lines</span>';
    overlays[faultLabel] = faultsLayer;

    L.control.layers(null, overlays, {
      position: 'topright',
      collapsed: true
    }).addTo(map);
  }

  function populateMapAndList(map, features, opts, listEl, countEl, maxMagEl, latestEl) {
    if (countEl) countEl.innerText = features.length;

    var maxMag = 0;
    var latestTime = null;
    var markersMap = {};
    var isEmbed = (map.getContainer().id === 'earthquake-map');

    if (features.length === 0) {
      if (listEl) {
        listEl.innerHTML = '<div class="p-3 text-muted font-small text-center">No earthquakes &ge; M' + opts.minMag + ' recorded within ' + opts.radiusKm + ' km in the past ' + opts.timespanDays + ' days.</div>';
      }
      if (maxMagEl) maxMagEl.innerText = '-';
      if (latestEl) latestEl.innerText = '-';
      return;
    }

    var eventBounds = L.latLngBounds([[opts.stationLat, opts.stationLon]]);

    var html = '<div class="list-group list-group-flush">';

    features.forEach(function (feat, idx) {
      var props = feat.properties;
      var coords = feat.geometry.coordinates; // [lon, lat, depth]
      var rawLon = coords[0];
      var qLat = coords[1];
      var qDepth = coords[2];
      var qLon = wrapLon(rawLon, opts.stationLon);
      var mag = props.mag !== null ? props.mag.toFixed(1) : '?';
      var magNum = props.mag || 0;
      if (magNum > maxMag) maxMag = magNum;
      if (!latestTime || props.time > latestTime) latestTime = props.time;

      eventBounds.extend([qLat, qLon]);

      var distKm = calculateDistance(opts.stationLat, opts.stationLon, qLat, qLon);
      var distMiles = distKm * 0.621371;
      var color = getMagColor(magNum);
      var radius = isEmbed
        ? Math.max(3.5, Math.min(12, (magNum * 2.4)))
        : Math.max(5, Math.min(18, (magNum * 3.2)));
      var dateStr = new Date(props.time).toLocaleString();

      var marker = L.circleMarker([qLat, qLon], {
        radius: radius,
        fillColor: color,
        color: '#ffffff',
        weight: isEmbed ? 1.0 : 1.5,
        opacity: 1,
        fillOpacity: 0.75
      }).addTo(map);

      var popupContent = '<div class="font-small"><strong>M ' + mag + ' - ' + props.place + '</strong><br>' +
        'Depth: ' + qDepth.toFixed(1) + ' km<br>' +
        'Distance: ' + distMiles.toFixed(1) + ' mi (' + distKm.toFixed(1) + ' km)<br>' +
        'Time: ' + dateStr + '<br>' +
        '<a href="' + props.url + '" target="_blank" rel="noopener" class="font-weight-bold mt-1 d-inline-block">View on USGS &rarr;</a></div>';

      marker.bindPopup(popupContent);
      markersMap[props.url || idx] = marker;

      html += '<a href="javascript:void(0)" class="list-group-item list-group-item-action p-2 quake-item" data-id="' + (props.url || idx) + '" data-lat="' + qLat + '" data-lon="' + qLon + '">' +
        '<div class="d-flex w-100 justify-content-between align-items-center">' +
        '<span class="badge badge-pill text-white mr-2" style="background-color:' + color + ';font-size:0.9rem;min-width:44px;">M ' + mag + '</span>' +
        '<div class="flex-grow-1 text-truncate font-small">' +
        '<div class="text-truncate font-weight-bold">' + props.place + '</div>' +
        '<div class="text-muted font-small">' + distMiles.toFixed(1) + ' mi away &bull; depth ' + qDepth.toFixed(1) + ' km</div>' +
        '</div>' +
        '<span class="text-muted font-small ml-2 text-nowrap">' + new Date(props.time).toLocaleDateString([], { month: 'short', day: 'numeric' }) + '</span>' +
        '</div>' +
        '</a>';
    });

    html += '</div>';

    if (listEl) {
      listEl.innerHTML = html;

      listEl.querySelectorAll('.quake-item').forEach(function (itemEl) {
        itemEl.addEventListener('click', function () {
          var id = this.dataset.id;
          var lat = parseFloat(this.dataset.lat);
          var lon = parseFloat(this.dataset.lon);

          map.closePopup();
          map.off('moveend');

          var currentZoom = map.getZoom();
          if (currentZoom >= 8) {
            map.panTo([lat, lon], { animate: true, duration: 0.45 });
          } else {
            map.flyTo([lat, lon], 8, { duration: 0.7, easeLinearity: 0.25 });
          }

          map.once('moveend', function () {
            if (markersMap[id]) {
              markersMap[id].openPopup();
            }
          });
        });
      });
    }

    if (maxMagEl) maxMagEl.innerText = 'M ' + maxMag.toFixed(1);
    if (latestEl && latestTime) {
      latestEl.innerText = new Date(latestTime).toLocaleDateString([], { month: 'short', day: 'numeric' });
    }

    try {
      if (features.length > 0) {
        map.fitBounds(eventBounds, { padding: isEmbed ? [16, 16] : [28, 28], maxZoom: 8.5 });
      }
    } catch (e) {}
  }

  function openEarthquakePopup(button) {
    if (!cachedOptions) return;
    var card = button.closest ? button.closest('.card') : null;
    var cardHeading = card && card.querySelector('h5');
    var title = (cardHeading && cardHeading.innerText.trim()) || 'Regional Earthquakes';

    if (typeof neowxOpenPopup !== 'function') {
      console.error('neowx-material: neowxOpenPopup is not available');
      return;
    }

    var popupMap = null;
    var modal = document.getElementById('neowx-popup');
    if (modal) {
      modal.classList.add('nwm-popup-earthquakes');
    }

    neowxOpenPopup(title, function (mount) {
      mount.innerHTML = [
        '<div class="row nwm-earthquake-modal-row h-100">',
        '  <div class="col-12 col-lg-7 mb-3 mb-lg-0 d-flex flex-column nwm-eq-map-col">',
        '    <div id="earthquake-popup-map" class="earthquake-popup-map flex-grow-1"></div>',
        '  </div>',
        '  <div class="col-12 col-lg-5 d-flex flex-column nwm-eq-list-col">',
        '    <div class="card mb-3 flex-shrink-0">',
        '      <div class="card-body p-3 text-center">',
        '        <div class="row">',
        '          <div class="col-4 border-right">',
        '            <div class="text-muted font-small text-uppercase">Events (' + cachedOptions.timespanDays + ' d)</div>',
        '            <h4 class="h3-responsive font-weight-bold mb-0 text-' + cachedOptions.themeColorName + '" id="quake-popup-total-count">-</h4>',
        '          </div>',
        '          <div class="col-4 border-right">',
        '            <div class="text-muted font-small text-uppercase">Max Mag</div>',
        '            <h4 class="h3-responsive font-weight-bold mb-0 text-danger" id="quake-popup-max-mag">-</h4>',
        '          </div>',
        '          <div class="col-4">',
        '            <div class="text-muted font-small text-uppercase">Latest</div>',
        '            <h4 class="h3-responsive font-weight-bold mb-0 text-' + cachedOptions.themeColorName + '" id="quake-popup-latest-time">-</h4>',
        '          </div>',
        '        </div>',
        '      </div>',
        '    </div>',
        '    <div class="card flex-grow-1 d-flex flex-column overflow-hidden nwm-eq-list-card">',
        '      <div class="card-header py-2 px-3 d-flex justify-content-between align-items-center flex-shrink-0">',
        '        <span class="font-weight-bold text-' + cachedOptions.themeColorName + '">',
        '          <i class="mdi mdi-format-list-bulleted mr-1"></i> Earthquakes (&ge; M' + cachedOptions.minMag + ')',
        '        </span>',
        '        <span class="badge badge-light font-small">' + cachedOptions.radiusKm + ' km radius</span>',
        '      </div>',
        '      <div class="card-body p-0 earthquake-popup-scroll flex-grow-1" id="earthquake-popup-list">',
        '        <div class="p-3 text-center text-muted font-small">Loading events...</div>',
        '      </div>',
        '    </div>',
        '  </div>',
        '</div>'
      ].join('');

      popupMap = L.map('earthquake-popup-map', {
        center: [cachedOptions.stationLat, cachedOptions.stationLon],
        zoom: 7,
        minZoom: 3,
        maxZoom: 14,
        zoomSnap: 0.5,
        scrollWheelZoom: true
      });

      popupMap.attributionControl.addAttribution('Data: <a href="https://earthquake.usgs.gov/" target="_blank" rel="noopener">USGS</a>');

      setupBasemap(popupMap, cachedOptions);
      addStationMarker(popupMap, cachedOptions);
      var circle = addCoverageCircle(popupMap, cachedOptions);
      popupMap.fitBounds(circle.getBounds(), { padding: [15, 15] });

      if (cachedOptions.showFaults && cachedFaultsData) {
        renderFaultsOnMap(popupMap, cachedFaultsData, cachedOptions);
      }

      var listEl = document.getElementById('earthquake-popup-list');
      var countEl = document.getElementById('quake-popup-total-count');
      var maxMagEl = document.getElementById('quake-popup-max-mag');
      var latestEl = document.getElementById('quake-popup-latest-time');

      var features = (cachedData && cachedData.features) ? cachedData.features : [];
      populateMapAndList(popupMap, features, cachedOptions, listEl, countEl, maxMagEl, latestEl);

      setTimeout(function () {
        popupMap.invalidateSize();
      }, 150);
    }, function () {
      if (modal) {
        modal.classList.remove('nwm-popup-earthquakes');
      }
      if (popupMap) {
        popupMap.remove();
        popupMap = null;
      }
    }, false, card);
  }

  function initEarthquakes() {
    var mapContainer = document.getElementById('earthquake-map');
    if (!mapContainer || typeof L === 'undefined') return;

    var stationLat = parseFloat(mapContainer.dataset.lat) || 0;
    var stationLon = parseFloat(mapContainer.dataset.lon) || 0;
    var radiusKm = parseFloat(mapContainer.dataset.radius) || 300;
    var minMag = parseFloat(mapContainer.dataset.minmag) || 2.5;
    var timespanDays = parseInt(mapContainer.dataset.timespan, 10) || 7;
    var basemap = (mapContainer.dataset.basemap || 'vector').toLowerCase();
    var customTileUrl = mapContainer.dataset.tileUrl || '';
    var showFaults = (mapContainer.dataset.showFaults || 'yes').toLowerCase() !== 'no';
    var stationName = mapContainer.dataset.stationName || 'Weather Station';

    var configMode = (mapContainer.dataset.themeMode || 'auto').toLowerCase();
    var isDark = false;
    if (configMode === 'dark' || document.body.classList.contains('dark-theme')) {
      isDark = true;
    } else if (configMode === 'light' || document.body.classList.contains('light-theme')) {
      isDark = false;
    } else {
      isDark = window.matchMedia && window.matchMedia('(prefers-color-scheme: dark)').matches;
    }

    var themeColorName = (mapContainer.dataset.themeColor || 'indigo').toLowerCase();
    var primaryHex = themeColors[themeColorName] || '#3f51b5';

    cachedOptions = {
      stationLat: stationLat,
      stationLon: stationLon,
      radiusKm: radiusKm,
      minMag: minMag,
      timespanDays: timespanDays,
      basemap: basemap,
      customTileUrl: customTileUrl,
      showFaults: showFaults,
      stationName: stationName,
      isDark: isDark,
      themeColorName: themeColorName,
      primaryHex: primaryHex
    };

    var map = L.map('earthquake-map', {
      center: [stationLat, stationLon],
      zoom: 7,
      minZoom: 3,
      maxZoom: 14,
      zoomSnap: 0.5,
      scrollWheelZoom: false
    });

    map.attributionControl.addAttribution('Data: <a href="https://earthquake.usgs.gov/" target="_blank" rel="noopener">USGS</a>');

    setupBasemap(map, cachedOptions);
    addStationMarker(map, cachedOptions);
    var coverageCircle = addCoverageCircle(map, cachedOptions);
    map.fitBounds(coverageCircle.getBounds(), { padding: [15, 15] });

    setTimeout(function () {
      map.invalidateSize();
    }, 150);

    window.addEventListener('resize', function () {
      map.invalidateSize();
    });
    if (typeof jQuery !== 'undefined') {
      jQuery('.nwm-expansion-panel').on('shown.bs.collapse', function () {
        setTimeout(function () {
          map.invalidateSize();
        }, 100);
      });
    }

    if (showFaults) {
      loadFaultLines();
    }

    function loadFaultLines() {
      var faultRadiusKm = Math.max(radiusKm * 4, 2400); // 4x extension (~1500 mi)
      var latDelta = (faultRadiusKm / 111.0);
      var cosLat = Math.cos(stationLat * Math.PI / 180);
      var lonDelta = (faultRadiusKm / (111.0 * Math.max(0.1, Math.abs(cosLat))));

      var minLon = stationLon - lonDelta;
      var maxLon = stationLon + lonDelta;
      var minLat = Math.max(-90, stationLat - latDelta);
      var maxLat = Math.min(90, stationLat + latDelta);

      var boxes = [];
      if (minLon < -180) {
        boxes.push([-180.0, minLat, maxLon, maxLat]);
        boxes.push([minLon + 360.0, minLat, 180.0, maxLat]);
      } else if (maxLon > 180) {
        boxes.push([minLon, minLat, 180.0, maxLat]);
        boxes.push([-180.0, minLat, maxLon - 360.0, maxLat]);
      } else {
        boxes.push([minLon, minLat, maxLon, maxLat]);
      }

      var faultsCacheKey = 'neowx_faults_cache_v2_' + stationLat.toFixed(2) + '_' + stationLon.toFixed(2) + '_' + faultRadiusKm;
      var cachedFaults = null;
      try {
        var item = localStorage.getItem(faultsCacheKey);
        if (item) {
          var parsed = JSON.parse(item);
          if (Date.now() - parsed.timestamp < 7 * 86400 * 1000) {
            cachedFaults = parsed.data;
          }
        }
      } catch (e) {}

      if (cachedFaults) {
        cachedFaultsData = cachedFaults;
        renderFaultsOnMap(map, cachedFaults, cachedOptions);
      } else {
        function fetchFaultBox(b) {
          var url = 'https://earthquake.usgs.gov/arcgis/rest/services/haz/Qfaults/MapServer/21/query?' +
            'where=1%3D1' +
            '&geometry=' + b[0].toFixed(4) + ',' + b[1].toFixed(4) + ',' + b[2].toFixed(4) + ',' + b[3].toFixed(4) +
            '&geometryType=esriGeometryEnvelope' +
            '&spatialRel=esriSpatialRelIntersects' +
            '&inSR=4326&outSR=4326&f=geojson' +
            '&outFields=fault_name,section_name,age,slip_rate,slip_sense,linetype' +
            '&maxAllowableOffset=0.005';
          return fetch(url).then(function (res) {
            if (!res.ok) throw new Error('HTTP ' + res.status);
            return res.json();
          });
        }

        Promise.all(boxes.map(fetchFaultBox))
          .then(function (results) {
            var combinedFeatures = [];
            results.forEach(function (res) {
              if (res && res.features) {
                combinedFeatures = combinedFeatures.concat(res.features);
              }
            });
            if (combinedFeatures.length > 0) {
              var data = { type: 'FeatureCollection', features: combinedFeatures };
              try {
                localStorage.setItem(faultsCacheKey, JSON.stringify({
                  timestamp: Date.now(),
                  data: data
                }));
              } catch (e) {}
              cachedFaultsData = data;
              renderFaultsOnMap(map, data, cachedOptions);
            }
          })
          .catch(function () {});
      }
    }

    var startTime = new Date(Date.now() - timespanDays * 86400 * 1000).toISOString();
    var apiUrl = 'https://earthquake.usgs.gov/fdsnws/event/1/query?format=geojson' +
      '&latitude=' + stationLat +
      '&longitude=' + stationLon +
      '&maxradiuskm=' + radiusKm +
      '&minmagnitude=' + minMag +
      '&starttime=' + startTime +
      '&orderby=time';

    var cacheKey = 'neowx_earthquakes_cache_' + stationLat + '_' + stationLon + '_' + radiusKm + '_' + minMag;
    var cached = null;
    try {
      var item = localStorage.getItem(cacheKey);
      if (item) {
        var parsed = JSON.parse(item);
        if (Date.now() - parsed.timestamp < 15 * 60 * 1000) {
          cached = parsed.data;
        }
      }
    } catch (e) {}

    if (cached) {
      cachedData = cached;
      renderData(cached);
    } else {
      fetch(apiUrl)
        .then(function (res) {
          if (!res.ok) throw new Error('HTTP ' + res.status);
          return res.json();
        })
        .then(function (data) {
          try {
            localStorage.setItem(cacheKey, JSON.stringify({
              timestamp: Date.now(),
              data: data
            }));
          } catch (e) {}
          cachedData = data;
          renderData(data);
        })
        .catch(function (err) {
          var listEl = document.getElementById('earthquake-list');
          if (listEl) {
            listEl.innerHTML = '<div class="alert alert-warning font-small m-3">Could not load earthquake data: ' + err.message + '</div>';
          }
        });
    }

    function renderData(data) {
      var features = (data && data.features) ? data.features : [];
      var listEl = document.getElementById('earthquake-list');
      var countEl = document.getElementById('quake-total-count');
      var maxMagEl = document.getElementById('quake-max-mag');
      var latestEl = document.getElementById('quake-latest-time');

      populateMapAndList(map, features, cachedOptions, listEl, countEl, maxMagEl, latestEl);
    }
  }

  document.addEventListener('click', function (e) {
    var btn = e.target.closest ? e.target.closest('[data-nwm-earthquake-popup]') : null;
    if (btn) {
      e.preventDefault();
      openEarthquakePopup(btn);
    }
  });

  if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initEarthquakes);
  } else {
    initEarthquakes();
  }
})();
