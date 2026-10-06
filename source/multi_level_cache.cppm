module;

#include <cstddef>
#include <functional>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>

export module multi_level_cache;

import arc_cache;
import lirs_cache;
import two_queue_cache;
import lfu_cache;
import lru_cache;
import base_cache;

export namespace caches {

enum class CacheType { kARC, k2Q, kLRU, kLFU, kLIRS };

struct CacheLevel {
  CacheType type;
  std::size_t capacity;
};

CacheType string_to_cache_type(const std::string_view str) {
  if (str == "LRU")
    return CacheType::kLRU;
  if (str == "ARC")
    return CacheType::kARC;
  if (str == "2Q")
    return CacheType::k2Q;
  if (str == "LFU")
    return CacheType::kLFU;
  if (str == "LIRS")
    return CacheType::kLIRS;
  throw std::invalid_argument("unknown cache type: " + std::string(str));
}

template <typename T, typename KeyT = int>
class MultiLevelCache {
public:
  MultiLevelCache(std::ranges::input_range auto&& levels) {
    for (const auto& level : levels) {
      switch (level.type) {
        case CacheType::kARC:
          cache_.emplace_back(std::make_unique<ARCCache<T, KeyT>>(level.capacity));
          break;
        case CacheType::k2Q:
          cache_.emplace_back(std::make_unique<TwoQueueCache<T, KeyT>>(level.capacity));
          break;
        case CacheType::kLRU:
          cache_.emplace_back(std::make_unique<LRUCache<T, KeyT>>(level.capacity));
          break;
        case CacheType::kLFU:
          cache_.emplace_back(std::make_unique<LFUCache<T, KeyT>>(level.capacity));
          break;
        case CacheType::kLIRS:
          cache_.emplace_back(std::make_unique<LIRSCache<T, KeyT>>(level.capacity));
          break;
      }
    }
  }

  bool lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page) {
    bool loaded = false;
    T page;

    auto get_page = [&](KeyT key) -> T {
      if (!loaded) {
        page = slow_get_page(key);
        loaded = true;
      }

      return page;
    };

    for (const auto& level : cache_) {
      auto hit = level->lookup_update(key, get_page);
      if (hit)
        return true;
    }
    return false;
  }

private:
  std::list<std::unique_ptr<BaseCache<T, KeyT>>> cache_;
};

}  // namespace caches
