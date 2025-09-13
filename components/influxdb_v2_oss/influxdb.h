#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/component.h"
#include "esphome/components/http_request/http_request.h"
#include "esphome/components/time/real_time_clock.h"

#include <list>
#include <vector>

namespace esphome {
namespace influxdb {

static const char *const TAG = "influxdb_v2_oss";

class BacklogEntry {
public:
  BacklogEntry(const std::string &url, std::string &&data) : url(url), data(std::move(data)) {}

  const std::string &url;
  std::string data;
};

class Measurement;

class InfluxDB : public Component {
public:
  void setup() override;
  void loop() override;
  void set_http_request(http_request::HttpRequestComponent *http) { this->http_request_ = http; };
  void set_url(std::string url) { this->url_ = std::move(url); }
  void set_token(std::string token) { this->token_ = std::string("Token ") + token; }
  void set_clock(time::RealTimeClock *clock) { this->clock_ = clock; }
  void set_backlog_max_depth(uint8_t val) { this->backlog_max_depth_ = val; }
  void set_backlog_drain_batch(uint8_t val) { this->backlog_drain_batch_ = val; }

  float get_setup_priority() const override { return setup_priority::LATE; }

  static void queue_action(const Measurement *measurement);
  static void queue_batch_action(std::list<const Measurement *> measurements);

  const std::string &get_url() { return this->url_; }

protected:
  void queue(const std::string &url, std::string &&data);
  bool send_data(const std::string &url, const std::string &data);

  http_request::HttpRequestComponent *http_request_;
  std::string url_;
  std::string token_;
  std::list<http_request::Header> headers_;
  time::RealTimeClock *clock_{nullptr};
  std::list<BacklogEntry> backlog_;
  uint8_t backlog_max_depth_{10};
  uint8_t backlog_drain_batch_{1};
};

}  // namespace influxdb
}  // namespace esphome
