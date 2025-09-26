
#include "field.h"
#include "influxdb.h"
#include "backlog_entry.h"

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

void Field::add_tag(const std::string& tag, const std::string &value) {
  this->tags_.emplace_back( tag, value );
}

BacklogEntry Field::to_entry( const std::string& url ) {
  time_t now = this->clock_->timestamp_now();
  std::string value = this->to_value();
  return BacklogEntry(url, this, std::move(value), now);
}

}  // namespace influxdb
}  // namespace esphome
