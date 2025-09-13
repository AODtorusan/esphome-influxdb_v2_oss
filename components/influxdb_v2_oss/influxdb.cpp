#include "influxdb.h"
#include "measurement.h"

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

  for (auto measurement : this->measurements_) {
    measurement->setup();
  }
}

void InfluxDB::loop() {
  if (!this->backlog_.empty()) {
    ESP_LOGD(TAG, "Sending InfluxDB lines to server");
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
  }
  this->disable_loop();
}

void InfluxDB::queue_action(const Measurement *measurement) {
  std::string timestamp;
  auto db = measurement->get_parent();

  if (db->clock_ != nullptr) {
    auto time = db->clock_->now();
    timestamp = str_sprintf(" %jd", (intmax_t) time.timestamp);
  }

  db->queue(measurement->get_url(), measurement->to_line(timestamp));
}

void InfluxDB::queue_batch_action(std::list<const Measurement *> measurements) {
  std::string timestamp;
  auto db = measurements.front()->get_parent();
  const std::string& url = measurements.front()->get_url();
  std::string data;

  if (db->clock_ != nullptr) {
    auto time = db->clock_->now();
    timestamp = str_sprintf(" %jd", (intmax_t) time.timestamp);
  }

  for (auto measurement : measurements) {
    if (measurement->get_parent() != db) {
      ESP_LOGE(TAG, "Batch cannot include measurements for multiple databases.");
      continue;
    }

    if (measurement->get_url() != url) {
      ESP_LOGE(TAG, "Batch cannot include measurements for multiple buckets.");
      continue;
    }

    data += measurement->to_line(timestamp);
  }

  db->queue(url, std::move(data));
}

void InfluxDB::queue(const std::string &url, std::string &&data) {
  ESP_LOGD(TAG, "Adding data (%d) into the InfluxDB queue for %s", data.size(), url.c_str());
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
