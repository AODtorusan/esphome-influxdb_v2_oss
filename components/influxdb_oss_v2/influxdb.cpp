#include "influxdb.h"

namespace esphome {
namespace influxdb {

#ifdef USE_BINARY_SENSOR
BinarySensorField::operator std::string() const {
  return this->sensor_->state ? "T" : "F";
}
#endif

#ifdef USE_SENSOR
SensorField::operator std::string() const {
  float state;

  if (this->raw_state_) {
    state = this->sensor_->get_raw_state();
  } else {
    state = this->sensor_->get_state();
  }

  switch (this->format_) {
  case SensorFieldFormat::Float:
    return "0.0";
  case SensorFieldFormat::Integer:
    return "0i";
  case SensorFieldFormat::UnsignedInteger:
    return "0u";
  }

  // will never be reached
  return "";
}
#endif

#ifdef USE_TEXT_SENSOR
TextSensorField::operator std::string() const {
  if (this->raw_state_) {
    return this->sensor_->get_raw_state();
  } else {
    return this->sensor_->get_state();
  }
}
#endif

void Measurement::publish() {
}

}  // namespace influxdb
}  // namespace esphome
