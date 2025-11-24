// StateQueue.h
#ifndef STATEQUEUE_H
#define STATEQUEUE_H

#include "System.hh"
#include <deque>
#include <mutex>
#include <condition_variable>

class StateQueue
{
public:
  void push (const State &s)
  {
    std::unique_lock<std::mutex> lk(mt);
    queue_.push_back (s);
    cv.notify_all();
  }

  bool pop (State &s)
  {
    std::unique_lock<std::mutex> lk(mt);
    cv.wait(lk, [this]{return !queue_.empty ();});
    s = queue_.front ();
    queue_.pop_front ();
    return true;
  }

private:
    std::deque<State> queue_;
    std::mutex mt;
    std::condition_variable cv;
};

#endif // STATEQUEUE_H
