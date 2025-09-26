#pragma once

#include <string>
#include <list>

namespace esphome {
namespace influxdb {

class Field;

class BacklogEntry {
public:

  BacklogEntry(const std::string& url, const Field* field, std::string&& value, time_t timestamp);
  void append( std::string& lines );

  size_t length;
  const std::string& url;
  const Field* field;
  const std::string value;
  const time_t timestamp;
};

}  // namespace influxdb
}  // namespace esphome
