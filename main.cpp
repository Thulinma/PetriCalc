#include "petricalc.h"
#include <iostream>
#include <time.h>

int main(int argc, char ** argv){

  srand(time(0));

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
  std::cerr << "Loaded " << argv[1] << "!" << std::endl;

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

