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
  const auto default_precision {std::cout.precision()};
  float state;

  if (this->raw_state_) {
    state = this->sensor_->get_raw_state();
  } else {
    state = this->sensor_->get_state();
  }

  switch (this->format_) {
  case SensorFieldFormat::Float:
    if (this->accuracy_decimals_ != -1) {
      line << std::scientific << std::setprecision(this->accuracy_decimals_) << state << std::defaultfloat << std::setprecision(default_precision);
    } else {
      line << std::scientific << state << std::defaultfloat;
    }
    break;
  case SensorFieldFormat::Integer:
    line << std::setprecision(0) << state << 'i' << std::setprecision(default_precision);
    break;
  case SensorFieldFormat::UnsignedInteger:
    line << std::setprecision(0) << std::abs(state) << 'u' << std::setprecision(default_precision);
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
  std::list<http_request::Header> headers;
  http_request::Header header;

  this->http_request_->set_method("POST");

  header.name = "Content-Type";
  header.value = "text/plain; charset=utf-8";
  headers.push_back(header);

  header.name = "Content-Encoding";
  header.value = "identity";
  headers.push_back(header);

  header.name = "Accept";
  header.value = "application/json";
  headers.push_back(header);

  if (!this->token_.empty()) {
    header.name = "Authorization";
    header.value = this->token_.c_str();
    headers.push_back(header);
  }

  this->http_request_->set_headers(headers);
}

void InfluxDB::publish_measurement(std::ostringstream &measurement) {
  if (this->clock_ != nullptr) {
    auto time = this->clock_->now();
    measurement << time.strftime(" %s");
  }

  ESP_LOGD(TAG, "Publishing: %s", measurement.str().c_str());

  this->http_request_->set_body(measurement.str().c_str());
  this->http_request_->send({});
  this->http_request_->close();
  // check response code
}

}  // namespace influxdb
}  // namespace esphome
