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

#include <iomanip>
#include <sstream>
#include <vector>

namespace esphome {
namespace influxdb {

static const char *const TAG = "influxdb_oss_v2";

class InfluxDB : public Component {
public:
  void set_url(const char *url) { this->url_ = url; }
  void set_token(const char *token) { this->token_ = token; }
  void set_useragent(const char *useragent) { this->useragent_ = useragent; }
#ifdef USE_TIME
  void set_clock(time::RealTimeClock *clock) { this->clock_ = clock; }
#endif

  void setup() override;

  void publish_measurement(std::ostringstream &measurement);

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

  bool publish(std::ostringstream &line) const;

protected:
  const binary_sensor::BinarySensor *sensor_;
  const char *field_name_{nullptr};
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

  bool publish(std::ostringstream &line) const;

protected:
  const sensor::Sensor *sensor_;
  const char *field_name_{nullptr};
  SensorFieldFormat format_;
  bool raw_state_{false};
};
#endif

#ifdef USE_TEXT_SENSOR
class TextSensorField {
public:
  void set_sensor(text_sensor::TextSensor *sensor) { this->sensor_ = sensor; }
  void set_field_name(const char *name) { this->field_name_ = name; }
  void set_raw_state(bool val) { this->raw_state_ = val; }

  bool publish(std::ostringstream &line) const;

protected:
  text_sensor::TextSensor *sensor_;
  const char *field_name_{nullptr};
  bool raw_state_{false};
};
#endif

class Measurement {
public:
  Measurement(InfluxDB *parent) : parent_(parent) {}

  void set_line_prefix(const char *prefix) { this->line_prefix_ = prefix; }

#ifdef USE_BINARY_SENSOR
  void add_binary_sensor_field(const BinarySensorField *sensor) { this->binary_sensor_fields_.push_back(sensor); }
#endif

#ifdef USE_SENSOR
  void add_sensor_field(const SensorField *sensor) { this->sensor_fields_.push_back(sensor); }
#endif

#ifdef USE_TEXT_SENSOR
  void add_text_sensor_field(const TextSensorField *sensor) { this->text_sensor_fields_.push_back(sensor); }
#endif

  void publish();

protected:
  InfluxDB *parent_;
  const char *line_prefix_;

#ifdef USE_BINARY_SENSOR
  std::vector<const BinarySensorField *> binary_sensor_fields_;
#endif

#ifdef USE_SENSOR
  std::vector<const SensorField *> sensor_fields_;
#endif

#ifdef USE_TEXT_SENSOR
  std::vector<const TextSensorField *> text_sensor_fields_;
#endif
};

}  // namespace influxdb
}  // namespace esphome
