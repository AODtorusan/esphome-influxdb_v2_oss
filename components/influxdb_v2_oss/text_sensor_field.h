#pragma once

#include "field.h"

#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"

namespace esphome {
namespace influxdb {

class TextSensorField : public Field {
public:
  void set_sensor(text_sensor::TextSensor *sensor) { this->sensor_ = sensor; }
  void set_raw_state(bool val) { this->raw_state_ = val; }

  void setup() override;
  bool sensor_has_state() const override { return this->sensor_->has_state(); }
  std::string sensor_object_id() const override { return this->sensor_->get_object_id(); }
  void to_line(std::string &line) const override;

protected:
  text_sensor::TextSensor *sensor_;
  bool raw_state_{false};
};

}  // namespace influxdb
}  // namespace esphome

#endif
