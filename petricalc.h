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
//  5 = Enabled transition counts during stepping
//  4 = Chosen transition during stepping


/// Since infinity is not representable as a number, the constant 0xFFFFFFFFFFFFFFFFull is used to represent infinity.
#define INFTY 0xFFFFFFFFFFFFFFFFull
/// Since negative infinity is not representable as a number, the constant -0xFFFFFFFFFFFFFFll is used to represent negative infinity.
#define NEGTY -0xFFFFFFFFFFFFFFll

/// \brief A PetriNet arc - contains the arc label for a PetriNet arc.
/// The range function, effect function and combine function are direct conversions from the range function, effect function and combination operator from Definition 11.
class PetriArc{
  public:
    PetriArc();
    PetriArc(unsigned long long rLow, unsigned long long rHigh, long long e);
    unsigned long long rangeLow; ///< The low range portion of the arc label.
    unsigned long long rangeHigh; ///< The high range portion of the arc label.
    long long effect; ///< The effect portion of the arc label.
    bool rangeFunction(unsigned long long);
    void effectFunction(unsigned long long &);
    void combine(PetriArc param);
};

/// \brief A PetriNet calculator.
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

