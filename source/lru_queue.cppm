module;

#include <cstddef>
#include <list>
#include <optional>
#include <unordered_map>

export module lru_queue;

namespace caches {

template <typename KeyT, typename QueueItemT = KeyT>
class BaseLRUQueue {
protected:
  std::list<QueueItemT> cache_;

  using QueueIt = typename std::list<QueueItemT>::iterator;
  std::unordered_map<KeyT, QueueIt> hash_;

  virtual KeyT get_key(QueueItemT& item) = 0;

  virtual ~BaseLRUQueue() = default;
public:
  std::size_t size() const { return cache_.size(); }

  bool empty() const { return size() == 0; }
  bool has(KeyT key) const { return hash_.find(key) != hash_.end(); }

  bool lookup(KeyT key) {
    auto hit = hash_.find(key);
    if (hit != hash_.end()) {
      auto eltit = hit->second;
      cache_.splice(cache_.begin(), cache_, eltit);
      return true;
    }
    return false;
  }

  void insert(QueueItemT item) {
    cache_.emplace_front(item);
    hash_.emplace(get_key(item), cache_.begin());
  }

  std::optional<QueueItemT> pop_most_recently_used() {
    if (empty())
      return std::nullopt;

    hash_.erase(get_key(cache_.front()));
    auto item = cache_.front();
    cache_.pop_front();

    return item;
  }

  std::optional<QueueItemT> pop_last_recently_used() {
    if (empty())
      return std::nullopt;

    hash_.erase(get_key(cache_.back()));
    auto item = cache_.back();
    cache_.pop_back();

    return item;
  }

  void erase(KeyT key) {
    auto hit = hash_.find(key);
    if (hit == hash_.end())
      return;
    auto eltit = hit->second;
    cache_.erase(eltit);
    hash_.erase(hit);
  }
};

template <typename KeyT, typename T>
struct LRUQueueItem {
  KeyT key;
  T page;
};

export template <typename KeyT, typename T>
class LRUQueue : public BaseLRUQueue<KeyT, LRUQueueItem<KeyT, T>> {
  using QueueItem = LRUQueueItem<T, KeyT>;
  KeyT get_key(QueueItem& item) { return item.key; }
};

export template <typename KeyT = int>
class GhostLRUQueue : public BaseLRUQueue<KeyT, KeyT> {
  KeyT get_key(KeyT& item) { return item; }
};

}  // namespace caches
