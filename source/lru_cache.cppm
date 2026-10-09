module;

#include <cstddef>
#include <functional>
#include <list>
#include <unordered_map>

export module lru_cache;

import base_cache;

namespace caches {

export template <typename KeyT, typename T>
class LRUCache : public BaseCache<KeyT, T> {

public:
  explicit LRUCache(std::size_t capacity) : BaseCache<KeyT, T>(capacity) {}

private:
  using BaseCache<KeyT, T>::max_capacity;

  // Each entry is {key, page}; most recently used entry is at the front.
  std::list<std::pair<KeyT, T>> cache_;

  using ListIt = typename std::list<std::pair<KeyT, T>>::iterator;
  std::unordered_map<KeyT, ListIt> hash_;

  bool is_full() const { return (cache_.size() == max_capacity()); }

  bool do_lookup_update(const KeyT& key, std::function<T(KeyT)> slow_get_page) {
    if (max_capacity() == 0)
      return false;

    auto hit = hash_.find(key);
    if (hit != hash_.end()) {
      auto eltit = hit->second;
      cache_.splice(cache_.begin(), cache_, eltit);
      return true;
    }

    T page = slow_get_page(key);

    if (is_full()) {
      hash_.erase(cache_.back().first);
      cache_.pop_back();
    }
    cache_.emplace_front(key, page);
    hash_.emplace(key, cache_.begin());
    return false;
  }
};

}  // namespace caches
