#include "influxdb.h"

namespace esphome {
namespace influxdb {

#ifdef USE_BINARY_SENSOR
void BinarySensorField::publish(std::string &line) const {
  line += (this->sensor_->state ? 'T' : 'F');
}
#endif

#ifdef USE_SENSOR
void SensorField::publish(std::string &line) const {
  float state;

  if (this->raw_state_) {
    state = this->sensor_->get_raw_state();
  } else {
    state = this->sensor_->get_state();
  }

  switch (this->format_) {
  case SensorFieldFormat::Float:
    line += value_accuracy_to_string(state, this->accuracy_decimals_);
    break;
  case SensorFieldFormat::Integer:
    line += str_sprintf("%ldi", std::lroundf(state));
    break;
  case SensorFieldFormat::UnsignedInteger:
    line += str_sprintf("%ldu", std::lroundf(std::abs(state)));
    break;
  }
}
#endif

#ifdef USE_TEXT_SENSOR
void TextSensorField::publish(std::string &line) const {
  line += '"';

  if (this->raw_state_) {
    line += this->sensor_->get_raw_state();
  } else {
    line += this->sensor_->get_state();
  }

  line += '"';
}
#endif

void Measurement::publish() {
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

    field->publish(line);

    sensor_sep = ',';
  }

  this->parent_->publish_measurement(this->url_, line);
}

void InfluxDB::publish_measurement(const std::string &url, std::string &measurement) {
  if (this->clock_ != nullptr) {
    auto time = this->clock_->now();
    measurement += str_sprintf(" %jd", (intmax_t) time.timestamp);
  }

  ESP_LOGD(TAG, "Publishing: %s", measurement.c_str());

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
  this->http_request_->set_url(url.c_str());
  this->http_request_->set_body(measurement.c_str());

#ifdef USE_ESP_IDF
  this->http_request_->send();
#else
  this->http_request_->send({});
  this->http_request_->close();
#endif

  // check response code
}

}  // namespace influxdb
}  // namespace esphome
