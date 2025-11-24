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
    {
        std::unique_lock<std::mutex> lk(mt);
        queue_.push_back (s);
    }
    isEmpty.notify_all();
  }

  bool pop (State &s)
  {
    {
        std::unique_lock<std::mutex> lk(mt);
        isEmpty.wait(lk, [this]{return (!queue_.empty () || isFinished);});
        if(isFinished && queue_.empty ()) return false;
        s = queue_.front ();
        queue_.pop_front ();
    }
    isEmpty.notify_all();
    return true;
  }
  void setFinished() {
    std::unique_lock<std::mutex> lk(mt);
    isFinished = true;
    isEmpty.notify_all();
  }

private:
  std::deque<State> queue_;
  std::mutex mt;
  std::condition_variable isEmpty;
  bool isFinished = false;
};

#endif // STATEQUEUE_H
