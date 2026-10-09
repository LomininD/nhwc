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
  LFUCache(std::size_t capacity) : capacity_(capacity), min_freq_(1) {}

  std::size_t max_capacity() const { return capacity_; }
  bool is_full() const { return (capacity_ == cache_map_.size()); }

  bool lookup_update(Key& key, std::function<Value(Key)> slow_get_page) {
    if (max_capacity() == 0)
      return false;

    if (auto it = cache_map_.find(key); it != cache_map_.end()) {
      Record node = std::move(*(it->second));
      freq_to_list_map_[node.freq].erase(it->second);
      node.freq += 1;


      freq_to_list_map_[node.freq].emplace_front(std::move(node));
      it->second = freq_to_list_map_[node.freq].begin();

      if (freq_to_list_map_[min_freq_].empty())
        min_freq_++;

      return true;
    } else {
      if (is_full()) {
        auto victim = freq_to_list_map_[min_freq_].back();
        cache_map_.erase(victim.key);
        freq_to_list_map_[min_freq_].pop_back();
      }

      auto page = slow_get_page(key);

      min_freq_ = 1;
      auto& temp_lst = freq_to_list_map_[min_freq_];
      temp_lst.emplace_front(key, std::move(page), 1);
      cache_map_[key] = temp_lst.begin();

      return false;
    }
  }

private:
  const std::size_t capacity_;
  int min_freq_;

  struct Record {
    Key key;
    Value page;
    unsigned int freq;
  };

  using NodeIt = typename std::list<Record>::iterator;
  std::unordered_map<Key, NodeIt> cache_map_;
  std::unordered_map<unsigned int, std::list<Record>> freq_to_list_map_;
};

}  // namespace caches
