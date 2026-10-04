module;

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <list>

export module config;

import multi_level_cache;

export namespace caches::config
{

std::list<caches::CacheLevel> parse_cache_levels_algorithms(const std::filesystem::path &config_path)
{
    std::ifstream file(config_path);

    if (!file)
    {
        std::cerr << "Unable to open config file '" << config_path << "'." << std::endl;
        std::exit(1);
    }

    std::list<caches::CacheLevel> levels;
    std::size_t cache_levels;

    file >> cache_levels;
    for (int i = 0; i < cache_levels; i++)
    {
        std::string level_algorithm;

        file >> level_algorithm;
        caches::CacheLevel level = {
            .type = caches::string_to_cache_type(level_algorithm),
            .capacity = 0
        };
        levels.emplace_back(level);
    }

    return levels;
}

}
