#include <cstring>
#include <iostream>
// FIXME: this include should be redundant, but the project doesn't compile without it
#include <list>

import cli11;

import multi_level_cache;
import config;
import util;

using Page = long long;
using PageId = long long;

int main(int argc, char* argv[]) {
  CLI::App app{"NHWC"};

  std::string config_path;
  app.add_option("-c,--config", config_path, "The config path")->required();

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError& e) {
    return app.exit(e);
  }

  auto levels = caches::config::parse_cache_levels_algorithms(config_path);
  for (auto& level : levels) {
    long long n;
    auto read_ok = util::read_integer(n);
    if (!read_ok || util::can_not_be_valid_size_t(n)) {
      std::cerr << "Expected nonnegative cache size.\n";
      return 1;
    }
    level.capacity = n;
  }

  caches::MultiLevelCache<Page, PageId> cache{levels};

  long long data_len;
  auto read_ok = util::read_integer(data_len);
  if (!read_ok || util::can_not_be_valid_size_t(data_len)) {
    std::cerr << "Expected nonnegative cache size.\n";
    return 1;
  }

  auto load = [](PageId key) { return key; };

  unsigned int hits = 0;
  for (int i = 0; i < data_len; i++) {
    PageId key;

    auto read_ok = util::read_integer(key);
    if (!read_ok || util::can_not_be_valid_size_t(key)) {
      std::cerr << "Expected nonnegative page key.\n";
      return 1;
    }

    bool hit = cache.lookup_update(key, load);
    if (hit)
      ++hits;
  }
  std::cout << hits << std::endl;
}
