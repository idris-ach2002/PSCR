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
    StateQueue(int nbThread) : active(nbThread) {}
    void push(const State &s)
    {
        {
            std::unique_lock<std::mutex> lk(mt);
            queue_.push_back(s);
        }
        isEmpty.notify_all();
    }

    bool pop(State &s)
    {
        {
            std::unique_lock<std::mutex> lk(mt);
            while (queue_.empty() && !isFinished)
            {
                active--;
                if (active == 0)
                {
                    isActif.notify_all();
                }
                isEmpty.wait(lk);
                active++;
            }  
            if (isFinished && queue_.empty())
                return false;
            s = queue_.front();
            queue_.pop_front();
        }
        return true;
    }
    void setFinished()
    {
        std::unique_lock<std::mutex> lk(mt);
        isFinished = true;
        isEmpty.notify_all();
    }

    void await()
    {
        {
            std::unique_lock<std::mutex> lk(mt);
            isActif.wait(lk, [this]
                         { return active <= 0; });
        }
    }

private:
    std::deque<State> queue_;
    std::mutex mt;
    std::condition_variable isEmpty;
    std::condition_variable isActif;
    bool isFinished = false;
    int active;
};

#endif // STATEQUEUE_H
