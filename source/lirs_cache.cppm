module;

#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <list>
#include <unordered_map>

export module lirs_cache;

import base_cache;

namespace caches {

template <typename KeyT, typename T>
class LIRSState {
public:
  explicit LIRSState(std::size_t capacity) : 
    capacity_(capacity),
    lir_max_(capacity > 0 ? capacity - std::max<std::size_t>(1, capacity / 100) : 0) {}

private:
  std::size_t capacity_;
  std::size_t lir_max_;
  std::size_t lir_count_ = 0;

  std::list<T> data_;
  enum class BlockStatus : bool { kLIR, kHIR };
  using CacheIt = typename std::list<T>::iterator;

  struct Record {
    KeyT key;
    CacheIt data;
    BlockStatus status;

    Record(KeyT k, CacheIt d, BlockStatus s) : key(k), data(d), status(s) {}
  };
  using RecordIt = typename std::list<Record>::iterator;

  std::list<Record> stack_;
  std::unordered_map<KeyT, RecordIt> hash_stack_;

  std::list<Record> queue_;
  std::unordered_map<KeyT, RecordIt> hash_queue_;

  RecordIt find_in_stack(KeyT key) {
    auto hit = hash_stack_.find(key);
    return hit == hash_stack_.end() ? stack_.end() : hit->second;
  }

  RecordIt find_in_queue(KeyT key) {
    auto hit = hash_queue_.find(key);
    return hit == hash_queue_.end() ? queue_.end() : hit->second;
  }

  CacheIt add_to_data(T page) {
    data_.emplace_front(page);
    return data_.begin();
  }

  void add_to_stack_top(KeyT key, CacheIt data, BlockStatus status) {
    stack_.emplace_front(key, data, status);
    hash_stack_.emplace(key, stack_.begin());
  }

  void add_to_queue_top(KeyT key, CacheIt data) {
    queue_.emplace_front(key, data, BlockStatus::kHIR);
    hash_queue_.emplace(key, queue_.begin());
  }

  bool move_to_stack_top(KeyT key) {
    RecordIt stack_it = find_in_stack(key);
    if (stack_it == stack_.end()) return false;
    stack_.splice(stack_.begin(), stack_, stack_it);
    return true;
  }

  bool move_to_queue_top(KeyT key) {
    RecordIt queue_it = find_in_queue(key);
    if (queue_it == queue_.end()) return false;
    queue_.splice(queue_.begin(), queue_, queue_it);
    return true;
  }

  bool remove_from_queue(KeyT key) {
    RecordIt queue_it = find_in_queue(key);
    if (queue_it == queue_.end()) return false;
    hash_queue_.erase(key);
    queue_.erase(queue_it);
    return true;
  }
};

export template <typename KeyT, typename T>
class LIRSCache : public BaseCache<T, KeyT> {
public:
  explicit LIRSCache(std::size_t capacity) :
    capacity_(capacity),
    lirs_max_(capacity > 0 ? capacity - std::max<std::size_t>(1, capacity / 100) : 0) {}

  std::size_t max_capacity() const { return capacity_; }

  bool lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page) {
    if (max_capacity() == 0)
      return false;

    auto hit_stack = hash_stack_.find(key);
    bool in_stack = hit_stack != hash_stack_.end();
    auto hit_queue = hash_queue_.find(key);
    bool in_queue = hit_queue != hash_queue_.end();

    if (in_stack && hit_stack->second->status == BlockStatus::kLIR) {
      stack_.splice(stack_.begin(), stack_, hit_stack->second);
      prune_stack();
      return true;
    }

    if (in_queue) {
      if (in_stack) {
        stack_.splice(stack_.begin(), stack_, hit_stack->second);
        hit_stack->second->status = BlockStatus::kLIR;
        ++lir_count_;
        queue_.erase(hit_queue->second);
        hash_queue_.erase(key);
        demote_lir_bottom();
        prune_stack();
      } else {
        queue_.splice(queue_.begin(), queue_, hit_queue->second);
        stack_.emplace_front(key, hit_queue->second->data, BlockStatus::kHIR);
        hash_stack_.emplace(key, stack_.begin());
        prune_stack();
      }

      return true;
    }

    T page = slow_get_page(key);

    if (!is_full()) {
      cache_.emplace_front(page);

      if (lir_count_ < lirs_max_) {
        stack_.emplace_front(key, cache_.begin(), BlockStatus::kLIR);
        hash_stack_.emplace(key, stack_.begin());
        ++lir_count_;
      } else {
        stack_.emplace_front(key, cache_.begin(), BlockStatus::kHIR);
        hash_stack_.emplace(key, stack_.begin());
        prune_stack();
        queue_.emplace_front(key, cache_.begin());
        hash_queue_.emplace(key, queue_.begin());
      }

      return false;
    }

    evict_lru_hir();
    cache_.emplace_front(page);

    if (in_stack) {
      stack_.splice(stack_.begin(), stack_, hit_stack->second);
      hit_stack->second->data = cache_.begin();
      hit_stack->second->status = BlockStatus::kLIR;
      ++lir_count_;

      demote_lir_bottom();
      prune_stack();
    } else {
      stack_.emplace_front(key, cache_.begin(), BlockStatus::kHIR);
      hash_stack_.emplace(key, stack_.begin());
      prune_stack();
      queue_.emplace_front(key, cache_.begin());
      hash_queue_.emplace(key, queue_.begin());
    }
    return false;
  }

private:
  std::size_t capacity_, lirs_max_;
  std::size_t lir_count_ = 0;

  std::list<T> cache_;
  enum class BlockStatus { kLIR, kHIR };
  using CacheIt = typename std::list<T>::iterator;

  struct StackRecord {
    KeyT key;
    CacheIt data;
    BlockStatus status;

    StackRecord(KeyT k, CacheIt d, BlockStatus s) : key(k), data(d), status(s) {}
  };

  struct QueueRecord {
    KeyT key;
    CacheIt data;

    QueueRecord(KeyT k, CacheIt d) : key(k), data(d) {}
  };

  using StackIt = typename std::list<StackRecord>::iterator;
  using QueueIt = typename std::list<QueueRecord>::iterator;

  std::list<StackRecord> stack_;
  std::list<QueueRecord> queue_;
  std::unordered_map<KeyT, StackIt> hash_stack_;
  std::unordered_map<KeyT, QueueIt> hash_queue_;

  void prune_stack() {
    while (!stack_.empty() && stack_.back().status == BlockStatus::kHIR) {
      hash_stack_.erase(stack_.back().key);
      stack_.pop_back();
    }
  }

  void demote_lir_bottom() {
    if (stack_.empty())
      return;

    StackRecord bottom = stack_.back();
    stack_.pop_back();
    hash_stack_.erase(bottom.key);
    bottom.status = BlockStatus::kHIR;
    queue_.emplace_front(bottom.key, bottom.data);
    hash_queue_.emplace(bottom.key, queue_.begin());
    --lir_count_;
  }

  bool is_full() const { return lir_count_ + queue_.size() >= capacity_; }

  void evict_lru_hir() {
    if (queue_.empty())
      return;

    QueueRecord victim = queue_.back();
    queue_.pop_back();
    hash_queue_.erase(victim.key);

    auto it = hash_stack_.find(victim.key);
    if (it != hash_stack_.end())
      it->second->data = cache_.end();

    cache_.erase(victim.data);
  }
};

}  // namespace caches
