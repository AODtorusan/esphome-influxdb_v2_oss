
#include "numeric_sensor_field.h"
#include "backlog_entry.h"

namespace esphome {
namespace influxdb {

#ifdef USE_SENSOR

const std::string STR_DEVICE_CLASS("device_class");
const std::string STR_UNIT("unit");
const std::string STR_STATE_CLASS("state_class");

void NumericSensorField::do_setup() {
  if (!this->sensor_->get_device_class().empty()) {
    this->add_tag(STR_DEVICE_CLASS, this->sensor_->get_device_class());
  }
  if (!this->sensor_->get_unit_of_measurement().empty()) {
    this->add_tag(STR_UNIT, this->sensor_->get_unit_of_measurement());
  }
  this->add_tag(STR_STATE_CLASS, state_class_to_string(this->sensor_->get_state_class()));
}

std::string NumericSensorField::to_value() const {
  std::string value;
  float state = this->sensor_->get_state();
  auto accuracy_decimals = this->sensor_->get_accuracy_decimals();
  if (accuracy_decimals > 0) {
    value += value_accuracy_to_string(state, accuracy_decimals);
  } else {
    value += std::to_string( state );
  }
  return value;
}

#endif

}  // namespace influxdb
}  // namespace esphome
