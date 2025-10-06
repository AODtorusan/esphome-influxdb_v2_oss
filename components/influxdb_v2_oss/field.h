#pragma once

#include <vector>
#include <string>
#include "esphome/core/defines.h"
#include "esphome/components/time/real_time_clock.h"
#include "esphome/core/entity_base.h"

#include "influxdb.h"

namespace esphome {
namespace influxdb {

const std::string STR_TAG_DEVICE_CLASS("device_class");
const std::string STR_TAG_UNIT("unit");
const std::string STR_TAG_STATE_CLASS("state_class");
const std::string STR_TAG_SENSOR_ID("sensor_id");
const std::string STR_TAG_SENSOR_NAME("sensor_name");

class Field {
public:
  void set_clock(time::RealTimeClock *clock) { this->clock_ = clock; }

  void set_measurement(std::string measurement) { this->measurement_ = measurement; }
  const std::string& get_measurement() const { return this->measurement_; }
  void set_field_name(std::string name) { this->field_name_ = name; }
  const std::string& get_field_name() const { return this->field_name_; }

  void add_tag(const std::string& tag, const std::string& value);
  const std::list<std::pair<const std::string, const std::string>>& get_tags() const { return this->tags_; }

  BacklogEntry to_entry( const std::string& url );

  void setup(DefaultNamePolicy default_name_policy);

  virtual void do_setup() = 0;
  virtual bool sensor_has_state() const = 0;
  virtual std::string sensor_object_id() const = 0;
  virtual std::string sensor_object_name() const = 0;
  virtual std::string sensor_object_device_class() const = 0;
  virtual std::string to_value() const = 0;

protected:
  std::string measurement_;
  std::list<std::pair<const std::string, const std::string>> tags_;
  std::string field_name_;
  time::RealTimeClock *clock_{nullptr};
};

class IgnoredField: public Field {

  void set_sensor(EntityBase* entity) { this->entity = entity; }

  void do_setup() override {}
  bool sensor_has_state() const override { return false; };
  std::string sensor_object_id() const override { return entity->get_object_id(); }
  std::string sensor_object_name() const override { return entity->get_name(); }
  std::string sensor_object_device_class() const override { return "none"; }
  std::string to_value() const override { return std::string(); }

protected:
  EntityBase* entity;

};

}  // namespace influxdb
}  // namespace esphome
