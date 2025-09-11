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

#include "esphome/components/time/real_time_clock.h"

#include <list>
#include <vector>

namespace esphome {
namespace influxdb {

static const char *const TAG = "influxdb_v2_oss";

class BacklogEntry {
public:
  BacklogEntry(const std::string &url, std::string &&data) : url(url), data(std::move(data)) {}

  const std::string &url;
  std::string data;
};

class Measurement;

class InfluxDB : public Component {
public:
  void setup() override;
  void loop() override;
  void set_http_request(http_request::HttpRequestComponent *http) { this->http_request_ = http; };
  void set_url(std::string url) { this->url_ = std::move(url); }
  void set_token(std::string token) { this->token_ = std::string("Token ") + token; }
  void set_clock(time::RealTimeClock *clock) { this->clock_ = clock; }
  void set_backlog_max_depth(uint8_t val) { this->backlog_max_depth_ = val; }
  void set_backlog_drain_batch(uint8_t val) { this->backlog_drain_batch_ = val; }

  float get_setup_priority() const override { return setup_priority::LATE; }

  static void queue_action(const Measurement *measurement);
  static void queue_batch_action(std::list<const Measurement *> measurements);

  const std::string &get_url() { return this->url_; }

protected:
  void queue(const std::string &url, std::string &&data);
  bool send_data(const std::string &url, const std::string &data);

  http_request::HttpRequestComponent *http_request_;
  std::string url_;
  std::string token_;
  std::list<http_request::Header> headers_;
  time::RealTimeClock *clock_{nullptr};
  std::list<BacklogEntry> backlog_;
  uint8_t backlog_max_depth_{10};
  uint8_t backlog_drain_batch_{1};
};

class Field {
public:
  void set_field_name(std::string name) { this->field_name_ = std::move(name); }
  const std::string &get_field_name() const { return this->field_name_; }

  virtual bool sensor_has_state() const = 0;
  virtual std::string sensor_object_id() const = 0;
  virtual void to_line(std::string &line) const = 0;

protected:
  std::string field_name_;
};

#ifdef USE_BINARY_SENSOR
class BinarySensorField : public Field {
public:
  void set_sensor(const binary_sensor::BinarySensor *sensor) { this->sensor_ = sensor; }

  bool sensor_has_state() const override { return this->sensor_->has_state(); }
  std::string sensor_object_id() const override { return this->sensor_->get_object_id(); }
  void to_line(std::string &line) const override;

protected:
  const binary_sensor::BinarySensor *sensor_;
};
#endif

#ifdef USE_SENSOR
enum class SensorFieldFormat { Float, Integer, UnsignedInteger };

class SensorField : public Field {
public:
  void set_sensor(const sensor::Sensor *sensor) { this->sensor_ = sensor; }
  void set_format(std::string format) {
    if (format[0] == 'f') {
      this->format_ = SensorFieldFormat::Float;
    } else if (format[0] == 'i') {
      this->format_ = SensorFieldFormat::Integer;
    } else {
      this->format_ = SensorFieldFormat::UnsignedInteger;
    }
  }
  void set_accuracy_decimals(int8_t val) { this->accuracy_decimals_ = val; }
  void set_raw_state(bool val) { this->raw_state_ = val; }

  bool sensor_has_state() const override { return this->sensor_->has_state(); }
  std::string sensor_object_id() const override { return this->sensor_->get_object_id(); }
  void to_line(std::string &line) const override;

protected:
  const sensor::Sensor *sensor_;
  SensorFieldFormat format_;
  int8_t accuracy_decimals_{4};
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
  void to_line(std::string &line) const override;

protected:
  text_sensor::TextSensor *sensor_;
  bool raw_state_{false};
};
#endif

class Measurement {
public:
  Measurement(InfluxDB *parent) : parent_(parent) {}

  void set_bucket(std::string bucket) { this->url_ = parent_->get_url() + "&bucket=" + bucket; }
  void set_line_prefix(std::string prefix) { this->line_prefix_ = std::move(prefix); }

  InfluxDB *get_parent() const { return this->parent_; }
  const std::string &get_url() const { return this->url_; }

#ifdef USE_BINARY_SENSOR
  void add_binary_sensor_field(const BinarySensorField *sensor) { this->fields_.push_back(sensor); }
#endif

#ifdef USE_SENSOR
  void add_sensor_field(const SensorField *sensor) { this->fields_.push_back(sensor); }
#endif

#ifdef USE_TEXT_SENSOR
  void add_text_sensor_field(const TextSensorField *sensor) { this->fields_.push_back(sensor); }
#endif

  std::string to_line(const std::string &timestamp) const;

protected:
  InfluxDB *parent_;
  std::string url_;
  std::string line_prefix_;
  std::vector<const Field *> fields_;
};

}  // namespace influxdb
}  // namespace esphome
