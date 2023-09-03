#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/component.h"

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif

#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

#include <vector>

namespace esphome {
namespace influxdb {

static const char *const TAG = "sunpower_solar";

class InfluxDB : public Component {
public:
  void set_url(const char *url) { this->url_ = url; }
  void set_token(const char *token) { this->token_ = token; }
  void set_useragent(const char *useragent) { this->useragent_ = useragent; }
#ifdef USE_TIME
  void set_clock(time::RealTimeClock *clock) { this->clock_ = clock; }
#endif

  void publish_measurement(const std::string &measurement);

protected:
  const char *url_;
  const char *token_{nullptr};
  const char *useragent_;
#ifdef USE_TIME
  time::RealTimeClock *clock_{nullptr};
#endif
};

#ifdef USE_BINARY_SENSOR
class BinarySensorField {
public:
  void set_sensor(const binary_sensor::BinarySensor *sensor) { this->sensor_ = sensor; }
  void set_field_name(const char *name) { this->field_name_ = name; }

  operator std::string() const;

protected:
  const binary_sensor::BinarySensor *sensor_;
  const char *field_name_;
};
#endif

#ifdef USE_SENSOR
enum class SensorFieldFormat { Float, Integer, UnsignedInteger };

class SensorField {
public:
  void set_sensor(const sensor::Sensor *sensor) { this->sensor_ = sensor; }
  void set_field_name(const char *name) { this->field_name_ = name; }
  void set_format(const char *format) {
    if (format[0] == 'f') {
      this->format_ = SensorFieldFormat::Float;
    } else if (format[0] == 'i') {
      this->format_ = SensorFieldFormat::Integer;
    } else {
      this->format_ = SensorFieldFormat::UnsignedInteger;
    }
  }
  void set_raw_state(bool val) { this->raw_state_ = val; }

  operator std::string() const;

protected:
  const sensor::Sensor *sensor_;
  const char *field_name_;
  SensorFieldFormat format_;
  bool raw_state_{false};
};
#endif

#ifdef USE_TEXT_SENSOR
class TextSensorField {
public:
  void set_sensor(const text_sensor::TextSensor *sensor) { this->sensor_ = sensor; }
  void set_field_name(const char *name) { this->field_name_ = name; }
  void set_raw_state(bool val) { this->raw_state_ = val; }

  operator std::string() const;

protected:
  const text_sensor::TextSensor *sensor_;
  const char *field_name_;
  bool raw_state_{false};
};
#endif

class Measurement {
public:
  Measurement(InfluxDB *parent) : parent_(parent) {}

  void set_line_prefix(const char *prefix) { this->line_prefix_ = prefix; }

#ifdef USE_BINARY_SENSOR
  void add_binary_sensor(const BinarySensorField *sensor) { this->binary_sensors_.push_back(sensor); }
#endif

#ifdef USE_SENSOR
  void add_sensor(const SensorField *sensor) { this->sensors_.push_back(sensor); }
#endif

#ifdef USE_TEXT_SENSOR
  void add_text_sensor(const TextSensorField *sensor) { this->text_sensors_.push_back(sensor); }
#endif

  void publish();

protected:
  InfluxDB *parent_;
  const char *line_prefix_;

#ifdef USE_BINARY_SENSOR
  std::vector<const BinarySensorField *> binary_sensors_;
#endif

#ifdef USE_SENSOR
  std::vector<const SensorField *> sensors_;
#endif

#ifdef USE_TEXT_SENSOR
  std::vector<const TextSensorField *> text_sensors_;
#endif
};

}  // namespace influxdb
}  // namespace esphome
