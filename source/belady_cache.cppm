module;

#include <unordered_map>
#include <vector>
#include <list>
#include <queue>
#include <functional>

export module belady_cache;

namespace caches {

export template <typename Key, typename Value>
class BeladyCache {
public:
  BeladyCache(std::size_t capacity) : capacity_(capacity) {}

  std::size_t max_capacity() const { return capacity_; }
  bool is_full() const { return (capacity_ == cache_map_.size()); }

  std::size_t calculate_hits(const std::vector<Key>& request_sequence,
                             std::function<Value(Key)> slow_get_page) {
    if (max_capacity() == 0)
      return 0;

    std::vector<std::size_t> next_occurrence(request_sequence.size());
    // next_occurrence stores next index in request_sequence where request_sequence[i] occurs again

    init_occurrence_order(request_sequence, next_occurrence);

    std::size_t hits = 0;

    for (std::size_t i = 0; i < request_sequence.size(); ++i){
      const Key& cur = request_sequence[i];
      const std::size_t& next = next_occurrence[i];

      auto it = request_occurrence_map_.find(cur);
      if (it != request_occurrence_map_.end()) {
        ++hits;
        it->second = next;
        heap_.push({ cur, next });
      }
      else {
        if (is_full()) {
          while (!heap_.empty()) {
            auto top = heap_.top();
            auto it = request_occurrence_map_.find(top.key);
            if (it != request_occurrence_map_.end() && it->second == top.next)
              break;
            heap_.pop();
          }

          Key victim = heap_.top().key;

          delete_page(victim);
        }

        insert_page(cur, next, slow_get_page);
      }
    }

    clear_cache();
    return hits;
  }

private:
  const size_t capacity_;

  struct HeapNode
  {
    Key key;
    std::size_t next;
  };

  struct HeapCmp
  {
    bool operator()(const HeapNode &a, const HeapNode &b) const { return a.next < b.next; }
  };

  std::list<Value> cache_list_;
  std::unordered_map<Key, typename std::list<Value>::iterator> cache_map_;
  std::unordered_map<Key, std::size_t> request_occurrence_map_;
  std::priority_queue<HeapNode, std::vector<HeapNode>, HeapCmp> heap_;

  void init_occurrence_order(const std::vector<Key>& request_sequence,
                             std::vector<std::size_t>& next_occurrence) {
    std::unordered_map<Key, std::size_t> last_seen;
    // last_seen maps key to the nearest occurrence in request_sequence
    // only required in this function

    std::size_t seq_size = request_sequence.size();
    for (std::size_t i = seq_size; i-- > 0; ) { // to avoid unsigned overflow and infinite loop
      auto it = last_seen.find(request_sequence[i]);
      next_occurrence[i] = (it == last_seen.end()) ? seq_size : it->second;
      last_seen[request_sequence[i]] = i;
    }
  }

  void delete_page(const Key& victim)
  {
    request_occurrence_map_.erase(victim);
    heap_.pop();
    cache_list_.erase(cache_map_[victim]);
    cache_map_.erase(victim);
  }

  void insert_page(const Key& key, const std::size_t next, std::function<Value(Key)> slow_get_page)
  {
    request_occurrence_map_[key] = next;
    heap_.push({key, next});
    cache_list_.emplace_front(slow_get_page(key));
    cache_map_[key] = cache_list_.begin();
  }

  void clear_cache() {
    cache_list_.clear();
    cache_map_.clear();
    request_occurrence_map_.clear();
    heap_ = {};
  }

};

}  // namespace caches
