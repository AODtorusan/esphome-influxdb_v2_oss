
#include "text_sensor_field.h"

namespace esphome {
namespace influxdb {

#ifdef USE_TEXT_SENSOR

void TextSensorField::setup() {
  if (!this->sensor_->get_device_class().empty()) {
    this->add_tag("device_class", this->sensor_->get_device_class());
  }

  if (this->get_field_name().empty()) {
    this->set_field_name( this->sensor_object_id() );
  }
}

void TextSensorField::to_line(std::string &line) const {
  line += this->tags_;
  line += ' ';
  line += this->get_field_name();
  line += "=\"";
  if (this->raw_state_) {
    line += this->sensor_->get_raw_state();
  } else {
    line += this->sensor_->get_state();
  }
  line += '"';
}

#endif

}  // namespace influxdb
}  // namespace esphome
