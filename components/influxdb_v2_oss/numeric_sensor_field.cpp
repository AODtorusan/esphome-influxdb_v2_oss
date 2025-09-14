
#include "numeric_sensor_field.h"

namespace esphome {
namespace influxdb {

#ifdef USE_SENSOR

void NumericSensorField::do_setup() {
  if (!this->sensor_->get_device_class().empty()) {
    this->add_tag("device_class", this->sensor_->get_device_class());
  }
  if (!this->sensor_->get_unit_of_measurement().empty()) {
    this->add_tag("unit", this->sensor_->get_unit_of_measurement());
  }
  this->add_tag("state_class", state_class_to_string(this->sensor_->get_state_class()));
}

void NumericSensorField::to_value(std::string &line) const {
  float state = this->sensor_->get_state();
  auto accuracy_decimals = this->sensor_->get_accuracy_decimals();
  if (accuracy_decimals > 0) {
    line += value_accuracy_to_string(state, accuracy_decimals);
  } else {
    line += std::to_string( state );
  }
}

#endif

}  // namespace influxdb
}  // namespace esphome
