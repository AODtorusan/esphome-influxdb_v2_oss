#pragma once

#include "esphome/core/defines.h"
#include "esphome/core/component.h"

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif

#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

#ifdef USE_TIME
#include "esphome/components/time/real_time_clock.h"
#endif

#include <unordered_map>

namespace esphome {
namespace influxdb {

static const char *const TAG = "sunpower_solar";

class InfluxDB : public Component {
  friend class Measurement;

public:
  void set_url(const char *url) { this->url_ = url; }
  void set_token(const char *token) { this->token_ = token; }
  void set_useragent(const char *useragent) { this->useragent_ = useragent; }
  void add_tag(const char *key, const char *value) { this->tags_.insert({key, value}); }
#ifdef USE_TIME
  void set_clock(time::RealTimeClock *clock) { this->clock_ = clock; }
#endif

protected:
  const char *url_;
  const char *token_{nullptr};
  const char *useragent_;
  std::unordered_map<const char *, const char *> tags_;
#ifdef USE_TIME
  time::RealTimeClock *clock_{nullptr};
#endif
};

class Measurement {
public:
  Measurement(InfluxDB *parent) : parent_(parent) {}

  void add_tag(const char *key, const char *value) { this->tags_.insert({key, value}); }

protected:
  InfluxDB *parent_;
  std::unordered_map<const char *, const char *> tags_;
};

}  // namespace influxdb
}  // namespace esphome
