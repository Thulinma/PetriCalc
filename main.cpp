/// \file main.cpp
/// \brief PetriCalc main function.
/// \author Jaron Viëtor
/// \date 2012-2016
/// \copyright This code is public domain - do with it what you want. A mention of the original author would be appreciated though.

#include "petricalc.h" //main PetriNet library
#include <iostream> //for std::cerr
#include <string> //for std::string
#include <time.h> //for time()
#include <sys/types.h> //for getpid()
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
  int stepmode = SINGLE_STEP;
  time_t lastSteps = 0, startTime = time(0), lastTime = time(0);
  std::map<std::string, unsigned int> cellnames;
  if (argc < 2){
    std::cerr << "Usage: " << argv[0] << " snoopy_petrinet_filename [[[steptype=single [print_interval=1] space_separated_list_of_places_to_output=all ...]" << std::endl;
    return 1;
  }
  
  if (argc > 2){
    stepmode = 0;
    std::string newMode = argv[2];
    if (newMode == "single"){stepmode = SINGLE_STEP;}
    if (newMode == "concurrent"){stepmode = CONCUR_STEP;}
    if (newMode == "autoconcurrent"){stepmode = AUTOCON_STEP;}
    if (newMode == "maxconcurrent"){stepmode = MAX_CONCUR_STEP;}
    if (newMode == "maxautoconcurrent"){stepmode = MAX_AUTOCON_STEP;}
    if (!stepmode){
      std::cerr << "steptype must be one of: single, concurrent, autoconcurrent, maxconcurrent, maxautoconcurrent. Aborting." << std::endl;
      return 1;
    }
  }

  std::cerr << "Step mode: ";
  switch (stepmode){
    case SINGLE_STEP: std::cerr << "single stepping"; break;
    case CONCUR_STEP: std::cerr << "concurrent stepping"; break;
    case AUTOCON_STEP: std::cerr << "auto-concurrent stepping"; break;
    case MAX_CONCUR_STEP: std::cerr << "maximally concurrent stepping"; break;
    case MAX_AUTOCON_STEP: std::cerr << "maximally auto-concurrent stepping"; break;
  }
  std::cerr << std::endl;
  
  if (argc > 3){
    printcount = atoi(argv[3]);
    if (printcount < 1){
      std::cerr << "print_interval must be >= 1. Aborting." << std::endl;
      return 1;
    }
  }

  //Load the net into memory
  std::cerr << "Loading " << argv[1] << "..." << std::endl;
  PetriNet Net(argv[1]);

  //Parse more command line if argument count > 4 (= the places we want to print)
  if (argc > 4){
    for (int i = 4; i < argc; ++i){
      std::string tmp = argv[i];
      cellnames.insert(std::pair<std::string, unsigned int>(tmp, Net.findPlace(tmp)));
    }
  }

  //Print the header for output
  unsigned int steps = 0;
  Net.printStateHeader(cellnames);
  Net.printState(cellnames);
  //While we can complete steps...
  while (Net.calculateStep(stepmode)){
    //Increase the step counter, print state if wanted
    steps++;
    if (steps % printcount == 0){
      Net.printState(cellnames);
    }
    //Print rough calculation speed approximately once per second
    time_t now = time(0);
    if (now > lastTime){
      std::cerr << "Calculated " << steps << " steps, avg: " << (steps/(double)(now-startTime)) << "s/s, cur:" << (steps-lastSteps)/(double)(now-lastTime) << " s/s..." << std::endl;
      lastTime = now;
      lastSteps = steps;
    }
  }
  //No more steps possible, exit cleanly.
  return 0;
}

