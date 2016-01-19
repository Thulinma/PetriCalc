/// \file main.cpp
/// \brief PetriCalc main function.
/// \author Jaron Viëtor
/// \date 2012-2016
/// \copyright This code is public domain - do with it what you want. A mention of the original author would be appreciated though.

//main PetriNet library
#include "petricalc.h"
//for std::cerr
#include <iostream>
//for time()
#include <time.h>
//for getpid()
#include <sys/types.h>
#include <unistd.h>

/// \brief Loads a Snoopy XML file and attempts to run a simulation on it.
/// 
/// Usage: PetriCalc snoopy_petrinet_filename [print every this many steps, default 1] [space-separated list of places to output, by default all places]
/// Simulation will stop once no more transitions are enabled, or continue indefinitely if this never happens.
/// \returns 1 on wrong command line options, 0 on simulation completion.
int main(int argc, char ** argv){
  //Initialize the random number generator with the current PID.
  //Comment out if randomness (each run being different) isn't wanted.
  srand(getpid());

  //Parse the command line - whine if it's obviously invalid
  int printcount = 1;
  time_t lastSteps = 0, nextTime = time(0) + 1;
  std::map<std::string, unsigned int> cellnames;
  if (argc < 2){
    std::cerr << "Usage: " << argv[0] << " snoopy_petrinet_filename [print_every_this_many_steps, default 1] [space-separated list of places to output, by default all places]" << std::endl;
    return 1;
  }
  if (argc > 2){
    printcount = atoi(argv[2]);
    if (printcount < 1){
      std::cerr << "print_ever_this_many_steps must be >= 1. Aborting run." << std::endl;
      return 1;
    }
  }

  //Load the net into memory
  std::cerr << "Loading " << argv[1] << "..." << std::endl;
  PetriNet Net(argv[1]);

  //Parse more command line if argument count > 3 (= the places we want to print)
  if (argc > 3){
    for (int i = 3; i < argc; ++i){
      std::string tmp = argv[i];
      cellnames.insert(std::pair<std::string, unsigned int>(tmp, Net.findPlace(tmp)));
    }
  }

  //Print the header for output
  unsigned int steps = 0;
  Net.printStateHeader(cellnames);
  Net.printState(cellnames);
  //While we can complete steps...
  while (Net.calculateStep()){
    //Increase the step counter, print state if wanted
    steps++;
    if (steps % printcount == 0){
      Net.printState(cellnames);
    }
    //Print rough calculation speed approximately once per second
    if (time(0) >= nextTime){
      nextTime = time(0)+1;
      std::cerr << "Calculated " << steps << " steps, " << (steps-lastSteps) << " steps/s..." << std::endl;
      lastSteps = steps;
    }
  }
  //No more steps possible, exit cleanly.
  return 0;
}

