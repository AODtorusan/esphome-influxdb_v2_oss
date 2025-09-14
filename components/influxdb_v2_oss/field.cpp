
#include "field.h"
#include "influxdb.h"

namespace esphome {
namespace influxdb {

void Field::setup(bool default_name_from_id) {
  if (this->get_field_name().empty()) {
    if (default_name_from_id) {
      this->set_field_name( this->sensor_object_id() );
    } else {
      this->set_field_name( str_sanitize(this->sensor_object_name()) );
    }
  }
  this->do_setup();
}

void Field::add_tag(const std::string& tag, const std::string& value) {
    if (this->tags_.size() > 0)
        this->tags_ += ',';
    this->tags_ += tag;
    this->tags_ += '=';
    this->tags_ += value;
}

std::string Field::to_line() {
  std::string line;
  time_t now = this->clock_->timestamp_now();
  if (now > 946681200 /* J2000, check is we have an absolute time */) {
    line.reserve( tags_.size() * 16 + 32 );
    line += this->measurement_;
    line += ',';
    line += this->tags_;
    line += ' ';
    line += this->get_field_name();
    line += '=';
    this->to_value( line );
    line += ' ';
    line += std::to_string( now );
    line += '\n';
  } else {
    ESP_LOGW(TAG, "Cannot submit influxdb metrics for %s, clock is not ready!", this->get_field_name().c_str());
  }
  return line;
}

}  // namespace influxdb
}  // namespace esphome
