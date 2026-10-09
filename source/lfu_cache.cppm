module;

#include <cstddef>
#include <functional>
#include <list>
#include <unordered_map>

export module lfu_cache;

import base_cache;

namespace caches {

export template <typename Key, typename Value>
class LFUCache : public BaseCache<Key, Value> {
public:
  using BaseCache<Key, Value>::max_capacity;

  LFUCache(std::size_t capacity) : BaseCache<Key, Value>(capacity), min_freq_(1) {}

  bool is_full() const { return (max_capacity() == cache_map_.size()); }

private:
  int min_freq_;

  struct Record {
    Key key;
    Value page;
    unsigned int freq;
  };

  using NodeIt = typename std::list<Record>::iterator;
  std::unordered_map<Key, NodeIt> cache_map_;
  std::unordered_map<unsigned int, std::list<Record>> freq_to_list_map_;

  bool do_lookup_update(const Key& key, std::function<Value(Key)> slow_get_page) {
    if (auto it = cache_map_.find(key); it != cache_map_.end()) {
      Record node = *(it->second);
      freq_to_list_map_[node.freq].erase(it->second);
      node.freq += 1;

      freq_to_list_map_[node.freq].push_front(node);
      cache_map_[key] = freq_to_list_map_[node.freq].begin();

      if (freq_to_list_map_[min_freq_].empty())
        min_freq_++;

      return true;
    } else {
      if (is_full()) {
        auto evicted_node = freq_to_list_map_[min_freq_].back();
        cache_map_.erase(evicted_node.key);
        freq_to_list_map_[min_freq_].pop_back();
      }

      auto page = slow_get_page(key);
      Record rec{.key = key, .page = page, .freq = 1};

      min_freq_ = 1;
      freq_to_list_map_[min_freq_].push_front(rec);
      cache_map_[key] = freq_to_list_map_[min_freq_].begin();

      return false;
    }
  }
};

}  // namespace caches
