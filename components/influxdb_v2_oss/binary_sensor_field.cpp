
#include "binary_sensor_field.h"

namespace esphome {
namespace influxdb {

#ifdef USE_BINARY_SENSOR

void BinarySensorField::do_setup() {}

std::string BinarySensorField::to_value() const {
  return std::string(this->sensor_->state ? "T" : "F");
}

#endif

}  // namespace influxdb
}  // namespace esphome
