
#include "binary_sensor_field.h"

namespace esphome {
namespace influxdb {

#ifdef USE_BINARY_SENSOR
void BinarySensorField::to_line(std::string &line) const {
  line += (this->sensor_->state ? 'T' : 'F');
}
#endif

}  // namespace influxdb
}  // namespace esphome
