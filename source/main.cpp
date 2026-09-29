#include <iostream>
#include <cstring>

import multi_level_cache;
import config;
import util;

using Page = long long;
using PageId = long long;

int main(int argc, char *argv[])
{
    auto config_path = caches::config::default_config_path;
    if (argc > 1 && 0 == std::strcmp(argv[1], "--config"))
    {
        if (argc < 3)
        {
            std::cerr << "error: --config requires a path argument.\n";
            return 1;
        }
        config_path = argv[2];
    }

    auto levels = caches::config::parse_cache_levels_algorithms(config_path);
    for (auto& level : levels)
    {
        long long n;
        auto read_ok = util::read_integer(n);
        if (!read_ok || util::can_not_be_valid_size_t(n))
        {
            std::cerr << "Expected nonnegative cache size.\n";
            return 1;
        }
        level.capacity = n;
    }

    caches::MultiLevelCache<Page, PageId> cache(levels);

    long long data_len;
    auto read_ok = util::read_integer(data_len);
    if (!read_ok || util::can_not_be_valid_size_t(data_len))
    {
        std::cerr << "Expected nonnegative cache size.\n";
        return 1;
    }

    auto load = [](PageId key) { return key; };

    unsigned int hits = 0;
    for (int i = 0; i < data_len; i++)
    {
        PageId key;

        auto read_ok = util::read_integer(key);
        if (!read_ok || util::can_not_be_valid_size_t(key))
        {
            std::cerr << "Expected nonnegative page key.\n";
            return 1;
        }

        bool hit = cache.lookup_update(key, load);
        if (hit)
            ++hits;
    }
    std::cout << hits << std::endl;
}
