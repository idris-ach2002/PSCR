// StateSet.h
#ifndef STATESET_H
#define STATESET_H

#include "State.hh"
#include <vector>
#include <mutex>
#include <atomic>
#include <forward_list>

class StateSet
{
public:
  StateSet (size_t num_buckets = 1024)
      : buckets_ (num_buckets), size_ (0), mutexs(num_buckets)
  {
  }

  bool add (const State &s)
  {
    size_t index = s.hash () % buckets_.size ();
    std::forward_list<State> &bucket = buckets_[index];
    {
        std::unique_lock<std::mutex> lk(mutexs[index]);
        for (const State &state_in_bucket : bucket) {
            if (state_in_bucket == s) {
                return false; // State already in set
            }
        }
        bucket.push_front (s);
    }
    ++size_;
    return true;
  }

  size_t size () const
  {
    return size_;
  }

private:
  std::vector<std::forward_list<State>> buckets_;
  std::atomic<size_t> size_;
  std::vector<std::mutex> mutexs;
};

#endif // STATESET_H
