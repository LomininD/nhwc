#include <iostream>

import belady_cache;
import util;

using Page = long long;
using PageId = long long;

int main()
{
    long long cache_size, data_len;
    if (!util::read_integer(cache_size) || !util::read_integer(data_len)
        || util::can_not_be_valid_size_t(cache_size) || util::can_not_be_valid_size_t(data_len))
    {
        std::cerr << "Expected nonnegative cache and data size.\n";
        return 1;
    }

    caches::BeladyCache<Page, PageId> cache(cache_size);

    std::vector<PageId> requests(data_len);

    for (int i = 0; i < data_len; i++)
    {
        PageId key;

        auto read_ok = util::read_integer(key);
        if (!read_ok || util::can_not_be_valid_size_t(key))
        {
            std::cerr << "Expected nonnegative page key.\n";
            return 1;
        }

        requests[i] = key;
    }
    std::cout << cache.calculate_hits(requests) << std::endl;
}
