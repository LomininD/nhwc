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

  std::size_t capacity() const { return capacity_; }

  bool is_full() const { return lir_count_ + queue_.size() >= capacity_; }

  bool lookup_lir(const KeyT& key) {
    RecordIt stack_it = find_in_stack(key);
    if (stack_it == stack_.end() || stack_it->status != BlockStatus::kLIR) {
      return false;
    }

    move_to_stack_top(key);
    prune_stack();
    return true;
  }

  bool lookup_hir(const KeyT& key) {
    RecordIt queue_it = find_in_queue(key);
    if (queue_it == queue_.end() || queue_it->status != BlockStatus::kHIR) {
      return false;
    }

    RecordIt stack_it = find_in_stack(key);
    if (stack_it != stack_.end()) {
      move_to_stack_top(key);
      stack_it->status = BlockStatus::kLIR;
      ++lir_count_;

      remove_from_queue(key);
      demote_to_hir();
      prune_stack();
    } else {
      move_to_queue_top(key);
      add_to_stack_top(key, queue_it->data, BlockStatus::kHIR);
      prune_stack();
    }
    return true;
  }

  void insert(const KeyT& key, T page) {
    if (is_full()) {
      evict_lru_hir();
    }

    CacheIt data = add_to_data(std::move(page));

    RecordIt stack_it = find_in_stack(key);
    if (stack_it != stack_.end()) {
      move_to_stack_top(key);
      stack_it->data = data;
      stack_it->status = BlockStatus::kLIR;
      ++lir_count_;
      demote_to_hir();
      prune_stack();
      return;
    }

    if (lir_count_ < lir_max_) {
      add_to_stack_top(key, data, BlockStatus::kLIR);
      ++lir_count_;
      return;
    }

    add_to_stack_top(key, data, BlockStatus::kHIR);
    prune_stack();
    add_to_queue_top(key, data);
  }

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

  RecordIt find_in_stack(const KeyT& key) {
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

  void add_to_stack_top(const KeyT& key, CacheIt data, BlockStatus status) {
    stack_.emplace_front(key, data, status);
    hash_stack_.emplace(key, stack_.begin());
  }

  void add_to_queue_top(const KeyT& key, CacheIt data) {
    queue_.emplace_front(key, data, BlockStatus::kHIR);
    hash_queue_.emplace(key, queue_.begin());
  }

  bool move_to_stack_top(const KeyT& key) {
    RecordIt stack_it = find_in_stack(key);
    if (stack_it == stack_.end()) {
      return false;
    }

    stack_.splice(stack_.begin(), stack_, stack_it);
    return true;
  }

  bool move_to_queue_top(const KeyT& key) {
    RecordIt queue_it = find_in_queue(key);
    if (queue_it == queue_.end()) {
      return false;
    }

    queue_.splice(queue_.begin(), queue_, queue_it);
    return true;
  }

  bool remove_from_queue(const KeyT& key) {
    RecordIt queue_it = find_in_queue(key);
    if (queue_it == queue_.end()) {
      return false;
    }

    hash_queue_.erase(key);
    queue_.erase(queue_it);
    return true;
  }

  void prune_stack() {
    while (!stack_.empty() && stack_.back().status == BlockStatus::kHIR) {
      hash_stack_.erase(stack_.back().key);
      stack_.pop_back();
    }
  }

  void demote_to_hir() {
    if (stack_.empty()) {
      return;
    }

    RecordIt bottom = std::prev(stack_.end());
    bottom->status = BlockStatus::kHIR;
    hash_stack_.erase(bottom->key);
    queue_.splice(queue_.begin(), stack_, bottom);
    hash_queue_.emplace(bottom->key, bottom);
    --lir_count_;
  }

  void evict_lru_hir() {
    if (queue_.empty()) {
      return;
    }

    KeyT key = std::move(queue_.back().key);
    CacheIt data = queue_.back().data;
    hash_queue_.erase(key);
    queue_.pop_back();

    RecordIt stack_it = find_in_stack(key);
    if (stack_it != stack_.end()) {
      stack_it->data = data_.end();
    }
    data_.erase(data);
  }
};

export template <typename KeyT, typename T>
class LIRSCache : public BaseCache<T, KeyT> {
public:
  explicit LIRSCache(std::size_t capacity) : state_(capacity) {}

  bool lookup_update(KeyT key, std::function<T(KeyT)> slow_get_page) {
    if (state_.capacity() == 0) {
      return false;
    }

    if (state_.lookup_lir(key)) {
      return true;
    }

    if (state_.lookup_hir(key)) {
      return true;
    }

    state_.insert(key, slow_get_page(key));
    return false;
  }

private:
  LIRSState<KeyT, T> state_;
};

}  // namespace caches
