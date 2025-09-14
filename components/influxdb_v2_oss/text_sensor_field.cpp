
#include "text_sensor_field.h"

namespace esphome {
namespace influxdb {

#ifdef USE_TEXT_SENSOR

void TextSensorField::do_setup() {
  if (!this->sensor_->get_device_class().empty()) {
    this->add_tag("device_class", this->sensor_->get_device_class());
  }
}

void TextSensorField::to_value(std::string &line) const {
  line += '"';
  line += this->sensor_->get_state();
  line += '"';
}

#endif

}  // namespace influxdb
}  // namespace esphome
