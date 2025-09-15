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

  - id: _wifi_rssi
    platform: wifi_signal
    name: ${node_name} WiFi Signal
```

This section configures two sensors, device uptime and the WiFi RSSI
(signal strength).

Any time a sensor receives a new value, it is immidiatly queued to be sent to InfluxDB

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
      accuracy_decimals: 1
      filters:
        - sliding_window_moving_average:
            window_size: 30
            send_every: 30

  - id: temperature_f
    platform: copy
    source_id: temperature_c
    unit_of_measurement: "°F"
    accuracy_decimals: 1
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
  watchdog_timeout: 16s
```

This section configures the `http_request` component, which is used by
the InfluxDB component to queue measurements to the server.

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
  sensors:
```

This section configures the InfluxDB component itself.

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
added to all measurements queued by the component. In this case a
tag named `device` will be sent containing the ESPHome `node_name`.

```yaml
influxdb_v2_oss:
  # ...
  sensors:
    _uptime:
      name: uptime
    _wifi_rssi:
      name: rssi
    _wifi_connected:
      name: connected
    _wifi_ip_address:
      name: ip_address
```

This section configures the first set of measurements. The key for
each entry is the ID of the sensor that is customized. The
bucket & measurement where these sensors will be published are
inherited from the top-level configuration. The names supplied are to
override the default names ('uptime' instead of '_uptime' and 'rssi'
instead of '_wifi_rssi', etc.)

It may be useful to note that if the WiFi network is not connected,
the value of `_wifi_connected` will be `false`, but queuing to the
InfluxDB server will not be possible. Because this configuration
includes a `time` component and the InfluxDB backlog is enabled, the
measurements containing a `false` value will be queued in the backlog
until the server becomes reachable again.

```yaml
sensor:
  - platform: sht4x
    ...
    temperature:
      id: temperature_c
      ...
      accuracy_decimals: 1

  - id: temperature_f
    ...
    accuracy_decimals: 1

influxdb_v2_oss:
  # ...
    temperature_c:
      name: temperature
      bucket: air_quality
      measurement: temperature
      tags:
        scale: C
    temperature_f:
      name: temperature
      bucket: air_quality
      measurement: temperature
      tags:
        scale: F
```

This section configures two measurements, which have different IDs but
the same `name`. This will result in two line entries for the same
bucket/measurement/value but a different tagset (when their timestamps
match). In InfluxDB these will be combined into a single measurement.
The result is that InfluxDB can be queried based on their tag values
depending on whether Celsius or Fahrenheit values are desired in the
query result.

This section also demonstrates how the floating-point values can be
rounded to a specified number of decimal places.
