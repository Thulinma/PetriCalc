/// PetriCalc header file.
/// Author: Jaron Viëtor, 2012-2016
/// This code is public domain - do with it what you want. A mention of the original author would be appreciated though.

#pragma once
#include <vector>
#include <map>
#include <set>
#include <string>
#include "tinyxml.h"

//DEBUG levels:
// 10 = All load stages at full verbosity
//  9 = Arc combining
//  5 = Enabled transition counts during stepping
//  4 = Chosen transition during stepping


// Infinity and negative infinity are represented by chosen constants.
#define INFTY 0xFFFFFFFFFFFFFFFFull
#define NEGTY -0xFFFFFFFFFFFFFFll

/// A PetriNet arc - contains the arc label for a PetriNet arc.
/// The range function, effect function and combine function are direct conversions from the range function, effect function and combination operator from Definition 11.
/// Since infinity is not representable as a number, the constant 0xFFFFFFFFFFFFFFFFull is used to represent infinity.
class PetriArc{
  public:
    PetriArc();
    PetriArc(unsigned long long rLow, unsigned long long rHigh, long long e);
    unsigned long long rangeLow;
    unsigned long long rangeHigh;
    long long effect;
    bool rangeFunction(unsigned long long);
    void effectFunction(unsigned long long &);
    void combine(PetriArc param);
};

enum calcType{
  CALC_FULL,
  CALC_MIDSTEP
};

///A PetriNet calculator.
class PetriNet{
  public:
    PetriNet(std::string XML);
    bool calculateStep();
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

