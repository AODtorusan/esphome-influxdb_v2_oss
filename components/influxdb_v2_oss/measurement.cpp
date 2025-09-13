
#include "measurement.h"

namespace esphome {
namespace influxdb {

void Measurement::setup() {
  for (auto field : this->fields_) {
    field->setup();
  }
}

std::string Measurement::to_line(const std::string &timestamp) const {
  std::string lines;
  lines.reserve( 128 * this->fields_.size() );
  for (const auto field : this->fields_) {
    if (!field->sensor_has_state()) {
      continue;
    }
    lines += this->name_;
    lines += ',';
    field->to_line(lines);
    lines += timestamp + '\n';
  }
  return lines;
}


}  // namespace influxdb
}  // namespace esphome
