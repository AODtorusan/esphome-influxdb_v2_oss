
#include "text_sensor_field.h"

namespace esphome {
namespace influxdb {

#ifdef USE_TEXT_SENSOR

void TextSensorField::do_setup() {}

std::string TextSensorField::to_value() const {
  std::string value;
  value += '"';
  value += this->sensor_->get_state();
  value += '"';
  return value;
}

#endif

}  // namespace influxdb
}  // namespace esphome
