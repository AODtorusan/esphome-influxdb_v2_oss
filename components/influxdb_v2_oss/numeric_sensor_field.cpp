
#include "numeric_sensor_field.h"

namespace esphome {
namespace influxdb {

#ifdef USE_SENSOR

void NumericSensorField::setup() {
  if (!this->sensor_->get_device_class().empty()) {
    this->add_tag("device_class", this->sensor_->get_device_class());
  }
  if (!this->sensor_->get_unit_of_measurement().empty()) {
    this->add_tag("unit", this->sensor_->get_unit_of_measurement());
  }
  this->add_tag("state_class", state_class_to_string(this->sensor_->get_state_class()));

  if (this->get_field_name().empty()) {
    this->set_field_name( this->sensor_object_id() );
  }
}

void NumericSensorField::to_line(std::string &line) const {
  float state;
  if (this->raw_state_) {
    state = this->sensor_->get_raw_state();
  } else {
    state = this->sensor_->get_state();
  }

  line += this->tags_;
  line += ' ';
  line += this->get_field_name();
  line += '=';
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
