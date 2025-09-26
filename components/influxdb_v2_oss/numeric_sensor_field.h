#pragma once

#include "field.h"

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace influxdb {

enum class NumericSensorFieldFormat { Float, Integer, UnsignedInteger };

class NumericSensorField : public Field {
public:
  void set_sensor(sensor::Sensor *sensor) { this->sensor_ = sensor; }
  void set_format(std::string format) {
    if (format[0] == 'f') {
      this->format_ = NumericSensorFieldFormat::Float;
    } else if (format[0] == 'i') {
      this->format_ = NumericSensorFieldFormat::Integer;
    } else {
      this->format_ = NumericSensorFieldFormat::UnsignedInteger;
    }
  }
  void set_accuracy_decimals(int8_t val) { this->accuracy_decimals_ = val; }

  void do_setup() override;
  bool sensor_has_state() const override { return this->sensor_->has_state(); }
  std::string sensor_object_id() const override { return this->sensor_->get_object_id(); }
  std::string sensor_object_name() const override { return this->sensor_->get_name(); }
  std::string to_value() const override;

protected:
  sensor::Sensor *sensor_;
  NumericSensorFieldFormat format_;
  int8_t accuracy_decimals_{4};
};

}  // namespace influxdb
}  // namespace esphome

#endif
