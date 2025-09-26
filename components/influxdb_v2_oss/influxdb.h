#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/http_request/http_request.h"
#include "esphome/components/time/real_time_clock.h"

#include <list>
#include <vector>
#include <unordered_map>

namespace esphome {
namespace influxdb {

static const char *const TAG = "influxdb_v2_oss";

class Field;
class BacklogEntry;

class InfluxDB : public Component {
public:
  void setup() override;
  void dump_config() override;
  void loop() override;
  void set_http_request(http_request::HttpRequestComponent *http) { this->http_request_ = http; };
  void set_url(std::string url) { this->url_ = std::move(url); }
  void set_token(std::string token) { this->token_ = std::string("Token ") + token; }
  void set_measurement(std::string measurement) { this->measurement_ = measurement; }
  void set_default_name_from_id(bool default_name_from_id) { this->default_name_from_id_ = default_name_from_id; }
  void set_publish_all(bool publish_all) { this->publish_all_ = publish_all; }
  void add_tag( const std::string key, const std::string value ) { this->global_tags_.push_back(std::pair(key, value)); }
  void set_clock(time::RealTimeClock *clock) { this->clock_ = clock; }
  void set_backlog_max_depth(uint8_t val) { this->backlog_max_depth_ = val; }
  void set_backlog_drain_batch(uint8_t val) { this->backlog_drain_batch_ = val; }
  void add_field(Field* field) { this->fields_.push_back(field); }
  const std::string &get_url() { return this->url_; }

  void queue(BacklogEntry&& entry);

  float get_setup_priority() const override { return setup_priority::LATE; }

protected:
  bool send_data(const std::string &url, const std::string &data);

  bool publish_all_;
  bool default_name_from_id_;
  std::list<std::pair<const std::string, const std::string>> global_tags_;
  std::string measurement_;

  http_request::HttpRequestComponent *http_request_;
  std::string url_;
  std::string token_;
  std::list<http_request::Header> headers_;
  std::list<Field*> fields_;

  time::RealTimeClock *clock_{nullptr};
  std::list<BacklogEntry> backlog_;
  uint8_t backlog_max_depth_{20};
  uint8_t backlog_drain_batch_{5};
};

}  // namespace influxdb
}  // namespace esphome
