// StateQueue.h
#ifndef STATEQUEUE_H
#define STATEQUEUE_H

#include "System.hh"
#include <deque>

class StateQueue
{
public:
  void push (const State &s)
  {
    queue_.push_back (s);
  }

  bool pop (State &s)
  {
    if (!queue_.empty ()) {
      s = queue_.front ();
      queue_.pop_front ();
      return true;
    }
    return false;
  }

private:
  std::deque<State> queue_;
};

#endif // STATEQUEUE_H
