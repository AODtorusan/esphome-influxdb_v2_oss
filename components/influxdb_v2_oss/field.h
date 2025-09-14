#pragma once

#include <vector>
#include <string>
#include "esphome/core/defines.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/entity_base.h"

namespace esphome {
namespace influxdb {

class Field {
public:
  void set_clock(time::RealTimeClock *clock) { this->clock_ = clock; }
  void set_measurement(std::string measurement) { this->measurement_ = measurement; }
  void set_field_name(std::string name) { this->field_name_ = name; }
  const std::string &get_field_name() const { return this->field_name_; }

  void add_tag(const std::string& tag, const std::string& value);
  std::string to_line();
  void setup(bool default_name_from_id);

  virtual void do_setup() = 0;
  virtual bool sensor_has_state() const = 0;
  virtual std::string sensor_object_id() const = 0;
  virtual std::string sensor_object_name() const = 0;
  virtual void to_value(std::string &line) const = 0;

protected:
  std::string measurement_;
  std::string tags_;
  std::string field_name_;
  time::RealTimeClock *clock_{nullptr};
};

class IgnoredField: public Field {

  void set_sensor(EntityBase* entity) { this->entity = entity; }

  void do_setup() override {}
  bool sensor_has_state() const override { return false; };
  std::string sensor_object_id() const override { return entity->get_object_id(); }
  std::string sensor_object_name() const override { return entity->get_name(); }
  void to_value(std::string &line) const override {}

protected:
  EntityBase* entity;

};

}  // namespace influxdb
}  // namespace esphome
