
#include "text_sensor_field.h"

namespace esphome {
namespace influxdb {

#ifdef USE_TEXT_SENSOR
void TextSensorField::to_line(std::string &line) const {
  line += '"';

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
