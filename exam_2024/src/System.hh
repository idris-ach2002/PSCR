#ifndef SYSTEM_H
#define SYSTEM_H

#include "State.hh"
#include <vector>
#include <string>

class System
{
public:
  // Returns the initial state
  virtual State initial_state () const = 0;

  // Generates successors of a given state
  virtual std::vector<State> successors (const State &s) const = 0;

public:
  System () {};
  virtual ~System () {};
};

#endif // SYSTEM_H
