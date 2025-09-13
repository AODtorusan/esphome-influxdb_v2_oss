
#include "binary_sensor_field.h"

namespace esphome {
namespace influxdb {

#ifdef USE_BINARY_SENSOR

void BinarySensorField::setup() {
  if (!this->sensor_->get_device_class().empty()) {
    this->add_tag("device_class", this->sensor_->get_device_class());
  }

  if (this->get_field_name().empty()) {
    this->set_field_name( this->sensor_object_id() );
  }
}

void BinarySensorField::to_line(std::string &line) const {
  line += this->tags_;
  line += ' ';
  line += this->get_field_name();
  line += '=';
  line += (this->sensor_->state ? 'T' : 'F');
}

#endif

}  // namespace influxdb
}  // namespace esphome
