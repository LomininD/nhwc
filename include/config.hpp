#pragma once

#include "multi_level_cache.hpp"

constexpr auto default_config_path = "config.txt";
std::list<caches::CacheLevel> parse_cache_levels_algorithms(const char* config_path);
