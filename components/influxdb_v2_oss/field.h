#pragma once

#include <vector>
#include <string>
#include "esphome/core/defines.h"

namespace esphome {
namespace influxdb {

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

}  // namespace influxdb
}  // namespace esphome
