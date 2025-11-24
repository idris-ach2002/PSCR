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
  StateQueue(int nbthread) : active(nbthread) {}

  void push (const State &s)
  {
    std::unique_lock<std::mutex> lk(mt);
    queue_.push_back (s);
    cv.notify_all();
  }

  bool pop (State &s)
  {
    std::unique_lock<std::mutex> lk(mt);
    while(queue_.empty () && !isFinished) {
        active--;
        if(active <= 0) {cvActif.notify_all();}
        cv.wait(lk);
        active++;
    }

    if(queue_.empty () && isFinished) return false;

    s = queue_.front ();
    queue_.pop_front ();
    return true;
  }

  void setFinished() {
    std::unique_lock<std::mutex> lk(mt);
    isFinished = true;
    cv.notify_all();
  }

  void await() {
    std::unique_lock<std::mutex> lk(mt);
    while(active > 0) {
        cvActif.wait(lk);
    }
  }

private:
    std::deque<State> queue_;
    std::mutex mt;
    std::condition_variable cv;
    std::condition_variable cvActif;
    bool isFinished = false;
    int active;
};

#endif // STATEQUEUE_H
