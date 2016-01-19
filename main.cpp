/// PetriCalc main function.
/// Author: Jaron Viëtor, 2012-2016
/// This code is public domain - do with it what you want. A mention of the original author would be appreciated though.

//main PetriNet library
#include "petricalc.h"
//for std::cerr
#include <iostream>
//for time()
#include <time.h>
//for getpid()
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char ** argv){
  //Initialize the random number generator with the current PID.
  //Comment out if randomness (each run being different) isn't wanted.
  srand(getpid());

  int printcount = 1;
  time_t lastSteps = 0, nextTime = time(0) + 1;
  std::map<std::string, unsigned int> cellnames;
  if (argc < 2){
    std::cerr << "Usage: " << argv[0] << " snoopy_petrinet_filename [print_every_this_many_steps, default 1] [space-seperated list of places to output, by default all]" << std::endl;
    return 1;
  }
  if (argc > 2){
    printcount = atoi(argv[2]);
  }

  std::cerr << "Loading " << argv[1] << "..." << std::endl;
  PetriNet Net(argv[1]);

  if (argc > 3){
    for (int i = 3; i < argc; ++i){
      std::string tmp = argv[i];
      cellnames.insert(std::pair<std::string, unsigned int>(tmp, Net.findPlace(tmp)));
    }
  }

  unsigned int steps = 0;
  Net.printStateHeader(cellnames);
  Net.printState(cellnames);
  while (Net.calculateStep()){
    steps++;
    if (steps % printcount == 0){
      Net.printState(cellnames);
    }
    if (time(0) >= nextTime){
      nextTime = time(0)+1;
      std::cerr << "Calculated " << steps << " steps, " << (steps-lastSteps) << " steps/s..." << std::endl;
      lastSteps = steps;
    }
  }
  
}

