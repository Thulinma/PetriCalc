#include "petricalc.h"
#include <iostream>
#include <time.h>

int main(int argc, char ** argv){
  int printcount = 1;
  unsigned int lastSteps = 0, nextTime = time(0) + 1;
  std::map<std::string, unsigned int> cellnames;
  if (argc < 2){
    std::cerr << "Usage: " << argv[0] << " snoopy_petrinet_filename [print_every_this_many_steps, default 1] [space-seperated list of cellnames to output, by default all]" << std::endl;
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
  if (cellnames.size() == 0){
    Net.PrintStateHeader();
    Net.PrintState();
  }else{
    Net.PrintStateHeader(cellnames);
    Net.PrintState(cellnames);
  }
  while (Net.CalculateStep()){
    steps++;
    if (steps % printcount == 0){
      if (cellnames.size() == 0){Net.PrintState();}else{Net.PrintState(cellnames);}
    }
    if (time(0) >= nextTime){
      nextTime = time(0)+1;
      std::cerr << "Calculated " << steps << " steps, " << (steps-lastSteps) << " steps/s..." << std::endl;
      lastSteps = steps;
    }
  }
  
  
}
