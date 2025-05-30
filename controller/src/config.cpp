#include "config.h"
#include "fs.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

const string config_file = "/atlas-11.yaml";
const string default_config_str = R"(
# Default configuration for Atlas-11
version: 1.0
settings:
  debug: false
  log_level: info
)";

// Parse default config once at startup
const YAML::Node default_config = []() {
    return YAML::Load(default_config_str);
}();

YAML::Node read_config() {
    string config_str = slurp(config_file);

    if (!config_str.empty()) {
        try {
            return YAML::Load(config_str);
        } catch (const YAML::Exception &e) {
            cout << "Error parsing configuration file " << config_file << ": " << e.what() << endl;
        }
    }
    cout << "Using default configuration." << endl;
    return default_config;
}
