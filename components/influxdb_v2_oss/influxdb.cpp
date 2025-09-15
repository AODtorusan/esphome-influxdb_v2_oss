#include "influxdb.h"

#include "binary_sensor_field.h"
#include "numeric_sensor_field.h"
#include "text_sensor_field.h"

namespace esphome {
namespace influxdb {

void InfluxDB::setup() {
  http_request::Header header;

  header.name = "Content-Type";
  header.value = "text/plain; charset=utf-8";
  this->headers_.push_back(header);

  header.name = "Content-Encoding";
  header.value = "identity";
  this->headers_.push_back(header);

  header.name = "Accept";
  header.value = "application/json";
  this->headers_.push_back(header);

  if (!this->token_.empty()) {
    header.name = "Authorization";
    header.value = this->token_.c_str();
    this->headers_.push_back(header);
  }

  if (this->publish_all_) {
#ifdef USE_BINARY_SENSOR
    for (auto *obj : App.get_binary_sensors()) {
      if (!obj->is_internal() && std::none_of(this->fields_.begin(), this->fields_.end(), [&obj](Field* o) { return o->sensor_object_id() == obj->get_object_id(); })) {
        BinarySensorField* field = new BinarySensorField();
        field->set_sensor(obj);
        field->set_measurement( this->measurement_ );
        obj->add_on_state_callback([this, field](bool state) {
          this->queue( this->url_, std::move(field->to_line()) );
        });
        this->add_field( field );
      }
    }
#endif
#ifdef USE_SENSOR
    for (auto *obj : App.get_sensors()) {
      if (!obj->is_internal() && std::none_of(this->fields_.begin(), this->fields_.end(), [&obj](Field* o) { return o->sensor_object_id() == obj->get_object_id(); })) {
        NumericSensorField* field = new NumericSensorField();
        field->set_sensor(obj);
        field->set_measurement( this->measurement_ );
        obj->add_on_state_callback([this, field](float state) {
          this->queue( this->url_, std::move(field->to_line()) );
        });
        this->add_field( field );
      }
    }
#endif
#ifdef USE_TEXT_SENSOR
    for (auto *obj : App.get_text_sensors()) {
      if (!obj->is_internal() && std::none_of(this->fields_.begin(), this->fields_.end(), [&obj](Field* o) { return o->sensor_object_id() == obj->get_object_id(); })) {
        TextSensorField* field = new TextSensorField();
        field->set_sensor(obj);
        field->set_measurement( this->measurement_ );
        obj->add_on_state_callback([this, field](std::string state) {
          this->queue( this->url_, std::move(field->to_line()) );
        });
        this->add_field( field );
      }
    }
#endif
  }

  for (auto field : this->fields_) {
    field->set_clock( this->clock_ );
    for (auto tag : this->global_tags_)
      field->add_tag( tag.first, tag.second );
    field->setup( this->default_name_from_id_ );
  }
}

void InfluxDB::dump_config() {
  ESP_LOGCONFIG(TAG, "InfluxDB component");
  ESP_LOGCONFIG(TAG, "  Fields:");
  for (auto field : this->fields_)
    ESP_LOGCONFIG(TAG, "    %s", field->get_field_name().c_str());
}

void InfluxDB::loop() {
  if (!this->backlog_.empty()) {
    ESP_LOGD(TAG, "Sending InfluxDB lines to server (queue size: %d)", this->backlog_.size());
    uint8_t item_count = 0;
    do {
      const auto &m = this->backlog_.front();

      auto success = this->send_data(m.url, m.data);
      if (success) {
        this->backlog_.pop_front();
        item_count++;
      } else {
        break;
      }
    } while (!this->backlog_.empty() && (item_count < this->backlog_drain_batch_));
    ESP_LOGD(TAG, "Drained %d items from backlog", item_count);
  } else {
    this->disable_loop();
  }
}

void InfluxDB::queue(const std::string &url, std::string &&data) {
  ESP_LOGD(TAG, "Adding data (%d) into the InfluxDB queue for %s: \n%s", data.size(), url.c_str(), data.c_str());
  if (this->backlog_.size() == this->backlog_max_depth_) {
    ESP_LOGW(TAG, "Backlog is full, dropping oldest entries.");
    this->backlog_.pop_front();
  }
  this->backlog_.emplace_back(url, std::move(data));
  this->enable_loop();
}

bool InfluxDB::send_data(const std::string &url, const std::string &data) {
  uint8_t buf[32];

  auto response = this->http_request_->post(url, data, this->headers_);

  if (response == nullptr) {
    ESP_LOGW(TAG, "Error sending metrics to %s", data.c_str());
    return false;
  }
  auto status_code = response->status_code;
  if (status_code != 204) {
    ESP_LOGW(TAG, "Error sending metrics to %s: HTTP %d", data.c_str(), status_code);
    return false;
  }

  // Drain the response
  while (response->read(buf, sizeof(buf)) != 0) {}
  response->end();

  return true;
}

}  // namespace influxdb
}  // namespace esphome
