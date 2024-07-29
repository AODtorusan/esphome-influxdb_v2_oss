# Full Configuration Example

The [full.yml](examples/full.yml) example demonstrates all features of
the component. It is based on a real-world configuration running on an
AirGradient One V9 air-quality sensing device, although the structure
of the data sent to InfluxDB in this example is not optimal (it was
designed to demonstrate the component's capabilities).

```yaml
esphome:
  name: influxdb-full
  friendly_name: Full InfluxDB Example

esp32:
  board: esp32-c3-devkitm-1
  framework:
    type: esp-idf

logger:
  hardware_uart: USB_SERIAL_JTAG

wifi:
  networks:
    - ssid: example-network
      password: network-password

api:
```

This section fulfills basic ESPHome requirements: node information,
board selection, logger, and WiFi/API connectivity.

```yaml
external_components:
  - source: github://kpfleming/esphome-influxdb_v2_oss
```

This configuration requires one external component,
esphome-influxdb_v2_oss.

```yaml
time:
  - id: _time
    platform: homeassistant
    timezone: EST5EDT,M3.2.0,M11.1.0
```

This section configures time synchronization with a Home Assistant
installation (time sync will be achieved once Home Assistant connects
to the ESPHome API on this device).

```yaml
uart:
  - id: senseair_s8_uart
    rx_pin: GPIO0
    tx_pin: GPIO1
    baud_rate: 9600

  - id: pms5003_uart
    rx_pin: GPIO20
    tx_pin: GPIO21
    baud_rate: 9600

i2c:
  sda: GPIO7
  scl: GPIO6
  frequency: 400kHz
```

This section configures hardware busses used by the sensing devices.

```yaml
binary_sensor:
  - id: _wifi_connected
    platform: template
    lambda: |-
      return id(_wifi).is_connected();
```

This section creates a binary sensor indicating whether or not the
WiFi interface is currently connected to an Access Point.

```yaml
sensor:
  - id: _uptime
    platform: uptime
    name: ${node_name} Uptime
    on_value:
      then:
        - if:
            condition:
              time.has_time:
            then:
              - influxdb.publish: _device_info

  - id: _wifi_rssi
    platform: wifi_signal
    name: ${node_name} WiFi Signal
```

This section configures two sensors, device uptime and the WiFi RSSI
(signal strength).

In addition, the configuration triggers publication to InfluxDB each
time the 'uptime' sensor has a new value (once per second), but only
if the 'time' component has achieved time synchronization. As noted
below, this is required for the InfluxDB component to be able to
generate timestamps, and also for the ability to queue measurements
for later publication if connectivity to the InfluxDB server is
interrupted.

```yaml
  - platform: pmsx003
    type: PMSX003
    uart_id: pms5003_uart
    update_interval: 2min
    pm_0_3um:
      id: pm_0_3um
      name: ${node_friendly_name} PM 0.3
      filters:
        - sliding_window_moving_average:
            window_size: 30
            send_every: 30
    pm_1_0:
      id: pm_1_0
      name: ${node_friendly_name} PM 1.0
      device_class: pm1
      filters:
        - sliding_window_moving_average:
            window_size: 30
            send_every: 30
    pm_2_5:
      id: pm_2_5
      name: ${node_friendly_name} PM 2.5
      device_class: pm25
      filters:
        - sliding_window_moving_average:
            window_size: 30
            send_every: 30
    pm_10_0:
      id: pm_10_0
      name: ${node_friendly_name} PM 10.0
      device_class: pm10
      filters:
        - sliding_window_moving_average:
            window_size: 30
            send_every: 30

  - platform: senseair
    uart_id: senseair_s8_uart
    update_interval: 2min
    co2:
      id: co2
      name: ${node_friendly_name} CO2
      filters:
        - skip_initial: 2
        - clamp:
            min_value: 400
        - sliding_window_moving_average:
            window_size: 30
            send_every: 30

  - platform: sht4x
    address: 0x44
    update_interval: 2min
    temperature:
      id: temperature_c
      name: ${node_friendly_name} Temperature
      filters:
        - sliding_window_moving_average:
            window_size: 30
            send_every: 30

  - id: temperature_f
    platform: copy
    source_id: temperature_c
    unit_of_measurement: "°F"
    filters:
      - multiply: 1.8
      - offset: 32.0
```

This section configures many of the sensing devices available in the
AirGradient One unit, and also configures a `copy` sensor to transform
the temperate reading from Celsius to Fahrenheit.

```yaml
text_sensor:
  - platform: wifi_info
    ip_address:
      id: _wifi_ip_address

```

This section configures a text sensor which reports the current IPv4
address of the unit.

```yaml
http_request:
  useragent: esphome/influxdb
  timeout: 15s
  watchdog_timeout: 15s
```

This section configures the `http_request` component, which is used by
the InfluxDB component to publish measurements to the server.

```yaml
influxdb_v2_oss:
  url: http://influxdb.example.com:8086
  organization: example
  token: influxdb-token
  time_id: _time
  backlog_max_depth: 60
  backlog_drain_batch: 10
  tags:
    device: ${node_name}
  measurements:
```

This section configures the InfluxDB component itself, but not the
measurements to be published (see below for those).

First, the basic details required to connect to the InfluxDB server:
URL, organization, and token.

Next, the ID of a 'time' component which the component can use to
generate timestamps. It is not necessary to specify `time_id` as the
presence of a `time` component will be detected automatically, but it
is available if an unusual configuration requires it.

The `backlog_max_depth` and `backlog_drain_batch` items control the
maximum number of measurements which can be stored in memory while
waiting for the InfluxDB server to be reachable, and the maximum
number of queued measurements which can be submitted in a single
batch. It is important to keep that number relatively low so that the
ESPHome device won't appear to 'lock up' when draining a large number
of queued measurements when the InfluxDB server becomes reachable
after a period of unreachability.

The 'tags' section configures tags (keys and values) which will be
added to all measurements published by the component. In this case a
tag named `device` will be sent containing the ESPHome `node_name`.

```yaml
    - id: _device_info
      bucket: iot_devices
      name: info
      sensors:
        - sensor_id: _uptime
          name: uptime
          format: integer
        - sensor_id: _wifi_rssi
          name: rssi
      binary_sensors:
        - sensor_id: _wifi_connected
          name: connected
      text_sensors:
        - sensor_id: _wifi_ip_address
          name: ip_address
```

This section configures the first measurement. It has an ID which can
be supplied to the `influxdb.publish` action to trigger publication, a
bucket name to receive the measurement, and a name ('info'). The
measurement will contain values from four sensors (specified by their
IDs), with names supplied to override the default names ('uptime'
instead of '_uptime' and 'rssi' instead of '_wifi_rssi', etc.)

One of the sensors is also published as an integer instead of the
default floating-point format, sine the unit's uptime is always a
whole number of seconds.

It may be useful to note that if the WiFi network is not connected,
the value of `_wifi_connected` will be `false`, but publishing to the
InfluxDB server will not be possible. Because this configuration
includes a `time` component and the InfluxDB backlog is enabled, the
measurements containing a `false` value will be queued in the backlog
until the server becomes reachable again.

```yaml
    - id: _pm
      bucket: air_quality
      name: particulate_matter
      sensors:
        - sensor_id: pm_0_3um
          raw_state: true
        - sensor_id: pm_1_0
          raw_state: true
        - sensor_id: pm_2_5
          raw_state: true
        - sensor_id: pm_10_0
          raw_state: true
```

This section configures another measurement, and demonstrates how the
component can publish the 'raw' state of a numeric (or text)
sensor. In this case the referenced sensors have
`sliding_window_moving_average` filters which are used to smooth out
the data reported to Home Assistant, but the data reported to InfluxDB
will be the raw data.

```yaml
    - id: _co2
      bucket: air_quality
      name: carbon_dioxide
      sensors:
        - sensor_id: co2
          format: unsigned_integer
```

This section configures another measurement, and demonstrates how a
numeric sensor (which has a floating-point value in ESPHome) can be
published as an 'unsigned integer' to InfluxDB.

```yaml
    - id: _temperature_c
      bucket: air_quality
      name: temperature
      tags:
        scale: C
      sensors:
        - sensor_id: temperature_c
          name: temperature
          accuracy_decimals: 1

    - id: _temperature_f
      bucket: air_quality
      name: temperature
      tags:
        scale: F
      sensors:
        - sensor_id: temperature_f
          name: temperature
          accuracy_decimals: 1
```

This section configures two measurements, which have different IDs but
the same `name`, so they will be combined by InfluxDB into a single
measurement when their timestamps match. Each measurement has one
field, and again they have the same name, but since there are
measurement-level tags applied these fields become a pair that can be
queried based on their tag values depending on whether Celsius or
Fahrenheit values are desired in the query result.

This section also demonstrates how the floating-point values can be
rounded to a specified number of decimal places.

```yaml
interval:
  - interval: 30s
    then:
      - influxdb.publish_batch:
          - _pm
          - _co2
          - _temperature_c
          - _temperature_f
```

The final section sets up an `interval` timer to publish some of the
measurements every 30 seconds. The `publish_batch` action is used
to ensure that all of the referenced measurements are published with
identical timestamps.
