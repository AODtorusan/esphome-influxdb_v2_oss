
#include "field.h"

namespace esphome {
namespace influxdb {

void Field::add_tag(const std::string& tag, const std::string& value) {
    if (this->tags_.size() > 0)
        this->tags_ += ',';
    this->tags_ += tag;
    this->tags_ += '=';
    this->tags_ += value;
}

}  // namespace influxdb
}  // namespace esphome
