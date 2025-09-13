#pragma once

#include "field.h"

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"

namespace esphome {
namespace influxdb {

enum class NumericSensorFieldFormat { Float, Integer, UnsignedInteger };

class NumericSensorField : public Field {
public:
  void set_sensor(const sensor::Sensor *sensor) { this->sensor_ = sensor; }
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
  void set_raw_state(bool val) { this->raw_state_ = val; }

  bool sensor_has_state() const override { return this->sensor_->has_state(); }
  std::string sensor_object_id() const override { return this->sensor_->get_object_id(); }
  void to_line(std::string &line) const override;

protected:
  const sensor::Sensor *sensor_;
  NumericSensorFieldFormat format_;
  int8_t accuracy_decimals_{4};
  bool raw_state_{false};
};

}  // namespace influxdb
}  // namespace esphome

#endif
