#include "petricalc.h"
#include <deque>
#include <algorithm>

PetriArc::PetriArc(){
  rangeLow = 0;
  rangeHigh = 0xFFFFFFFFFFFFFFFFull;
  effect = 0;
}

PetriArc::PetriArc(unsigned long long rLow, unsigned long long rHigh, long long e){
  rangeLow = rLow;
  rangeHigh = rHigh;
  effect = e;
}

// From definition 11: fr ((l, h), m) = true if l ≤ m ≤ h, false otherwise
bool PetriArc::rangeFunction(unsigned long long m){
  return (rangeLow <= m && m <= rangeHigh);
}

// From definition 11: fe (e, m) = e + m
void PetriArc::effectFunction(unsigned long long & m){
  m += effect;
}

// From definition 11: ⊗(((l1 , h1 ), e1 ), ((l2 , h2 ), e2 )) = (l1 + l2 , min(h1 , h2)), e1 + e2 )
void PetriArc::combine(PetriArc param){
  #if DEBUG >= 5
  fprintf(stderr, "Combining: ((%llu, %llu), %lld) COMB ((%llu, %llu), %lld) = ", rangeLow, rangeHigh, effect, param.rangeLow, param.rangeHigh, param.effect);
  #endif 
  //Set l to the sum of l1 and l2
  rangeLow += param.rangeLow;
  //Set h to the maximum of h1 and h2
  if (param.rangeHigh < rangeHigh){rangeHigh = param.rangeHigh;}
  //Set e to the sum of e1 and e2
  effect += param.effect;
  fprintf(stderr, "((%llu, %llu), %lld)\n", rangeLow, rangeHigh, effect);
}

PetriNet::PetriNet(std::string XML){
  myXML = TiXmlDocument(XML);
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
  fprintf(stderr, "Added place %s ID %llu, %llu initial tokens\n", places[ID].c_str(), ID, marking[ID]);
  #endif
}

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
  fprintf(stderr, "Added transition %s ID %llu\n", transitions[ID].c_str(), ID);
  #endif
}

enum edgeType{
  EDGE_NORMAL,
  EDGE_ACTIVATOR,
  EDGE_INHIBITOR,
  EDGE_RESET,
  EDGE_EQUAL
};

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
  
  unsigned long long aRLow, aRHigh;
  long long aEffect;

  if (E == EDGE_NORMAL){
    if (multiplicity < 0){
      aRLow = -multiplicity;
    }else{
      aRLow = 0;
    }
    aRHigh = 0xFFFFFFFFFFFFFFFFull;
    aEffect = multiplicity;
  }
  if (E == EDGE_ACTIVATOR){
    if (multiplicity < 0){
      aRLow = -multiplicity;
    }else{
      aRLow = multiplicity;
    }
    aRHigh = 0xFFFFFFFFFFFFFFFFull;
    aEffect = 0;
  }
  if (E == EDGE_INHIBITOR){
    if (multiplicity < 0){
      aRHigh = -multiplicity - 1;
    }else{
      aRHigh = multiplicity - 1;
    }
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
    aEffect = 0;
  }
  if (E == EDGE_RESET){
    if (multiplicity < 0){
      aRLow = -multiplicity;
    }else{
      aRLow = 0;
    }
    aRHigh = 0xFFFFFFFFFFFFFFFFull;
    aEffect = -0xFFFFFFFFFFFFFFll;
  }

  if (arcs.count(transition) && arcs[transition].count(place)){
    #if DEBUG >= 5
    fprintf(stderr, "Combining arc between transition %s and place %s:\n", transitions[transition].c_str(), places[place].c_str());
    #endif
    arcs[transition][place].combine(PetriArc(aRLow, aRHigh, aEffect));
  }else{
    #if DEBUG >= 10
    fprintf(stderr, "Added arc between transition %s and place %s: ((%llu, %llu), %lli)\n", transitions[transition].c_str(), places[place].c_str(), aRLow, aRHigh, aEffect);
    #endif
    arcs[transition][place] = PetriArc(aRLow, aRHigh, aEffect);
  }
}

bool PetriNet::calculateStep(){
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

void PetriNet::printState(){
  std::map<unsigned long long, unsigned long long>::iterator i;
  for (i = marking.begin(); i != marking.end(); i++){
    printf("%lli\t", i->second);
  }
  printf("\n");
}

void PetriNet::printStateHeader(){
  std::map<unsigned long long, unsigned long long>::iterator i;
  for (i = marking.begin(); i != marking.end(); i++){
    printf("%s\t", places[i->first].c_str());
  }
  printf("\n");
}

unsigned int PetriNet::findPlace(std::string placename){
  std::map<unsigned long long, std::string>::iterator pIter;
  for (pIter = places.begin(); pIter != places.end(); pIter++){
    if (pIter->second == placename){return pIter->first;}
  }
  return 0;
}

void PetriNet::printState(std::map<std::string, unsigned int> & cellnames){
  std::map<std::string, unsigned int>::iterator nIter;
  for (nIter = cellnames.begin(); nIter != cellnames.end(); nIter++){
    printf("%llu\t", marking[nIter->second]);
  }
  printf("\n");
}

void PetriNet::printStateHeader(std::map<std::string, unsigned int> & cellnames){
  std::map<std::string, unsigned int>::iterator nIter;
  for (nIter = cellnames.begin(); nIter != cellnames.end(); nIter++){
    printf("%s\t", nIter->first.c_str());
  }
  printf("\n");
}

