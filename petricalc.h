/// \file petricalc.h
/// \brief PetriCalc header file.
/// \author Jaron Viëtor
/// \date 2012-2016
/// \copyright This code is public domain - do with it what you want. A mention of the original author would be appreciated though.

#pragma once
#include <vector>
#include <map>
#include <set>
#include <string>
#include "tinyxml.h"

//DEBUG levels:
// 10 = All load stages at full verbosity
//  9 = Arc combining
//  8 = Range function evaluations
//  5 = Enabled transition counts during stepping
//  4 = Chosen transition during stepping

#define SINGLE_STEP 1 ///< Single step mode
#define CONCUR_STEP 2 ///< Concurrent step mode
#define AUTOCON_STEP 3 ///< Auto-concurrent step mode
#define MAX_CONCUR_STEP 4 ///< Maximally concurrent step mode
#define MAX_AUTOCON_STEP 5 ///< Maximally auto-concurrent step mode


/// Since infinity is not representable as a number, the constant 0xFFFFFFFFFFFFFFFFull is used to represent infinity.
#define INFTY 0xFFFFFFFFFFFFFFFFull

/// \brief A PetriNet arc - contains the arc label for a PetriNet arc.
/// The range function, effect function and combine function are direct conversions from the range function, effect function and combination operator from Definition 11.
class PetriArc{
  public:
    PetriArc();
    PetriArc(unsigned long long rUsed, unsigned long long rLow, unsigned long long rHigh, long long e, bool eSetter);
    unsigned long long rangeUsed; ///< The used range portion of the arc label.
    unsigned long long rangeLow; ///< The low range portion of the arc label.
    unsigned long long rangeHigh; ///< The high range portion of the arc label.
    long long effect; ///< The effect portion of the arc label.
    bool effectSetter; ///> Is the effect a setter?
    long long effectAdded;///> Internal use only: total amount of tokens ever added.
    bool rangeFunction(unsigned long long);
    void effectFunction(unsigned long long &);
    void combine(PetriArc param);
    std::string label();
};

class PetriSuperTrans{
  public:
    bool isEnabled(std::map<unsigned long long, unsigned long long> & marking);
    void combine(std::map<unsigned long long, PetriArc> & addArcs);
    bool isCombinedEnabled(std::map<unsigned long long, PetriArc> & addArcs, std::map<unsigned long long, unsigned long long> & marking);
    std::map<unsigned long long, PetriArc> myArcs;
};

/// \brief A PetriNet calculator.
class PetriNet{
  public:
    PetriNet(std::string XML);
    bool calculateStep(int stepMode);
    void printStateHeader(std::map<std::string, unsigned int> & cellnames);
    void printState(std::map<std::string, unsigned int> & cellnames);
    bool isEnabled(unsigned int T);
    unsigned int findPlace(std::string placename);
private:
    std::map<unsigned long long, std::string> places;///< Human readable names for places
    std::map<unsigned long long, unsigned long long> marking;///< Markings for places
    std::map<unsigned long long, std::string> transitions;///< Human readable names for transitions
    std::map<unsigned long long, std::map<unsigned long long, PetriArc> > arcs;///<All arcs, in the format: arcs[transition][place]
    void parseNodes(TiXmlNode * N);
    void parseEdges(TiXmlNode * N);
    void addPlace(TiXmlNode * N);
    void addTransition(TiXmlNode * N);
    void addEdge(TiXmlNode * N, unsigned int E);
};//PetriNet

