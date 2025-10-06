#include "influxdb.h"

#include "backlog_entry.h"
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
          this->queue( std::move(field->to_entry( this->url_ )) );
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
          this->queue( std::move(field->to_entry( this->url_ )) );
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
          this->queue( std::move(field->to_entry( this->url_ )) );
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
    field->setup( this->default_name_policy );
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
      auto& m = this->backlog_.front();

      // Find all queued messages that go to the same url
      std::list<std::list<BacklogEntry>::iterator> active;
      size_t len = 0;
      uint_fast8_t idx;
      for (std::list<BacklogEntry>::iterator it = this->backlog_.begin(); it != this->backlog_.end(); ++it) {
        if (it->url == it->url && active.size() < this->backlog_drain_batch_) {
          active.push_back(it);
          len += it->length;
        }
        idx++;
      }

      std::string body;
      ESP_LOGD(TAG, "Reserving memory for influxdb POST body: %d b", len);
      body.reserve(len);
      for (auto& item : active)
        item->append( body );
      bool success = this->send_data(m.url, std::move(body));

      if (success) {
        for (auto& item : active) {
          item_count++;
          this->backlog_.erase(item);
        }
      } else {
        break;
      }
    } while (!this->backlog_.empty() && (item_count < this->backlog_drain_batch_));
    ESP_LOGD(TAG, "Drained %d items from backlog", item_count);
  } else {
    this->disable_loop();
  }
}

void InfluxDB::queue(BacklogEntry&& data) {
  if (data.timestamp < 946681200 /* J2000, check is we have an absolute time */) {
    ESP_LOGW(TAG, "Cannot submit influxdb metrics for %s, clock is not ready!", data.field->get_field_name().c_str());
    return;
  }

  ESP_LOGD(TAG, "Adding data (%d) into the InfluxDB queue for %s", data.length, data.url.c_str());
  if (this->backlog_.size() == this->backlog_max_depth_) {
    ESP_LOGW(TAG, "Backlog is full, dropping oldest entries.");
    this->backlog_.pop_front();
  }
  this->backlog_.push_back(data);
  this->enable_loop();
}

bool InfluxDB::send_data(const std::string &url, const std::string &data) {
  uint8_t buf[32];
  auto response = this->http_request_->post(url, data, this->headers_);

  if (response == nullptr) {
    ESP_LOGW(TAG, "Error sending metrics to %s", url.c_str());
    return false;
  }
  auto status_code = response->status_code;
  if (status_code != 204) {
    ESP_LOGW(TAG, "Error sending metrics to %s: HTTP %d", url.c_str(), status_code);
    return false;
  }

  // Drain the response
  while (response->read(buf, sizeof(buf)) != 0) {}
  response->end();

  return true;
}

}  // namespace influxdb
}  // namespace esphome
