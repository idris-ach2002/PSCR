// StateSet.h
#ifndef STATESET_H
#define STATESET_H

#include "State.hh"
#include <vector>
#include <mutex>
#include <forward_list>

class StateSet
{
public:
  StateSet (size_t num_buckets = 1024)
      : buckets_ (num_buckets), size_ (0)
  {
  }

  bool add (const State &s)
  {
    std::unique_lock<std::mutex> lk(mt);
    size_t index = s.hash () % buckets_.size ();
    std::forward_list<State> &bucket = buckets_[index];
    for (const State &state_in_bucket : bucket) {
      if (state_in_bucket == s) {
        return false; // State already in set
      }
    }
    bucket.push_front (s);
    ++size_;
    return true;
  }

  size_t size () const
  {
    std::unique_lock<std::mutex> lk(mt);
    return size_;
  }

private:
  std::vector<std::forward_list<State>> buckets_;
  size_t size_;
  std::mutex mt;
};

#endif // STATESET_H
