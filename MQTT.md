# MQTT Real-Time Updates Guide

This guide explains how to set up MQTT with WeeWX to enable **real-time updates** in the NeoWX Material skin without page refresh.

## 📋 Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Option 1: Using an Existing MQTT Broker](#option-1-using-an-existing-mqtt-broker)
- [Option 2: Setting Up Mosquitto MQTT Broker](#option-2-setting-up-mosquitto-mqtt-broker)
- [Installing WeeWX MQTT Extension](#installing-weewx-mqtt-extension)
- [Configuring WeeWX](#configuring-weewx)
- [Configuring NeoWX Material Skin](#configuring-neowx-material-skin)
- [Sensor Mapping](#sensor-mapping)
- [Testing Your Setup](#testing-your-setup)
- [Troubleshooting](#troubleshooting)
- [Additional Resources](#additional-resources)

---

## Overview

MQTT (Message Queuing Telemetry Transport) enables real-time data updates in your weather station dashboard. When configured:

✅ **Live updates** appear instantly without page refresh
✅ **Visual feedback** with configurable flash effects
✅ **Connection status** indicator shows broker connectivity
✅ **Efficient** - only changed values are transmitted

### What You Need

1. **MQTT Broker** - A message broker (like Mosquitto) to relay data
2. **WeeWX MQTT Extension** - Publishes weather data to the broker
3. **NeoWX Material Configuration** - Connects to broker and displays live data

---

## Architecture

```
┌─────────────┐         ┌──────────────┐         ┌─────────────┐
│   WeeWX     │ MQTT    │    MQTT      │ WebSocket│   Browser   │
│   Station   ├────────>│   Broker     ├─────────>│  (NeoWX)    │
│             │  1883   │  (Mosquitto) │   9001   │             │
└─────────────┘         └──────────────┘          └─────────────┘
    Publishes                Relays                  Subscribes
    sensor data              messages                & displays
```

**Data Flow:**
1. WeeWX publishes sensor data to MQTT broker (port 1883)
2. Broker stores and relays messages
3. Browser connects via WebSocket (port 9001)
4. Updates appear in real-time on your dashboard

---

## Option 1: Using an Existing MQTT Broker

If you already have an MQTT broker running, skip to [Installing WeeWX MQTT Extension](#installing-weewx-mqtt-extension).

**Prerequisites:**
- MQTT broker hostname/IP address
- Port 1883 (MQTT) accessible from WeeWX server
- Port 9001 (WebSocket) accessible from client browsers
- Username and password for authentication (recommended)

---

## Option 2: Setting Up Mosquitto MQTT Broker

### Using Docker Compose (Recommended)

#### 1. Create Directory Structure

```bash
sudo mkdir -p /volume1/docker/mqtt/{config,data,log}
cd /volume1/docker/mqtt
```

#### 2. Create `docker-compose.yml`

```yaml
version: '3.8'

services:
  mosquitto:
    image: eclipse-mosquitto:latest
    container_name: mosquitto
    restart: unless-stopped
    ports:
      - "1883:1883"    # MQTT (for WeeWX)
      - "8883:8883"    # MQTTS (secure, optional)
      - "9001:9001"    # WebSockets (for browsers)
    volumes:
      - /volume1/docker/mqtt/config:/mosquitto/config
      - /volume1/docker/mqtt/data:/mosquitto/data
      - /volume1/docker/mqtt/log:/mosquitto/log
    user: "1000:1000"  # Adjust to your UID:GID
```

#### 3. Create `mosquitto.conf`

Create `/volume1/docker/mqtt/config/mosquitto.conf`:

```conf
# Basic listener configuration
listener 1883
# Allow all IPs (or specify specific IP)
# bind_address 0.0.0.0

# --- General Security ---
allow_anonymous false
password_file /mosquitto/config/password.txt
acl_file /mosquitto/config/acl.conf

# WebSocket listener (for browsers)
listener 9001
protocol websockets
allow_anonymous false

# Persistence (save messages to disk)
persistence true
persistence_location /mosquitto/data/
# Save to disk every 300 seconds (5 minutes)
autosave_interval 300

# Keep messages for offline clients for 1 day
persistent_client_expiration 1d

# Logging
log_dest file /mosquitto/log/mosquitto.log
log_type error
log_type warning
log_type notice
log_type information

# Connection timeout (optional)
# connection_messages true
# log_timestamp true
```

#### 4. Create User Accounts

Create two users: one for WeeWX (read/write), one for browsers (read-only):

```bash
# Create password file with WeeWX user
docker run -it --rm -v /volume1/docker/mqtt/config:/mosquitto/config eclipse-mosquitto mosquitto_passwd -c /mosquitto/config/password.txt weewx

# Add read-only user for browsers
docker run -it --rm -v /volume1/docker/mqtt/config:/mosquitto/config eclipse-mosquitto mosquitto_passwd /mosquitto/config/password.txt weewx-readonly
```

**Note:** You'll be prompted to enter passwords. Remember these for configuration!

#### 5. Create Access Control List (ACL)

Create `/volume1/docker/mqtt/config/acl.conf`:

```conf
# WeeWX user - can read and write to weewx topics
user weewx
topic readwrite weewx/#

# Read-only user for browsers - can only read
user weewx-readonly
topic read weewx/#
```

#### 6. Set Permissions

```bash
# Set ownership (adjust UID:GID to match docker-compose)
sudo chown -R 1000:1000 /volume1/docker/mqtt

# Set permissions
sudo chmod -R 755 /volume1/docker/mqtt
sudo chmod 600 /volume1/docker/mqtt/config/password.txt
```

#### 7. Start Mosquitto

```bash
cd /volume1/docker/mqtt
docker-compose up -d
```

#### 8. Verify Mosquitto is Running

```bash
# Check container status
docker ps | grep mosquitto

# Check logs
docker logs mosquitto

# Test connection
mosquitto_sub -h localhost -p 1883 -u weewx-readonly -P <password> -t weewx/# -v
```

---

### Alternative: Native Installation

If you prefer not to use Docker:

```bash
# Debian/Ubuntu
sudo apt-get update
sudo apt-get install mosquitto mosquitto-clients

# RHEL/CentOS
sudo yum install mosquitto mosquitto-clients

# Start and enable service
sudo systemctl start mosquitto
sudo systemctl enable mosquitto
```

Then configure `/etc/mosquitto/mosquitto.conf` with the same settings as above.

---

## Installing WeeWX MQTT Extension

The [weewx-mqtt extension](https://github.com/matthewwall/weewx-mqtt) publishes WeeWX data to your MQTT broker.

### Installation

#### WeeWX 5.x (Python 3)

```bash
weectl extension install https://github.com/matthewwall/weewx-mqtt/archive/master.zip
```

#### WeeWX 4.x (Python 2/3)

```bash
wee_extension --install https://github.com/matthewwall/weewx-mqtt/archive/master.zip
```

---

## Configuring WeeWX

After installing the extension, configure it in `weewx.conf`:

### 1. Locate the MQTT Section

Find or add the `[[MQTT]]` section under `[StdRESTful]`:

```ini
[StdRESTful]
    [[MQTT]]
        # Server connection
        server_url = mqtt://weewx:YOUR_PASSWORD@localhost:1883

        # Topic to publish to
        topic = weewx

        # Unit system for data
        unit_system = METRIC  # or US, METRICWX

        # Retain messages on broker
        retain = true

        # When to send data
        binding = archive, loop
        # archive = send on each archive interval (typically 5 min)
        # loop = send on each sensor reading (typically 2.5 sec)

        # How to aggregate data
        aggregation = aggregate

        # Logging
        log_success = false  # Set to true for debugging
        log_failure = true
```

### 2. Configuration Options Explained

| Parameter | Description | Example |
|-----------|-------------|---------|
| `server_url` | MQTT broker connection string | `mqtt://user:pass@host:1883` |
| `topic` | Base topic for publishing | `weewx` (becomes `weewx/temperature`, etc.) |
| `unit_system` | Unit system for values | `METRIC`, `US`, or `METRICWX` |
| `retain` | Broker keeps last message | `true` (recommended) |
| `binding` | When to publish data | `archive, loop` (both recommended) |
| `aggregation` | How to aggregate loop data | `aggregate` (recommended) |

### 3. Server URL Format

```
mqtt://[username]:[password]@[host]:[port]/
```

**Examples:**
- Local broker: `mqtt://weewx:password123@localhost:1883/`
- Remote broker: `mqtt://weewx:password123@mqtt.example.com:1883/`
- With special characters in password: Use URL encoding
  - `@` → `%40`
  - `#` → `%23`
  - `&` → `%26`

### 4. Restart WeeWX

```bash
sudo systemctl restart weewx
```

### 5. Verify Publishing

Monitor MQTT messages to confirm WeeWX is publishing:

```bash
# Subscribe to all weewx topics
mosquitto_sub -h localhost -p 1883 -u weewx-readonly -P <password> -t "weewx/#" -v

# You should see output like:
# weewx/outTemp_C 15.4
# weewx/outHumidity 65
# weewx/barometer_mbar 1013.25
```

---

## Configuring NeoWX Material Skin

Now configure the skin to connect to your MQTT broker and display live data.

### 1. Edit `skin.conf`

Find the `[[MQTT]]` section under `[Extras]`:

```ini
[Extras]
    [[MQTT]]
        # Enable MQTT functionality
        enabled = true

        # Debug mode (shows console logs in browser)
        debug = false

        # Flash animation on value updates
        flash_on_update = true
        flash_color = "#00ff00"  # Green (must use quotes!)

        # Paho MQTT library (leave as-is)
        paho_mqtt_url = https://cdnjs.cloudflare.com/ajax/libs/paho-mqtt/1.1.0/paho-mqtt.min.js

        # Broker connection details
        host = mqtt.example.com  # Your MQTT broker hostname (no http://)
        port = 9001              # WebSocket port
        topic = "weewx/#"        # Topic to subscribe to

        # WebSocket configuration
        use_ssl = false          # Set to true if using HTTPS
        use_websocket = true     # Required for browser connection
        websocket_path = /mqtt   # WebSocket path (default: /mqtt or /)

        # Authentication
        username = weewx-readonly
        password = readonly
```

### 2. Configuration Details

#### Basic Settings

| Parameter | Description | Default |
|-----------|-------------|---------|
| `enabled` | Enable/disable MQTT | `false` |
| `debug` | Enable browser console logging | `false` |
| `flash_on_update` | Flash effect when values change | `true` |
| `flash_color` | Hex color for flash (use quotes!) | `"#00ff00"` |

#### Connection Settings

| Parameter | Description | Example |
|-----------|-------------|---------|
| `host` | MQTT broker hostname | `mqtt.example.com` or `localhost` |
| `port` | WebSocket port | `9001` |
| `topic` | Topic pattern to subscribe to | `"weewx/#"` |
| `use_ssl` | Use secure WebSocket (wss://) | `true` if your site uses HTTPS |
| `use_websocket` | Enable WebSocket protocol | `true` (required) |
| `websocket_path` | WebSocket endpoint path | `/mqtt` or `/` |
| `username` | MQTT username | `weewx-readonly` |
| `password` | MQTT password | Your password |

#### Important Notes

⚠️ **Flash Color Format:**
- Must use quotes: `flash_color = "#00ff00"` ✅
- Without quotes fails: `flash_color = #00ff00` ❌ (# starts a comment)

⚠️ **SSL/TLS:**
- If your website uses HTTPS, set `use_ssl = true`
- Otherwise browsers will block insecure WebSocket connections

⚠️ **Host Format:**
- Use hostname/IP only (no `http://` or `mqtt://` prefix)
- Examples: `localhost`, `192.168.1.100`, `mqtt.example.com`

### 3. Reload WeeWX

```bash
sudo systemctl reload weewx
```

---

## Sensor Mapping

The skin needs to map MQTT payload attributes to WeeWX sensor names. This is configured in the `[[[SensorMapping]]]` section.

### Default Mapping

```ini
[Extras]
    [[MQTT]]
        # ...connection settings...

        [[[SensorMapping]]]
            # Format: sensor_name = "payload_attr", "unit", decimals

            # Temperature sensors
            outTemp = "outTemp_C", "°C", 1
            inTemp = "inTemp_C", "°C", 1
            dewpoint = "dewpoint_C", "°C", 1
            heatindex = "heatindex_C", "°C", 1
            windchill = "windchill_C", "°C", 1
            appTemp = "appTemp_C", "°C", 1

            # Humidity
            outHumidity = "outHumidity", "%", 0
            inHumidity = "inHumidity", "%", 0

            # Pressure
            barometer = "barometer_mbar", " hPa", 1
            pressure = "pressure_mbar", " mbar", 2
            altimeter = "altimeter_mbar", " mbar", 2

            # Wind
            windSpeed = "windSpeed_kph", " km/h", 0
            windGust = "windGust_kph", " km/h", 1

            # Rain
            rain = "dayRain_cm", " mm", 1
            rainRate = "rainRate_cm_per_hour", " mm/h", 2

            # Solar
            radiation = "radiation_Wpm2", " W/m²", 1
            UV = "UV", "", 1

            # Other
            cloudbase = "cloudbase_meter", " m", 0

            # Extra sensors (if you have them)
            extraTemp1 = "extraTemp1_C", "°C", 1
            extraTemp2 = "extraTemp2_C", "°C", 1
            extraHumid1 = "extraHumid1", "%", 0
            extraHumid2 = "extraHumid2", "%", 0
```

### Mapping Format

Each sensor mapping has three parts:

```ini
sensor_name = "mqtt_payload_attribute", "display_unit", decimal_places
```

**Example:**
```ini
outTemp = "outTemp_C", "°C", 1
```

This means:
- **`outTemp`** - WeeWX sensor name (used in the skin templates)
- **`"outTemp_C"`** - Attribute name in MQTT payload (from WeeWX MQTT extension)
- **`"°C"`** - Unit displayed in the UI
- **`1`** - Number of decimal places (15.4°C)

### Understanding MQTT Payloads

When WeeWX publishes to MQTT, it sends individual topics:

```
weewx/outTemp_C → 15.4
weewx/outHumidity → 65
weewx/barometer_mbar → 1013.25
```

The skin subscribes to `weewx/#` (all topics) and matches them to sensors using the mapping.

### Customizing Mapping

If your MQTT topics use different attribute names, adjust the mapping:

**Example:** If WeeWX publishes `temperature` instead of `outTemp_C`:

```ini
outTemp = "temperature", "°C", 1
```

**Example:** If you want to display Fahrenheit instead:

```ini
outTemp = "outTemp_F", "°F", 1
```

### Adding Custom Sensors

If you have additional sensors, add them to the mapping:

```ini
[[[SensorMapping]]]
    # Standard sensors...

    # Custom pond temperature sensor
    extraTemp1 = "pondTemp_C", "°C", 1

    # Soil moisture sensor
    soilMoist1 = "soilMoisture1", "%", 0

    # Custom wind sensor
    extraTemp2 = "windTemp_C", "°C", 1
```

---

## Testing Your Setup

### 1. Check MQTT Broker Logs

```bash
# Docker
docker logs mosquitto

# Native installation
sudo tail -f /var/log/mosquitto/mosquitto.log
```

Look for successful connections and authentication.

### 2. Monitor MQTT Traffic

```bash
# Subscribe to all weewx topics
mosquitto_sub -h localhost -p 1883 -u weewx-readonly -P <password> -t "weewx/#" -v
```

You should see continuous updates from WeeWX.

### 3. Test Browser Connection

1. Open your weather station in a browser
2. Open browser developer console (F12)
3. Look for MQTT connection logs:
   ```
   [MQTT] Connecting to: mqtt.example.com:9001
   [MQTT] ✓ Connected to MQTT broker
   [MQTT] ✓ Subscribed to: weewx/#
   ```

4. Watch for live updates:
   ```
   [MQTT] ✓ Updated outTemp to: 15.4°C
   ```

### 4. Enable Debug Mode

For troubleshooting, enable debug mode in `skin.conf`:

```ini
[[MQTT]]
    debug = true
```

This will show detailed logs in the browser console.

### 5. Verify Visual Feedback

- Values should flash (green by default) when updated
- MQTT status indicator should show green (connected) or red (disconnected)
- Updates should appear without page refresh

---

## Troubleshooting

### Connection Issues

**Problem:** Browser can't connect to MQTT broker

**Solutions:**
1. Check firewall - port 9001 must be open
2. Verify WebSocket listener in `mosquitto.conf`:
   ```conf
   listener 9001
   protocol websockets
   ```
3. Check browser console for error messages
4. If using HTTPS, ensure `use_ssl = true` in skin.conf
5. Try connecting from command line:
   ```bash
   mosquitto_sub -h <host> -p 9001 -u weewx-readonly -P <password> -t "weewx/#"
   ```

### Authentication Errors

**Problem:** "Connection refused: Not authorized"

**Solutions:**
1. Verify username/password are correct
2. Check `password.txt` file exists and is readable
3. Verify ACL permissions in `acl.conf`
4. Check Mosquitto logs for authentication attempts

### No Data Updates

**Problem:** Connected but no data appears

**Solutions:**
1. Verify WeeWX is publishing:
   ```bash
   mosquitto_sub -h localhost -p 1883 -u weewx-readonly -P <password> -t "weewx/#" -v
   ```
2. Check sensor mapping in `skin.conf`
3. Ensure topic matches: `topic = "weewx/#"` in both WeeWX and skin
4. Check browser console for JavaScript errors
5. Verify payload attribute names match your MQTT topics

### Wrong Values/Units

**Problem:** Values display incorrectly or with wrong units

**Solutions:**
1. Check sensor mapping - ensure payload attributes match
2. Verify unit_system in `weewx.conf` (METRIC vs US)
3. Adjust decimal places in mapping
4. Check for unit conversion issues

### SSL/TLS Errors

**Problem:** "Mixed content" or WebSocket security errors

**Solutions:**
1. If your site uses HTTPS, set `use_ssl = true`
2. Ensure MQTT broker has valid SSL certificate
3. Configure secure WebSocket (port 8883) if needed
4. For testing, temporarily use HTTP (not recommended for production)

### Performance Issues

**Problem:** Browser sluggish with MQTT enabled

**Solutions:**
1. Reduce update frequency - use `binding = archive` in `weewx.conf` (skip loop)
2. Reduce mapped sensors - only map sensors you display
3. Disable flash animation: `flash_on_update = false`
4. Check network latency to broker

---

## Additional Resources

### Documentation

- **WeeWX MQTT Extension**: [github.com/matthewwall/weewx-mqtt](https://github.com/matthewwall/weewx-mqtt)
- **Mosquitto Documentation**: [mosquitto.org/documentation/](https://mosquitto.org/documentation/)
- **MQTT Protocol**: [mqtt.org](https://mqtt.org)
- **Paho MQTT JavaScript**: [eclipse.org/paho/](https://www.eclipse.org/paho/)

### MQTT Tools

- **MQTT Explorer** (GUI client): [mqtt-explorer.com](http://mqtt-explorer.com/)
- **MQTT.fx** (GUI client): [mqttfx.jensd.de](https://mqttfx.jensd.de/)
- **mosquitto_sub/pub** (command-line): Included with Mosquitto

### Testing Tools

```bash
# Test MQTT publish
mosquitto_pub -h localhost -p 1883 -u weewx -P <password> -t "weewx/test" -m "hello"

# Test MQTT subscribe
mosquitto_sub -h localhost -p 1883 -u weewx-readonly -P <password> -t "weewx/#" -v

# Test WebSocket connection (requires websocat)
websocat ws://localhost:9001
```

### Security Best Practices

1. **Always use authentication** - Don't allow anonymous connections
2. **Use ACLs** - Separate read/write permissions
3. **Use SSL/TLS** - Encrypt traffic for public servers
4. **Strong passwords** - Use password manager
5. **Firewall rules** - Restrict broker access to known IPs
6. **Regular updates** - Keep Mosquitto and WeeWX updated

### Example: Complete Production Setup

**docker-compose.yml** with SSL:
```yaml
version: '3.8'
services:
  mosquitto:
    image: eclipse-mosquitto:latest
    container_name: mosquitto
    restart: unless-stopped
    ports:
      - "1883:1883"    # MQTT (internal only)
      - "8883:8883"    # MQTTS (SSL)
      - "9001:9001"    # WebSockets (SSL)
    volumes:
      - ./config:/mosquitto/config
      - ./data:/mosquitto/data
      - ./log:/mosquitto/log
      - ./certs:/mosquitto/certs
    networks:
      - mqtt_network

networks:
  mqtt_network:
    driver: bridge
```

**mosquitto.conf** with SSL:
```conf
# MQTT with SSL
listener 8883
protocol mqtt
cafile /mosquitto/certs/ca.crt
certfile /mosquitto/certs/server.crt
keyfile /mosquitto/certs/server.key
require_certificate false

# WebSocket with SSL
listener 9001
protocol websockets
cafile /mosquitto/certs/ca.crt
certfile /mosquitto/certs/server.crt
keyfile /mosquitto/certs/server.key

# Security
allow_anonymous false
password_file /mosquitto/config/password.txt
acl_file /mosquitto/config/acl.conf

# Persistence & logging...
```

---

## Summary

You now have a complete MQTT setup:

✅ **MQTT broker** (Mosquitto) running and secured
✅ **WeeWX publishing** sensor data to broker
✅ **NeoWX Material** displaying live updates
✅ **Sensor mapping** configured correctly

Your weather dashboard now updates in real-time! 🎉

For support, open an issue at [github.com/seehase/neowx-material](https://github.com/seehase/neowx-material/issues).

---

**Made with ❤️ for the WeeWX community**

