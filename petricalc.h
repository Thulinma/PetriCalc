#pragma once
#include <vector>
#include <map>
#include <set>
#include <string>
#include "tinyxml.h"

/// A PetriNet arc - contains the arc label for a PetriNet arc.
/// The range function, effect function and combine function are direct conversions from the range function, effect function and combination operator from Definition 11.
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
///The initializer wants a std::string pointing to a file holding the net's XML, in Snoopy format.
///Public function calculatestep will do a single maximally-enabled calculating step
class PetriNet{
  public:
    PetriNet(std::string XML);/// Parse a std::string representation of the net XML
    bool calculateStep();///< Does a single maximally-enabled calculation step
    void printStateHeader();///< Prints the header for states, seperated by tabs, followed by a newline.
    void printState();///< Prints the current net state, seperated by tabs, followed by a newline.
    void printStateHeader(std::map<std::string, unsigned int> & cellnames);
    void printState(std::map<std::string, unsigned int> & cellnames);
    bool isEnabled(unsigned int T);///< Returns if this transition is enabled
    std::map<unsigned long long, std::string> places;///< Human readable names for places
    std::map<unsigned long long, unsigned long long> marking;///< Markings for places
    std::map<unsigned long long, std::string> transitions;///< Human readable names for transitions
    std::map<unsigned long long, std::map<unsigned long long, PetriArc> > arcs;///<All arcs, in the format: arcs[transition][place]
    unsigned int findPlace(std::string placename);
    void fire(unsigned int T);
private:
    void loadCache();
    void parseNodes(TiXmlNode * N);
    void parseEdges(TiXmlNode * N);
    void parseMeta(TiXmlNode * N);
    void addPlace(TiXmlNode * N);
    void addTransition(TiXmlNode * N);
    void addEdge(TiXmlNode * N, unsigned int E);
    TiXmlDocument myXML;
};//PetriNet

