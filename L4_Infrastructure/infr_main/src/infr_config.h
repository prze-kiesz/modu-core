#pragma once

#include <string>
#include <toml.hpp>

namespace infr {

struct InfrMainConfig {
    std::string device_name = "default_device";
    int port = 8080;
    bool enable_logging = true;
    double timeout_seconds = 30.0;
};

// ADL-based serialization functions
void to_toml(toml::value& dest, const InfrMainConfig& value);
void from_toml(const toml::value& src, InfrMainConfig& value);

}  // namespace infr
