#pragma once
#include <vector>
#include <map>
#include <set>
#include <string>
#include "tinyxml.h"

/// A PetriNet Place - holds a single place's state information.
class PetriPlace{
  public:
    unsigned int id; ///< Internal ID
    std::string name; ///< Human-readable name
    std::string color; ///< Colorset
    unsigned long long int iMarking; ///< Integer marking
    std::map<std::string, unsigned int> marking; ///< Colored marking
};

/// A PetriNet Transition - holds a single transitions state information.
class PetriTrans{
  public:
    unsigned int id;
    std::string name;
    std::string guard;
    std::set<unsigned int> inputs;
    std::set<unsigned int> outputs;
    std::set<unsigned int> conflicts_with;
};

/// Contains possible edge types.
enum edgeType{
  EDGE_NORMAL,
  EDGE_ACTIVATOR,
  EDGE_INHIBITOR,
  EDGE_RESET,
  EDGE_EQUAL
};

enum calcType{
  CALC_FULL,
  CALC_MIDSTEP
};

/// A PetriNet Edge - holds a single edge's state information.
class PetriEdge{
  public:
    unsigned int id;
    unsigned int source;
    unsigned int target;
    edgeType etype;
    unsigned int multiplicity;
    std::string expression;
};

/// A PetriNet Color - holds information about possible colors in a net.
class PetriColor{
  public:
    std::string name;
    std::string type;
    std::string value;
};

///A PetriNet calculator.
///The initializer wants a std::string pointing to a file holding the net's XML, in Snoopy format.
///Public function calculatestep will do a single maximally-enabled calculating step
class PetriNet{
  public:
    PetriNet(std::string XML);/// Parse a std::string representation of the net XML
    bool CalculateStep();///< Does a single maximally-enabled calculation step
    void PrintStateHeader();///< Prints the header for states, seperated by tabs, followed by a newline.
    void PrintState();///< Prints the current net state, seperated by tabs, followed by a newline.
    void PrintStateHeader(std::map<std::string, unsigned int> & cellnames);
    void PrintState(std::map<std::string, unsigned int> & cellnames);
    unsigned int isEnabled(unsigned int T, calcType C = CALC_FULL);///< Returns how many times this transition is enabled.
    std::map<unsigned int, PetriPlace> places;
    std::map<unsigned int, PetriTrans> transitions;
    std::map<unsigned int, PetriEdge> edges;
    std::map<std::string, PetriColor> colors;
    std::map<std::string, std::string> variables;
    unsigned int findPlace(std::string placename);
    bool conflicts(unsigned int transition, std::set<unsigned int> & checked_transitions);
private:
    void LoadCache();
    void parseNodes(TiXmlNode * N);
    void parseEdges(TiXmlNode * N);
    void parseMeta(TiXmlNode * N);
    void addPlace(TiXmlNode * N);
    void addTransition(TiXmlNode * N);
    void addEdge(TiXmlNode * N, edgeType E);
    void addColset(TiXmlNode * N);
    void addVariable(TiXmlNode * N);
    bool findInput(unsigned int src, std::string expression, std::map<std::string, std::string> & vars);
    void doInput(unsigned int T, unsigned int cnt);
    void doInputOutput(unsigned int T, unsigned int cnt);
    TiXmlDocument myXML;
};//PetriNet
