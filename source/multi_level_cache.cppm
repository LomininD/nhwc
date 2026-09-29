module;

#include <functional>
#include <list>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

#include "lfu_cache.hpp"
#include "two_queue_cache.hpp"

export module multi_level_cache;

import arc_cache;
import lirs_cache;
import lru_cache;
import base_cache;

export namespace caches
{

enum class CacheType
{
    ARC,
    TWO_QUEUE,
    LRU,
    LFU,
    LIRS
};

struct CacheLevel
{
    CacheType type;
    std::size_t capacity;
};

CacheType string_to_cache_type(const std::string_view str)
{
    if (str == "LRU")
        return CacheType::LRU;
    if (str == "ARC")
        return CacheType::ARC;
    if (str == "2Q")
        return CacheType::TWO_QUEUE;
    if (str == "LFU")
        return CacheType::LFU;
    if (str == "LIRS")
        return CacheType::LIRS;
    throw std::invalid_argument("unknown cache type: " + std::string(str));
}

template <typename T, typename KeyT = int> class MultiLevelCache
{
    std::list<std::unique_ptr<BaseCache<T, KeyT>>> cache_;

public:
    MultiLevelCache(std::list<CacheLevel> levels)
    {
        for (auto level : levels)
        {
            switch (level.type)
            {
                case CacheType::ARC:
                    cache_.emplace_back(std::make_unique<ARCCache<T, KeyT>>(level.capacity));
                    break;
                case CacheType::TWO_QUEUE:
                    cache_.emplace_back(std::make_unique<TwoQueueCache<T, KeyT>>(level.capacity));
                    break;
                case CacheType::LRU:
                    cache_.emplace_back(std::make_unique<LRUCache<T, KeyT>>(level.capacity));
                    break;
                case CacheType::LFU:
                    cache_.emplace_back(std::make_unique<LFUCache<T, KeyT>>(level.capacity));
                    break;
                case CacheType::LIRS:
                    cache_.emplace_back(std::make_unique<LIRSCache<T, KeyT>>(level.capacity));
                    break;
            }
        }
    }

    bool lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page)
    {
        bool loaded = false;
        T page;

        auto get_page = [&](KeyT key) -> T
        {
            if (!loaded)
            {
                page = slow_get_page(key);
                loaded = true;
            }

            return page;
        };

        for (const auto& level : cache_)
        {
            auto hit = level->lookup_update(key, get_page);
            if (hit)
                return true;
        }
        return false;
    }
};

} // namespace caches
