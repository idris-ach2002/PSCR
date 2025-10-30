#ifndef PETRI_H
#define PETRI_H

#include "System.hh"
#include <string>

class PetriNet : public System {

public:
  // Builder API methods
  int addPlace(const std::string& name);              // Returns place index
  int addTransition(const std::string& name);         // Returns transition index
  void addPreArc(int tindex, int pindex, int value = 1);
  void addPostArc(int tindex, int pindex, int value = 1);
  void setInitialTokens(int pindex, int tokens);

  // Factory method to create a System instance
  static System* buildNet(const char * path);

  // Returns the initial state
  State initial_state() const;

  // Generates successors of a given state
  std::vector<State> successors(const State& s) const;


  size_t getPlaceCount() const {
      return place_names.size();
  }

  size_t getTransitionCount() const {
      return transitions.size();
  }
private:
  // Internal representation
  std::vector<std::string> place_names;
  State initial_state_; // Vector of token counts

  // Transition representation
  struct Transition {
      std::string name;
      IntVector pre;  // Pre-condition vector
      IntVector post; // Post-condition vector

      Transition(const std::string& name_, size_t num_places)
          : name(name_), pre(num_places), post(num_places) {}
  };
  std::vector<Transition> transitions;

  // private ctor
  PetriNet() : initial_state_(0) {}
  // parser class is a friend
  friend PetriNet* loadPNML(const std::string& filename);
};

#endif
