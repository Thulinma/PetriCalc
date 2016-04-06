/// \file petricalc.cpp
/// \brief PetriCalc implementation.
/// \author Jaron Viëtor
/// \date 2012-2016
/// \copyright This code is public domain - do with it what you want. A mention of the original author would be appreciated though.

#include "petricalc.h"
#include <deque>
#include <sstream>
#include <algorithm>
#include <iostream>

/// \brief Base constructor will create a No-Operation arc ((0, 0, inf), 0).
PetriArc::PetriArc(){
  rangeUsed = 0;
  rangeLow = 0;
  rangeHigh = INFTY;
  effect = 0;
}

/// \brief Fancy constructor will create a given arc labeled as ((rLow, rHigh), e).
PetriArc::PetriArc(unsigned long long rUsed, unsigned long long rLow, unsigned long long rHigh, long long e, bool eSetter){
  rangeUsed = rUsed;
  rangeLow = rLow;
  rangeHigh = rHigh;
  effect = e;
  effectAdded = 0;
  if (effect > 0){effectAdded = effect;}
  effectSetter = eSetter;
}

/// \brief The range function from definition 8.
/// 
/// When called, true or false is returned to indicate if this arc can enable connected transitions (true) or not (false).
bool PetriArc::rangeFunction(unsigned long long m){
  // From definition 8: fr ((l, h), m) = true if l ≤ m ≤ h and u ≤ v, false otherwise
#if DEBUG >= 8
  std::cerr << "Arc " << label() << " is " << ((rangeLow <= m && m <= rangeHigh && rangeUsed <= m)?"enabled":"disabled") << " with " << m << " tokens" << std::endl;
#endif
  return (rangeLow <= m && m <= rangeHigh && rangeUsed <= m);
}

/// \brief The effect function from definition 11.
/// 
/// When called, the effect is applied to the given unsigned long long value by reference.
void PetriArc::effectFunction(unsigned long long & m){
  // From definition 8: fe (e, m) = e + m
  if (effectSetter){
    m = effect;
  }else{
    m += effect;
  }
}

/// \brief Return a human-readable printed arc label.
std::string PetriArc::label(){
  std::stringstream out;
  out << "((" << rangeUsed << ", " << rangeLow << ", ";
  if (rangeHigh == INFTY){
    out << "Infty";
  }else{
    out << rangeHigh;
  }
  out << "), (" << (effectSetter?"S":"") << effect << ", +" << effectAdded << "))";
  return out.str();
}


/// \brief The combination operator from definition 8.
/// 
/// When called, this PetriArc and given PetriArc are combined into this PetriArc (irreversibly).
void PetriArc::combine(PetriArc param){
  // From definition 8: ⊗(((u1, l1 , h1 ), e1 ), ((u2, l2 , h2 ), e2 )) = (u1 + u2, max(l1, l2), min(h1 , h2)), e1 + e2 )
  #if DEBUG >= 9
  std::cerr << " (" << label() << " COMB " << param.label() << ") = ";
  #endif 
  //Set u to the sum of u1 and u2
  rangeUsed += param.rangeUsed;
  //Set l to the maximum of l1 and l2
  if (param.rangeLow > rangeLow){rangeLow = param.rangeLow;}
  //Set h to the minimum of h1 and h2
  if (param.rangeHigh < rangeHigh){rangeHigh = param.rangeHigh;}
  //Set e to the sum of e1 and e2
  effectAdded += param.effectAdded;
  if (!effectSetter && !param.effectSetter){
    effect += param.effect;
  }else{
    effectSetter = true;
    effect = effectAdded;
  }
  #if DEBUG >= 9
  std::cerr << label() << std::endl;
  #endif 
}

/// \brief Constructor that parses a std::string containing Snoopy XML into a PetriNet.
/// 
/// It does this by checking if nodeclasses and edgeclasses entries are present, and if so, feeds those to parseNodes respectively parseEdges.
/// All other contents of the net are ignored.
PetriNet::PetriNet(std::string XML){
  TiXmlDocument myXML = TiXmlDocument(XML);
  if (!myXML.LoadFile()){
    fprintf(stderr, "Error: Could not read file %s\n", XML.c_str());
    exit(42);
  }
  TiXmlElement * e = myXML.RootElement();
  TiXmlNode * c = e->FirstChild("nodeclasses");
  if (!c){
    fprintf(stderr, "Error: Parsed file is not a valid snoopy petri net\n");
    exit(42);
  }
  parseNodes(c);
  c = e->FirstChild("edgeclasses");
  if (!c){
    fprintf(stderr, "Error: Parsed file is not a valid snoopy petri net\n");
    exit(42);
  }
  parseEdges(c);
};

/// \brief Parses all node types from a Snoopy XML file and calls addPlace or addTransition on all places respectively transitions found in the file.
void PetriNet::parseNodes(TiXmlNode * N){
  TiXmlNode * c = 0, * d = 0;
  TiXmlElement * e;
  while ((c = N->IterateChildren(c))){
    e = c->ToElement();
    if (!e){continue;}
    if (!e->Attribute("name")){return;}
    std::string name = e->Attribute("name");
    if (name == "Place"){
      d = 0;
      while ((d = c->IterateChildren(d))){addPlace(d);}
      fprintf(stderr, "Loaded %u places\n", (unsigned int)places.size());
    }
    if (name == "Transition"){
      d = 0;
      while ((d = c->IterateChildren(d))){addTransition(d);}
      fprintf(stderr, "Loaded %u transitions\n", (unsigned int)transitions.size());
    }
    //other types not supported yet
  }
}

/// \brief Adds a single place to the net from a Snoopy XML file.
/// 
/// Since in our model places are nothing more than labels, this means creating a new entry in the place ID to place name map.
/// Additionally, an entry in the place ID to marking map is made.
/// If the new place has no name, it's given the name "place_" followed by the ID, instead. Thus all places are guaranteed to have a name.
void PetriNet::addPlace(TiXmlNode * N){
  TiXmlElement * e;
  e = N->ToElement();
  const char * id = e->Attribute("id");
  if (!id){return;}
  unsigned long long ID = atoi(id);
  TiXmlNode * c = 0;
  while ((c = N->IterateChildren(c))){
    e = c->ToElement();
    if (!e){continue;}
    if (!e->Attribute("name")){continue;}
    std::string name = e->Attribute("name");
    if (name == "Name"){
      places[ID] = e->GetText();
    }
    if (name == "ID"){
      if (places[ID] == ""){
        places[ID] = std::string("place_")+e->GetText();
      }
    }
    if (name == "Marking"){
      marking[ID] = atoi(e->GetText());
    }
  }
  #if DEBUG >= 10
  std::cerr << "Added place " << places[ID] << " with " << marking[ID] << " tokens" << std::endl;
  #endif
}

/// \brief Adds a single transition to the net from a Snoopy XML file.
/// 
/// Since in our model transitions are nothing more than labels, this means creating a new entry in the transition ID to transition name map.
/// If the new transition has no name, it's given the name "trans_" followed by the ID, instead. Thus all transitions are guaranteed to have a name.
void PetriNet::addTransition(TiXmlNode * N){
  TiXmlElement * e;
  e = N->ToElement();
  const char * id = e->Attribute("id");
  if (!id){return;}
  unsigned long long ID = atoi(id);
  TiXmlNode * c = 0;
  while ((c = N->IterateChildren(c))){
    e = c->ToElement();
    if (!e){continue;}
    if (!e->Attribute("name")){continue;}
    std::string name = e->Attribute("name");
    if (name == "Name"){
      transitions[ID] = e->GetText();
    }
    if (name == "ID"){
      if (transitions[ID] == ""){
        transitions[ID] = std::string("trans_")+e->GetText();
      }
    }
  }
  #if DEBUG >= 10
  std::cerr << "Added transition " << transitions[ID] << std::endl;
  #endif
}

/// Edge types. Only used internally, and only inside parseEdges and addEdge.
enum edgeType{
  EDGE_NORMAL,
  EDGE_ACTIVATOR,
  EDGE_INHIBITOR,
  EDGE_RESET,
  EDGE_EQUAL
};

/// \brief Parses all edge types from a Snoopy XML file and calls addEdge for each edge found in the file.
void PetriNet::parseEdges(TiXmlNode * N){
  TiXmlNode * c = 0, * d = 0;
  TiXmlElement * e;
  while ((c = N->IterateChildren(c))){
    e = c->ToElement();
    if (!e){continue;}
    if (!e->Attribute("name")){return;}
    std::string name = e->Attribute("name");
    if (name == "Edge"){
      d = 0;
      while ((d = c->IterateChildren(d))){addEdge(d, EDGE_NORMAL);}
    }
    if (name == "Read Edge"){
      d = 0;
      while ((d = c->IterateChildren(d))){addEdge(d, EDGE_ACTIVATOR);}
    }
    if (name == "Inhibitor Edge"){
      d = 0;
      while ((d = c->IterateChildren(d))){addEdge(d, EDGE_INHIBITOR);}
    }
    if (name == "Reset Edge"){
      d = 0;
      while ((d = c->IterateChildren(d))){addEdge(d, EDGE_RESET);}
    }
    if (name == "Equal Edge"){
      d = 0;
      while ((d = c->IterateChildren(d))){addEdge(d, EDGE_EQUAL);}
    }
    //other types not supported yet
  }
}

/// \brief Adds a single arc to the net, from a Snoopy XML file.
/// 
/// This function combines arc labels using the combination operator if an arc between the same place and transition already exists.
/// The result of this is that arc labels never need be combined later, as they have been combined right here during net load already.
void PetriNet::addEdge(TiXmlNode * N, unsigned int E){
  TiXmlElement * e;
  e = N->ToElement();
  const char * id = e->Attribute("id");
  if (!id){return;}
  unsigned long long SOURCE = atoi(e->Attribute("source"));
  unsigned long long TARGET = atoi(e->Attribute("target"));
  unsigned long long transition;
  unsigned long long place;
  long long multiplicity = 1;
  if (transitions.count(SOURCE)){
    transition = SOURCE;
    place = TARGET;
  }
  if (places.count(SOURCE)){
    transition = TARGET;
    place = SOURCE;
  }
  TiXmlNode * c = 0;
  while ((c = N->IterateChildren(c))){
    e = c->ToElement();
    if (!e){continue;}
    if (!e->Attribute("name")){continue;}
    std::string name = e->Attribute("name");
    if (name == "Multiplicity"){
      multiplicity = atoi(e->GetText());
    }
  }

  //If the place is the source, everything is negative
  if (place == SOURCE){
    multiplicity *= -1;
  }
  
  unsigned long long aRLow, aRHigh, aRUsed;
  long long aEffect;
  bool aEffectSetter = false;

  if (E == EDGE_NORMAL){
    if (multiplicity < 0){
      aRLow = -multiplicity;
    }else{
      aRLow = 0;
    }
    aRUsed = aRLow;
    aRHigh = INFTY;
    aEffect = multiplicity;
  }
  if (E == EDGE_ACTIVATOR){
    if (multiplicity < 0){
      aRLow = -multiplicity;
    }else{
      aRLow = multiplicity;
    }
    aRUsed = 0;
    aRHigh = INFTY;
    aEffect = 0;
  }
  if (E == EDGE_INHIBITOR){
    if (multiplicity < 0){
      aRHigh = -multiplicity - 1;
    }else{
      aRHigh = multiplicity - 1;
    }
    aRUsed = 0;
    aRLow = 0;
    aEffect = 0;
  }
  if (E == EDGE_EQUAL){
    if (multiplicity < 0){
      aRLow = -multiplicity;
      aRHigh = -multiplicity;
    }else{
      aRLow = multiplicity;
      aRHigh = multiplicity;
    }
    aRUsed = 0;
    aEffect = 0;
  }
  if (E == EDGE_RESET){
    aRLow = 0;
    aRUsed = 0;
    aRHigh = INFTY;
    aEffect = 0;
    aEffectSetter = true;
  }

  if (arcs.count(transition) && arcs[transition].count(place)){
    #if DEBUG >= 9
    std::cerr << "Combining arc " << transitions[transition] << "<->" << places[place] << ": ";
    #endif
    arcs[transition][place].combine(PetriArc(aRUsed, aRLow, aRHigh, aEffect, aEffectSetter));
  }else{
    arcs[transition][place] = PetriArc(aRUsed, aRLow, aRHigh, aEffect, aEffectSetter);
    #if DEBUG >= 10
    std::cerr << "Inserting arc " << transitions[transition] << "<->" << places[place] << ": " << arcs[transition][place].label() << std::endl;
    #endif
  }
}

/// \brief Does a single calculation step, following the method given in definition 8.
/// 
/// Returns true if a step was completed, false if no more transitions are enabled.
bool PetriNet::calculateStep(int stepMode){
  // Definition 8: To calculate a single transition step for a given marked Petri net N = ((P, T, A), (D, fr , fe, L, ⊗, I), M ), do the following:
  // - Create a list E of all enabled transitions, using the method described in Definition 5 to determine enabledness for all t ∈ T .
  // - Pick any one transition t ∈ E and fire it using the method described in Definition 6.

  std::map<unsigned long long, std::map<unsigned long long, PetriArc> >::iterator T;
  std::map<unsigned long long, PetriArc>::iterator A;
  std::set<unsigned long long> enabled; //Enabled transitions
  std::set<unsigned long long>::iterator selector;
  
  //Every transition is checked for enabledness, and made part of a subset consisting of only enabled transitions.
  for (T = arcs.begin(); T != arcs.end(); T++){
    if (isEnabled(T->first)){enabled.insert(T->first);}
  }
  #if DEBUG >= 5
  fprintf(stderr, "Stepping: %u transitions enabled\n", (unsigned int)enabled.size());
  #endif
  //Nothing enabled? We're done. Cancel running net.
  if (enabled.size() == 0){return false;}

  selector = enabled.begin();
  std::advance(selector, rand() % enabled.size());


  #if DEBUG >= 4
  fprintf(stderr, "Stepping: picked transition %s\n", transitions[*selector].c_str());
  #endif

  // Definition 6 In a marked Petri net N = ((P, T, A), (D, fr , fe , L, ⊗, I), M ) the firing of a transition t ∈ T is changing M into M 0 so that:
  // - ∀p ∈ P such that p ‡ t, M 0(p) = fE (aE , M (p))
  // - ∀p ∈ P such that p † t, M 0(p) = M (p)
  // Where a is the pt-combined arc label.

  //Run the effect function on each arc of the chosen transition.
  //We do not calculate the pt-combined arc label here, since it's been pre-calculated during net load already for each transition
  std::map<unsigned long long, PetriArc> & selected = arcs[*selector];
  for (A = selected.begin(); A != selected.end(); A++){
    A->second.effectFunction(marking[A->first]);
  }
  
  //Step completed.
  return true;
}

/// \brief Returns true if the given transition ID is enabled, false otherwise.
/// 
/// Follows definition 5 for deciding if the transition is enabled or not.
bool PetriNet::isEnabled(unsigned int T){
// Definition 5: In a marked Petri net N = ((P, T, A), (D, fr , fe , L, ⊗, I), M ) a transition t ∈ T is enabled when for all p ∈ P such that p‡t, fR(aR , M (p)) = true, where a is the pt-combined arc label.

  //We consider transitions without arcs to not be enabled, since that is the only thing that makes sense.
  if (!arcs[T].size()){
    return false;
  }

  // Loop over all p ∈ P such that p‡t
  std::map<unsigned long long, PetriArc>::iterator A;
  for (A = arcs[T].begin(); A != arcs[T].end(); A++){
    //Check fR(aR , M (p)), if false, return false
    //We do not calculate the pt-combined arc label here, since it's been pre-calculated during net load already for each transition
    if (!A->second.rangeFunction(marking[A->first])){return false;}
  }

  //only if all fR(aR , M (p)) are true, return true
  return true;
}

/// \brief Returns the ID for a given string placename.
/// 
/// Returns 0 if not found.
unsigned int PetriNet::findPlace(std::string placename){
  std::map<unsigned long long, std::string>::iterator pIter;
  for (pIter = places.begin(); pIter != places.end(); pIter++){
    if (pIter->second == placename){return pIter->first;}
  }
  return 0;
}

/// \brief Prints the current net marking, separated by tabs, followed by a newline.
/// 
/// The cellnames argument contains a map from place names to place IDs.
/// If cellnames is empty, prints markings for all places.
void PetriNet::printState(std::map<std::string, unsigned int> & cellnames){
  if (cellnames.size()){
    std::map<std::string, unsigned int>::iterator nIter;
    for (nIter = cellnames.begin(); nIter != cellnames.end(); nIter++){
      printf("%llu\t", marking[nIter->second]);
    }
  }else{
    std::map<unsigned long long, unsigned long long>::iterator i;
    for (i = marking.begin(); i != marking.end(); i++){
      printf("%lli\t", i->second);
    }
  }
  printf("\n");
}

/// \brief Prints the header for states, separated by tabs, followed by a newline.
/// 
/// The cellnames argument contains a map from place names to place IDs.
/// If cellnames is empty, prints headers for all places.
void PetriNet::printStateHeader(std::map<std::string, unsigned int> & cellnames){
  if (cellnames.size()){
    std::map<std::string, unsigned int>::iterator nIter;
    for (nIter = cellnames.begin(); nIter != cellnames.end(); nIter++){
      printf("%s\t", nIter->first.c_str());
    }
  }else{
    std::map<unsigned long long, unsigned long long>::iterator i;
    for (i = marking.begin(); i != marking.end(); i++){
      printf("%s\t", places[i->first].c_str());
    }
  }
  printf("\n");
}

