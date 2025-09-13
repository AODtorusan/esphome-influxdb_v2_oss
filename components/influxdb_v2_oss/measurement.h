#pragma once

#include <vector>
#include "esphome/core/defines.h"

#include "influxdb.h"
#include "field.h"

namespace esphome {
namespace influxdb {

class Measurement {
public:
  Measurement(InfluxDB *parent) : parent_(parent) {}

  void set_bucket(std::string bucket) { this->url_ = parent_->get_url() + "&bucket=" + bucket; }
  void set_line_prefix(std::string prefix) { this->line_prefix_ = std::move(prefix); }

  InfluxDB *get_parent() const { return this->parent_; }
  const std::string &get_url() const { return this->url_; }
  void add_field(const Field *sensor) { this->fields_.push_back(sensor); }

  std::string to_line(const std::string &timestamp) const;

protected:
  InfluxDB *parent_;
  std::string url_;
  std::string line_prefix_;
  std::vector<const Field *> fields_;
};

}  // namespace influxdb
}  // namespace esphome
