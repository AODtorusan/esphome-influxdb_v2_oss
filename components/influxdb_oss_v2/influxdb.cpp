#include "influxdb.h"

namespace esphome {
namespace influxdb {

#ifdef USE_BINARY_SENSOR
void BinarySensorField::publish(std::ostringstream &line) const {
  line << (this->sensor_->state ? 'T' : 'F');
}
#endif

#ifdef USE_SENSOR
void SensorField::publish(std::ostringstream &line) const {
  float state;

  if (this->raw_state_) {
    state = this->sensor_->get_raw_state();
  } else {
    state = this->sensor_->get_state();
  }

  switch (this->format_) {
  case SensorFieldFormat::Float:
    line << std::scientific << state;
    break;
  case SensorFieldFormat::Integer:
    line << std::setprecision(0) << state << 'i';
    break;
  case SensorFieldFormat::UnsignedInteger:
    line << std::setprecision(0) << std::abs(state) << 'u';
    break;
  }
}
#endif

#ifdef USE_TEXT_SENSOR
void TextSensorField::publish(std::ostringstream &line) const {
  line << '"';

  if (this->raw_state_) {
    line << this->sensor_->get_raw_state();
  } else {
    line << this->sensor_->get_state();
  }

  line << '"';
}
#endif

void Measurement::publish() {
  std::ostringstream line;

  line << this->line_prefix_ << ' ';

  char sensor_sep = ' ';

  for (const auto field : this->fields_) {
    if (!field->sensor_has_state()) {
      continue;
    }

    line << sensor_sep;

    if (field->get_field_name() != nullptr) {
      line << field->get_field_name();
    } else {
      line << field->sensor_object_id();
    }

    line << '=';

    field->publish(line);

    sensor_sep = ',';
  }

  this->parent_->publish_measurement(line);
}

void InfluxDB::setup() {
  // setup http_request object
}

void InfluxDB::publish_measurement(std::ostringstream &measurement) {
  // check for clock and add timestamp
  // send line as POST body
  ESP_LOGD(TAG, "Publishing: %s", measurement.str().c_str());
}

}  // namespace influxdb
}  // namespace esphome
