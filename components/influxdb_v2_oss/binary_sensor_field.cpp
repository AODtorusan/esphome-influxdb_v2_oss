
#include "binary_sensor_field.h"

namespace esphome {
namespace influxdb {

#ifdef USE_BINARY_SENSOR

void BinarySensorField::do_setup() {
  if (!this->sensor_->get_device_class().empty()) {
    this->add_tag("device_class", this->sensor_->get_device_class());
  }
}

std::string BinarySensorField::to_value() const {
  return std::string(this->sensor_->state ? "T" : "F");
}

#endif

}  // namespace influxdb
}  // namespace esphome
