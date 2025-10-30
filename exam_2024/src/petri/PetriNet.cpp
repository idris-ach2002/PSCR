#include "PetriNet.hh"
#include "PetriNetLoader.hh"

System* PetriNet::buildNet(const char* path) {
  return loadPNML(path);
}

int PetriNet::addPlace(const std::string& name) {
    place_names.push_back(name);
    // Extend initial_state_ to include the new place
    initial_state_.values.push_back(0);

    // Update existing transitions' pre and post vectors
    for (auto& t : transitions) {
        t.pre.values.push_back(0);
        t.post.values.push_back(0);
    }
    return static_cast<int>(place_names.size() - 1); // Return place index
}

int PetriNet::addTransition(const std::string& name) {
    transitions.emplace_back(name, place_names.size());
    return static_cast<int>(transitions.size() - 1); // Return transition index
}

void PetriNet::addPreArc(int tindex, int pindex, int value) {
    transitions[tindex].pre.values[pindex] = value;
}

void PetriNet::addPostArc(int tindex, int pindex, int value) {
    transitions[tindex].post.values[pindex] = value;
}

void PetriNet::setInitialTokens(int pindex, int tokens) {
    initial_state_.values[pindex] = tokens;
}

State PetriNet::initial_state() const {
    return initial_state_;
}

std::vector<State> PetriNet::successors(const State& s) const {
    std::vector<State> successors;
    for (const auto& t : transitions) {
        if (t.pre <= s) {
            State new_state = s - t.pre + t.post;
            successors.push_back(new_state);
        }
    }
    return successors;
}
