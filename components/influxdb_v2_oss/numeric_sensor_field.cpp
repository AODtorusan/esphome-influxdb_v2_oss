
#include "numeric_sensor_field.h"

namespace esphome {
namespace influxdb {

#ifdef USE_SENSOR
void NumericSensorField::to_line(std::string &line) const {
  float state;

  if (this->raw_state_) {
    state = this->sensor_->get_raw_state();
  } else {
    state = this->sensor_->get_state();
  }

  switch (this->format_) {
  case NumericSensorFieldFormat::Float:
    line += value_accuracy_to_string(state, this->accuracy_decimals_);
    break;
  case NumericSensorFieldFormat::Integer:
    line += str_sprintf("%ldi", std::lroundf(state));
    break;
  case NumericSensorFieldFormat::UnsignedInteger:
    line += str_sprintf("%ldu", std::lroundf(std::abs(state)));
    break;
  }
}
#endif

}  // namespace influxdb
}  // namespace esphome
