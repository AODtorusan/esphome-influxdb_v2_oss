#pragma once

#include <vector>
#include "esphome/core/defines.h"

#include "influxdb.h"
#include "field.h"

namespace esphome {
namespace influxdb {

struct InfluxLineProtocol {
  std::string measturement_;
  std::vector<std::tuple<const std::string*, std::string>> tags_;
  std::vector<std::tuple<const std::string*, std::string>> values_;
  long timestamp_;
};

class Measurement {
public:
  Measurement(InfluxDB *parent) : parent_(parent) { parent->add_measurement(this); }

  void set_bucket(std::string bucket) { this->url_ = parent_->get_url() + "&bucket=" + bucket; }
  void set_name(std::string name) { this->name_ = std::move(name); }

  void setup();
  InfluxDB *get_parent() const { return this->parent_; }
  const std::string &get_url() const { return this->url_; }
  void add_field(Field *sensor) { this->fields_.push_back(sensor); }

  std::string to_line(const std::string &timestamp) const;

protected:
  InfluxDB *parent_;
  std::string name_;
  std::string url_;
  std::vector<Field *> fields_;
};

}  // namespace influxdb
}  // namespace esphome
