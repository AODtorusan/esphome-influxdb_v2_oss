
#include "measurement.h"

namespace esphome {
namespace influxdb {

std::string Measurement::to_line(const std::string &timestamp) const {
  std::string line{this->line_prefix_};
  char sensor_sep = ' ';

  for (const auto field : this->fields_) {
    if (!field->sensor_has_state()) {
      continue;
    }

    line += sensor_sep;

    if (!field->get_field_name().empty()) {
      line += field->get_field_name();
    } else {
      line += field->sensor_object_id();
    }

    line += '=';

    field->to_line(line);

    sensor_sep = ',';
  }

  line += timestamp + '\n';

  return line;
}


}  // namespace influxdb
}  // namespace esphome
