// main.cpp
#include "System.hh"
#include "petri/PetriNet.hh"
#include "ConcurrentStateQueue.hh"
#include "ConcurrentStateSet.hh"
#include <iostream>
#include <chrono>
#include <cstdlib>
#include <vector>
#include <thread>

int main(int argc, const char **argv)
{
  // Check if the user has provided a PNML file
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " <pnml file>" << std::endl;
    return 1;
  }
  // Build the system using the builder API
  System *system = PetriNet::buildNet(argv[1]);
  int n = atoi(argv[2]);
  std::vector<std::thread> threads;
  threads.reserve(n);

  auto start_time = std::chrono::steady_clock::now();

  State initial = system->initial_state();

  // Data structures
  StateQueue todo(n);
  // ~ 2 Million buckets
  StateSet seen(2 << 21);

  // Initialize
  todo.push(initial);
  seen.add(initial);

  for (int i = 0; i < n; i++)
  {

    threads.emplace_back([&]()
                         {
      State s;
      while (todo.pop (s)) {
        std::vector<State> successors = system->successors (s);
        for (const State &succ : successors) {
          if (seen.add (succ)) {
            todo.push (succ);
          }
        }
      } });
  }

  todo.await();
  todo.setFinished();

  for (auto &th : threads)
  {
    th.join();
  }

  auto end_time = std::chrono::steady_clock::now();
  auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                            end_time - start_time)
                            .count();

  std::cout << "Total states explored: " << seen.size() << " in "
            << total_duration << " ms." << std::endl;

  delete system;
  return 0;
}
