#pragma once
#include <vector>
#include <map>
#include "main.h"
#include <string>
#include <tinyxml.h>
//class Cell holds a single cell.
//
//it's public member variable protein holds the amount of protein currently held for each protein type.
//it can be checked for example like this: CellPointer->protein["cookie"] or CellObject.protein["cookie"], which will return an unsigned integer indicating the amount of cookie protein. Setting works the same: CellObject.protein["cookie"]++, for example, will work as you expect.
//
// The public member variable Degrade holds the degradation factor for all protein types (by default 0)
//
//The public member function Connect will connect this cell to another cell.
//It is overloaded for making multiple connections at the same time.
class Cell{
  private:
  public:
    std::map<Cell*, double> Weight;
//	std::vector<unsigned int> protein;
//	std::vector<string> proteinName;
	std::vector<std::string> protName;
    std::map<std::string, unsigned int> protein;//current protein contents (default all zero)
    std::map<std::string, double> Degrade;//protein degradation values (default all zero)
    std::map<std::string, int> Produce;//protein production amounts (default all zero)
    void Connect(Cell * other, double weight);//expects pointer to an existing cell
    void Connect(unsigned int count, Cell ** other, double weight);//expects count, followed by array of pointers
    Cell();//initializer without location

    //and, to make wouter happy:
    int x, y, z;//coordinates, stored but completely unused in calculations. For easy displaying in GUI.
	int r,g,b;
    Cell(int sx, int sy, int sz);//initializer with location
};//Cell

class PetriPlace{
  public:
    unsigned int id;
    std::string color;
    std::map<std::string, unsigned int> marking;
};

class PetriTrans{
  public:
    unsigned int id;
    std::string guard;
};

enum edgeType{
  EDGE_NORMAL,
  EDGE_READ
};

class PetriEdge{
  public:
    unsigned int id;
    unsigned int source;
    unsigned int target;
    edgeType etype;
    std::string expression;
};

class PetriColor{
  public:
    std::string name;
    std::string type;
    std::string value;
};

//actual petrinet.
//The initializer wants a std::string pointing to a file holding the net's XML, in snoopy format.
//Public function calculatestep will do a single maximally-enabled calculating step
//Cells holds all the cells used in calculations. You will need to .push_back() a few Cells here. For example:
//forcertain (x, y, z)
//  myNet.Cells.push_back(Cell(x, y, z));
//Then display and any needed calculations for it should be fairly straightforward.
class PetriNet{
  public:
    PetriNet(std::string XML);//wants a std::string representation of the net XML, for parsing
    void CalculateStep();//does a single maximally-enabled calculation step
    std::vector<Cell> Cells;//holds all the network's biological cells
    void createNet(int sx, int sy, int sz);
    void createNet(int sx, int sy);
    void createNet(int sx);
    void parseNodes(TiXmlNode * N);
    void parseEdges(TiXmlNode * N);
    void parseMeta(TiXmlNode * N);
    void addPlace(TiXmlNode * N);
    void addTransition(TiXmlNode * N);
    void addEdge(TiXmlNode * N, edgeType E);
    void addColset(TiXmlNode * N);
    void addVariable(TiXmlNode * N);
    void addProteinType(string name);
    void addProduction(string name, int cell, int amount);
    void addDegradation(string name, int percentage);
    void updateColors();
  private:
    bool findInput(unsigned int src, std::string expression, std::map<std::string, std::string> & vars);
    bool isEnabled(unsigned int T);
    void doInput(unsigned int T);
    void doInputOutput(unsigned int T);
    std::map<unsigned int, PetriPlace> places;
    std::map<unsigned int, PetriTrans> transitions;
    std::map<unsigned int, PetriEdge> edges;
    std::map<std::string, PetriColor> colors;
    std::map<std::string, std::string> variables;
    TiXmlDocument myXML;
    //private data not yet done for PetriNet
};//PetriNet
