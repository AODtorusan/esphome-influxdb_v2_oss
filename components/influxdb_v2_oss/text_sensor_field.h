#pragma once

#include "field.h"

#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace influxdb {

class TextSensorField : public Field {
public:
  void set_sensor(text_sensor::TextSensor *sensor) { this->sensor_ = sensor; }

  void do_setup() override;
  bool sensor_has_state() const override { return this->sensor_->has_state(); }
  std::string sensor_object_id() const override { return this->sensor_->get_object_id(); }
  std::string sensor_object_name() const override { return this->sensor_->get_name(); }
  std::string sensor_object_device_class() const override { return this->sensor_->get_device_class(); }
  std::string to_value() const override;

protected:
  text_sensor::TextSensor *sensor_;
};

}  // namespace influxdb
}  // namespace esphome

#endif
