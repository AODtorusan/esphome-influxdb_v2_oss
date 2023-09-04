#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/http_request/http_request.h"

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
#include <iostream>
#include <list>
#include <sstream>
#include <vector>

namespace esphome {
namespace influxdb {

static const char *const TAG = "influxdb_v2_oss";

class InfluxDB : public Component {
public:
  void set_http_request(http_request::HttpRequestComponent *http) { this->http_request_ = http; };
  void set_url(std::string url) { this->url_ = std::move(url); }
  void set_token(const char *token) { this->token_ = std::string("Token ") + token; }
#ifdef USE_TIME
  void set_clock(time::RealTimeClock *clock) { this->clock_ = clock; }
#endif

  float get_setup_priority() const override { return setup_priority::LATE; }

  void setup() override;

  void publish_measurement(const std::string &url, std::ostringstream &measurement);

  const std::string & get_url() { return this->url_; }

protected:
  http_request::HttpRequestComponent *http_request_;
  std::string url_;
  std::string token_;
#ifdef USE_TIME
  time::RealTimeClock *clock_{nullptr};
#endif
};

class Field {
public:
  void set_field_name(const char *name) { this->field_name_ = name; }
  const char *get_field_name() const { return this->field_name_; }

  virtual bool sensor_has_state() const = 0;
  virtual std::string sensor_object_id() const = 0;
  virtual void publish(std::ostringstream &line) const = 0;

protected:
  const char *field_name_{nullptr};
};

#ifdef USE_BINARY_SENSOR
class BinarySensorField : public Field {
public:
  void set_sensor(const binary_sensor::BinarySensor *sensor) { this->sensor_ = sensor; }

  bool sensor_has_state() const override { return this->sensor_->has_state(); }
  std::string sensor_object_id() const override { return this->sensor_->get_object_id(); }
  void publish(std::ostringstream &line) const override;

protected:
  const binary_sensor::BinarySensor *sensor_;
};
#endif

#ifdef USE_SENSOR
enum class SensorFieldFormat { Float, Integer, UnsignedInteger };

class SensorField : public Field {
public:
  void set_sensor(const sensor::Sensor *sensor) { this->sensor_ = sensor; }
  void set_format(const char *format) {
    if (format[0] == 'f') {
      this->format_ = SensorFieldFormat::Float;
    } else if (format[0] == 'i') {
      this->format_ = SensorFieldFormat::Integer;
    } else {
      this->format_ = SensorFieldFormat::UnsignedInteger;
    }
  }
  void set_accuracy_decimals(int val) { this->accuracy_decimals_ = val; }
  void set_raw_state(bool val) { this->raw_state_ = val; }

  bool sensor_has_state() const override { return this->sensor_->has_state(); }
  std::string sensor_object_id() const override { return this->sensor_->get_object_id(); }
  void publish(std::ostringstream &line) const override;

protected:
  const sensor::Sensor *sensor_;
  SensorFieldFormat format_;
  int accuracy_decimals_{-1};
  bool raw_state_{false};
};
#endif

#ifdef USE_TEXT_SENSOR
class TextSensorField : public Field {
public:
  void set_sensor(text_sensor::TextSensor *sensor) { this->sensor_ = sensor; }
  void set_raw_state(bool val) { this->raw_state_ = val; }

  bool sensor_has_state() const override { return this->sensor_->has_state(); }
  std::string sensor_object_id() const override { return this->sensor_->get_object_id(); }
  void publish(std::ostringstream &line) const override;

protected:
  text_sensor::TextSensor *sensor_;
  bool raw_state_{false};
};
#endif

class Measurement {
public:
  Measurement(InfluxDB *parent) : parent_(parent) {}

  void set_bucket(const char* bucket) { this->url_ = parent_->get_url() + "&bucket=" + bucket; }
  void set_line_prefix(const char *prefix) { this->line_prefix_ = prefix; }

#ifdef USE_BINARY_SENSOR
  void add_binary_sensor_field(const BinarySensorField *sensor) { this->fields_.push_back(sensor); }
#endif

#ifdef USE_SENSOR
  void add_sensor_field(const SensorField *sensor) { this->fields_.push_back(sensor); }
#endif

#ifdef USE_TEXT_SENSOR
  void add_text_sensor_field(const TextSensorField *sensor) { this->fields_.push_back(sensor); }
#endif

  void publish();

protected:
  InfluxDB *parent_;
  std::string url_;
  const char *line_prefix_;
  std::vector<const Field *> fields_;
};

}  // namespace influxdb
}  // namespace esphome
