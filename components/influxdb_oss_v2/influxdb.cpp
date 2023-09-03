#include "influxdb.h"

namespace esphome {
namespace influxdb {

#ifdef USE_BINARY_SENSOR
bool BinarySensorField::publish(std::ostringstream &line) const {
  if (!this->sensor_->has_state()) {
    return false;
  }

  if (this->field_name_ != nullptr) {
    line << this->field_name_;
  } else {
    line << this->sensor_->get_object_id();
  }

  line << '=' << (this->sensor_->state ? 'T' : 'F');

  return true;
}
#endif

#ifdef USE_SENSOR
bool SensorField::publish(std::ostringstream &line) const {
  if (!this->sensor_->has_state()) {
    return false;
  }

  if (this->field_name_ != nullptr) {
    line << this->field_name_;
  } else {
    line << this->sensor_->get_object_id();
  }

  line << '=';

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

  return true;
}
#endif

#ifdef USE_TEXT_SENSOR
bool TextSensorField::publish(std::ostringstream &line) const {
  if (!this->sensor_->has_state()) {
    return false;
  }

  if (this->field_name_ != nullptr) {
    line << this->field_name_;
  } else {
    line << this->sensor_->get_object_id();
  }

  line << '=' << '"';

  if (this->raw_state_) {
    line << this->sensor_->get_raw_state();
  } else {
    line << this->sensor_->get_state();
  }

  line << '"';

  return true;
}
#endif

void Measurement::publish() {
  std::ostringstream line;

  line << this->line_prefix_ << ' ';

  std::string sensor_sep;

#ifdef USE_BINARY_SENSOR
  for (const auto sensor_field : this->binary_sensor_fields_) {
    line << sensor_sep;
    if (sensor_field->publish(line)) {
      sensor_sep = ",";
    } else {
      sensor_sep = "";
    }
  }
#endif

#ifdef USE_SENSOR
  for (const auto sensor_field : this->sensor_fields_) {
    line << sensor_sep;
    if (sensor_field->publish(line)) {
      sensor_sep = ",";
    } else {
      sensor_sep = "";
    }
  }
#endif

#ifdef USE_TEXT_SENSOR
  for (const auto sensor_field : this->text_sensor_fields_) {
    line << sensor_sep;
    if (sensor_field->publish(line)) {
      sensor_sep = ",";
    } else {
      sensor_sep = "";
    }
  }
#endif

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
