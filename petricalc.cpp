#include "petricalc.h"
#include <deque>
#include <algorithm>

// intern werken met range arcs werken die values hebben voor effect
// uitwerken formeel van hoe alles werkt
// updaten eerste document

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
  c = e->FirstChild("metadataclasses");
  if (c){
    parseMeta(c);
  }
  LoadCache();
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
  PetriPlace P;
  unsigned int initial = 0;
  e = N->ToElement();
  const char * id = e->Attribute("id");
  if (!id){return;}
  P.id = atoi(id);
  TiXmlNode * c = 0;
  while ((c = N->IterateChildren(c))){
    e = c->ToElement();
    if (!e){continue;}
    if (!e->Attribute("name")){continue;}
    std::string name = e->Attribute("name");
    if (name == "Colorset"){
      P.color = e->GetText();
    }
    if (name == "Name"){
      P.name = e->GetText();
    }
    if (name == "ID"){
      if (P.name == ""){
        P.name = std::string("place_")+e->GetText();
      }
    }
    if (name == "Marking"){
      P.iMarking = atoi(e->GetText());
    }
    if (name == "MarkingList"){
      TiXmlNode * g = 0, * s = c->FirstChild()->LastChild();//get the table body
      while ((g = s->IterateChildren(g))){
        std::string marking;
        unsigned int mcount = 0;
        marking = g->FirstChild()->ToElement()->GetText();
        mcount = atoi(g->LastChild()->ToElement()->GetText());
        fprintf(stderr, "  Token for next place: %s, %u times\n", marking.c_str(), mcount);
        P.marking.insert(std::pair<std::string, unsigned int>(marking, mcount));
        initial += mcount;
      }
    }
  }
  places.insert(std::pair<unsigned int, PetriPlace>(P.id, P));
  #if DEBUG >= 10
  fprintf(stderr, "Added place %s ID %u, color %s, %u initial tokens\n", P.name.c_str(), P.id, P.color.c_str(), initial+P.iMarking);
  #endif
}

void PetriNet::addTransition(TiXmlNode * N){
  TiXmlElement * e;
  PetriTrans T;
  e = N->ToElement();
  const char * id = e->Attribute("id");
  if (!id){return;}
  T.id = atoi(id);
  TiXmlNode * c = 0;
  while ((c = N->IterateChildren(c))){
    e = c->ToElement();
    if (!e){continue;}
    if (!e->Attribute("name")){continue;}
    std::string name = e->Attribute("name");
    if (name == "GuardList"){
      TiXmlNode * g = 0, * s = c->FirstChild()->LastChild();//get the table body
      while ((g = s->IterateChildren(g))){
        T.guard = g->LastChild()->ToElement()->GetText();
      }
    }
    if (name == "Name"){
      T.name = e->GetText();
    }
    if (name == "ID"){
      if (T.name == ""){
        T.name = std::string("trans_")+e->GetText();
      }
    }
  }
  transitions.insert(std::pair<unsigned int, PetriTrans>(T.id, T));
  #if DEBUG >= 10
  fprintf(stderr, "Added transition %s ID %u, guard %s\n", T.name.c_str(), T.id, T.guard.c_str());
  #endif
}

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
  fprintf(stderr, "Loaded %u edges\n", (unsigned int)edges.size());
}

void PetriNet::addEdge(TiXmlNode * N, edgeType E){
  TiXmlElement * e;
  PetriEdge R;
  R.multiplicity = 0;
  e = N->ToElement();
  const char * id = e->Attribute("id");
  if (!id){return;}
  R.id = atoi(id);
  R.source = atoi(e->Attribute("source"));
  R.target = atoi(e->Attribute("target"));
  R.etype = E;
  if (transitions.find(R.source) != transitions.end()){
    transitions[R.source].outputs.insert(R.id);
  }
  if (transitions.find(R.target) != transitions.end()){
    transitions[R.target].inputs.insert(R.id);
  }
  TiXmlNode * c = 0;
  while ((c = N->IterateChildren(c))){
    e = c->ToElement();
    if (!e){continue;}
    if (!e->Attribute("name")){continue;}
    std::string name = e->Attribute("name");
    if (name == "ExpressionList"){
      TiXmlNode * g = 0, * s = c->FirstChild()->LastChild();//get the table body
      while ((g = s->IterateChildren(g))){
        R.expression = g->LastChild()->ToElement()->GetText();
      }
    }
    if (name == "Multiplicity"){
      R.multiplicity = atoi(e->GetText());
    }
  }
  if ((R.expression == "") && !R.multiplicity){R.multiplicity = 1;}
  edges.insert(std::pair<unsigned int, PetriEdge>(R.id, R));
  #if DEBUG >= 10
  fprintf(stderr, "Added edge from %u to %u, multiplicity %u, expression %s\n", R.source, R.target, R.multiplicity, R.expression.c_str());
  #endif
}

void PetriNet::parseMeta(TiXmlNode * N){
  TiXmlNode * c = 0, * d = 0;
  TiXmlElement * e;
  while ((c = N->IterateChildren(c))){
    e = c->ToElement();
    if (!e){continue;}
    if (!e->Attribute("name")){return;}
    std::string name = e->Attribute("name");
    if (name == "Basic Colorset Class"){
      d = 0;
      while ((d = c->IterateChildren(d))){addColset(d);}
    }
    if (name == "Structured Colorset Class"){
      d = 0;
      while ((d = c->IterateChildren(d))){addColset(d);}
    }
    if (name == "Variable Class"){
      d = 0;
      while ((d = c->IterateChildren(d))){addVariable(d);}
    }
    //other types not supported yet
  }
}

void PetriNet::addColset(TiXmlNode * N){
  TiXmlElement * e;
  PetriColor C;
  TiXmlNode * c = 0;
  while ((c = N->IterateChildren(c))){
    e = c->ToElement();
    if (!e){continue;}
    if (!e->Attribute("name")){continue;}
    std::string name = e->Attribute("name");
    if (name == "ColorsetList" || name == "StructuredColorsetList"){
      TiXmlNode * g = 0, * s = c->FirstChild()->LastChild();//get the table body
      while ((g = s->IterateChildren(g))){
        C.name = g->FirstChild()->ToElement()->GetText();
        C.type = g->FirstChild()->NextSibling()->ToElement()->GetText();
        C.value = g->LastChild()->ToElement()->GetText();
        colors.insert(std::pair<std::string, PetriColor>(C.name, C));
        #if DEBUG >= 10
        fprintf(stderr, "Added colorset %s, type %s, value %s\n", C.name.c_str(), C.type.c_str(), C.value.c_str());
        #endif
      }
    }
  }
}

void PetriNet::addVariable(TiXmlNode * N){
  TiXmlElement * e;
  PetriColor C;
  TiXmlNode * c = 0;
  while ((c = N->IterateChildren(c))){
    e = c->ToElement();
    if (!e){continue;}
    if (!e->Attribute("name")){continue;}
    std::string name = e->Attribute("name");
    if (name == "VariableList"){
      TiXmlNode * g = 0, * s = c->FirstChild()->LastChild();//get the table body
      while ((g = s->IterateChildren(g))){
        std::string name, type;
        name = g->FirstChild()->ToElement()->GetText();
        type = g->FirstChild()->NextSibling()->ToElement()->GetText();
        variables.insert(std::pair<std::string, std::string>(name, type));
        #if DEBUG >= 10
        fprintf(stderr, "Added variable %s, type %s\n", name.c_str(), type.c_str());
        #endif
      }
    }
  }
}

bool PetriNet::CalculateStep(){
  std::map<unsigned int, PetriTrans>::iterator T;
  std::set<unsigned int> enabled;
  std::set<unsigned int>::iterator sT;
  std::deque<unsigned int> conflict;
  std::deque<unsigned int>::iterator dT;
  std::map<unsigned int, unsigned long long int> counter;
  std::map<unsigned int, unsigned long long int>::iterator cit;
  
  
  //Every transition is checked for enabledness, and made part of a subset consisting of only enabled transitions.
  for (T = transitions.begin(); T != transitions.end(); T++){
    if (isEnabled(T->first)){enabled.insert(T->first);}
  }
  if (enabled.size() == 0){return false;}

  //for all enabled transitions, check if they conflict with any of the other enabled transitions
  for (sT = enabled.begin(); sT != enabled.end(); sT++){
    if (conflicts(*sT, enabled)){
      conflict.push_back(*sT);//add to set of conflicting transitions
    }
  }

  #if DEBUG >= 5
  fprintf(stderr, "Stepping: %u transitions enabled, %u in conflict\n", (unsigned int)enabled.size(), (unsigned int)conflict.size());
  #endif

  //remove conflicting transitions from set of enabled transitions
  for (dT = conflict.begin(); dT != conflict.end(); dT++){enabled.erase(*dT);}

  //all non-conflicting transitions consume their input as many times as possible
  //no output is done just yet
  while (enabled.size() > 0){
    sT = enabled.begin();
    unsigned long long int cnt = isEnabled(*sT);
    if (cnt > 0){
      counter[*sT] += cnt;//mark transition
      doInput(*sT, cnt);
    }
    enabled.erase(sT);//transition is never enabled anymore at this point
  }

  //all conflicting transitions consume their input in random order, one time
  //calculations are done as "midstep" type to prevent activator arcs from disabling already enabled edges
  while (conflict.size() > 0){
    std::random_shuffle(conflict.begin(), conflict.end());
    unsigned long long int cnt = isEnabled(conflict[0], CALC_MIDSTEP);
    if (cnt < 1){
      conflict.erase(conflict.begin());
    }else{
      cnt = (rand() % cnt) + 1;//do random amount of times
      counter[conflict[0]] += cnt;//mark transition
      doInput(conflict[0], cnt);
      if (isEnabled(conflict[0], CALC_MIDSTEP) < 1){conflict.erase(conflict.begin());}
    }
  }

  //For every marking on every transition, calculate appropiate output tokes and output them.
  for (cit = counter.begin(); cit != counter.end(); cit++){
    doOutput(cit->first, cit->second);
  }
  
  //Clear all markings, we are now ready for the next calculating step.
  return true;
}

void PetriNet::LoadCache(){
  std::set<unsigned int>::iterator edgeit;
  std::set<unsigned int>::iterator edgeit2;
  std::map<unsigned int, PetriTrans>::iterator transIt;
  std::map<unsigned int, PetriTrans>::iterator transIt2;
  for (transIt = transitions.begin(); transIt != transitions.end(); transIt++){
    for (edgeit = transIt->second.inputs.begin(); edgeit != transIt->second.inputs.end(); edgeit++){
      if (edges[*edgeit].etype == EDGE_ACTIVATOR){continue;}//these arcs are not affected by conflicts
      if (edges[*edgeit].etype == EDGE_INHIBITOR){continue;}//these arcs are not affected by conflicts
      if (edges[*edgeit].etype == EDGE_EQUAL){continue;}//these arcs are not affected by conflicts
      if (edges[*edgeit].multiplicity > 0){
        for (transIt2 = transitions.begin(); transIt2 != transitions.end(); transIt2++){
          if (transIt2 != transIt){//skip self
            for (edgeit2 = transIt2->second.inputs.begin(); edgeit2 != transIt2->second.inputs.end(); edgeit2++){
              if (edges[*edgeit2].etype == EDGE_INHIBITOR){continue;}//these arcs are not affected by conflicts
              if (edges[*edgeit].source == edges[*edgeit2].source){
                transIt->second.conflicts_with.insert(transIt2->first);
                break;
              }
            }
          }
        }
      }else{
        /// \todo Conflict checking for colored nets is missing
        //assuming in conflict with all for now
        for (transIt2 = transitions.begin(); transIt2 != transitions.end(); transIt2++){
          if (transIt2 != transIt){//skip self
            transIt->second.conflicts_with.insert(transIt2->first);
          }
        }
      }
    }
  }
}

bool PetriNet::conflicts(unsigned int transition, std::set<unsigned int> & checked_transitions){
  std::set<unsigned int>::iterator checkit;
  //check the cache and checked_transitions for matching transitions
  //any match = conflicting transition
  if (checked_transitions.size() > transitions[transition].conflicts_with.size()){
    for (checkit = transitions[transition].conflicts_with.begin(); checkit != transitions[transition].conflicts_with.end(); checkit++){
      if (checked_transitions.count(*checkit) > 0){return true;}
    }
  }else{
    for (checkit = checked_transitions.begin(); checkit != checked_transitions.end(); checkit++){
      if (transitions[transition].conflicts_with.count(*checkit) > 0){return true;}
    }
  }
  //no matches = no conflict
  return false;
}

bool PetriNet::findInput(unsigned int src, std::string expression, std::map<std::string, std::string> & vars){
  std::map<std::string, unsigned int>::iterator it;
  for (it = places[src].marking.begin(); it != places[src].marking.end(); it++){
    if (it->second < 1){continue;}
    if (expression.size() == 1){//we assume variable when the expression is one character
      if (vars[expression] == ""){
        vars[expression] = it->first;
        return true;
      }else{
        if (vars[expression] == it->first){return true;}
      }
    }else{
      //TODO: parse other types of expressions
    }
  }
  return false;
}

/// Returns how many times this expression can fire currently
unsigned long long int PetriNet::isEnabled(unsigned int T, calcType C){
  /// \todo Optimize this by only checking changed transitions (changed input/output) every iteration
  std::map<std::string, std::string> vars;
  std::set<unsigned int>::iterator edgeit;
  unsigned long long int fire_count = 0xFFFFFFF; //start with maximum possible
  if (transitions[T].guard.size() > 0){
    return 0;/// \todo guard support
  }else{
    for (edgeit = transitions[T].inputs.begin(); edgeit != transitions[T].inputs.end(); edgeit++){
      unsigned long long int marking = places[edges[*edgeit].source].iMarking;
      unsigned long long int multiplicity = edges[*edgeit].multiplicity;
      //skip read edges if calculation type is midstep
      if ((C == CALC_MIDSTEP) && (edges[*edgeit].etype == EDGE_ACTIVATOR)){continue;}
      //skip equals edges if calculation type is midstep
      if ((C == CALC_MIDSTEP) && (edges[*edgeit].etype == EDGE_EQUAL)){continue;}

      //no expression, just a number
      if (multiplicity > 0){
        //inhibitor arcs disable firing if filled, unaffect firing if not filled
        if (edges[*edgeit].etype == EDGE_INHIBITOR){
          if (marking < multiplicity){continue;}
          return 0;
        }
        //equal arcs disable firing if not equal, unaffect firing if they are
        if (edges[*edgeit].etype == EDGE_EQUAL){
          if (marking == multiplicity){continue;}
          return 0;
        }
        if (marking >= multiplicity){
          //if edgetype is EDGE_ACTIVATOR, fire count is not affected as long as it is enabled
          if (edges[*edgeit].etype == EDGE_ACTIVATOR){continue;}
          unsigned long long int tmp = 0;
          if (multiplicity == 1){tmp = marking;}else{tmp = marking/multiplicity;}
          if (tmp < fire_count){fire_count = tmp;}//set fire_count to the minimum count of the input edges
        }else{
          return 0;
        }
      }else{
        fprintf(stderr, "Error: not a multiplicity\n");
        //try to match an input
        if (!findInput(edges[*edgeit].source, edges[*edgeit].expression, vars)){
          //if no match can be made, this transition can't fire
          return 0;
        }else{
          if (fire_count > 0){fire_count = 1;}//do one at a time for nontrivial cases
        }
      }
    }
  }
  if (fire_count == 0xFFFFFFFF){
    //this makes no sense
    std::cerr << "A transition could fire unboundedly. Don't do that." << std::endl;
    exit(999);
  }
  return fire_count;
}

std::string fillVars(std::string S, std::map<std::string, std::string> vars){
  std::map<std::string, std::string>::iterator it;
  //replace all variables with their values in the given string, then return it
  for (it = vars.begin(); it != vars.end(); it++){
    while (S.find(it->first) != std::string::npos){
      S.replace(S.find(it->first), it->first.size(), it->second);
    }
  }
  return S;
}

void PetriNet::doInput(unsigned int T, unsigned long long int cnt){
  std::map<std::string, std::string> vars;
  std::set<unsigned int>::iterator edgeit;
  #if DEBUG >= 5
  fprintf(stderr, "Doing input step for transition %s...\n", transitions[T].name.c_str());
  #endif
  if (transitions[T].guard.size() > 0){
    fprintf(stderr, "Guard unsupported!\n");
    return;//we don't support guards just yet...
  }else{
    for (edgeit = transitions[T].inputs.begin(); edgeit != transitions[T].inputs.end(); edgeit++){
      if (edges[*edgeit].multiplicity > 0){
        //remove input, only if edgetype is not EDGE_ACTIVATOR or EDGE_INHIBITOR or EDGE_EQUAL
        if (edges[*edgeit].etype == EDGE_ACTIVATOR){continue;}
        if (edges[*edgeit].etype == EDGE_INHIBITOR){continue;}
        if (edges[*edgeit].etype == EDGE_EQUAL){continue;}
        places[edges[*edgeit].source].iMarking -= edges[*edgeit].multiplicity * cnt;
        //for reset edges, remove all tokens
        if (edges[*edgeit].etype == EDGE_RESET){places[edges[*edgeit].source].iMarking = 0;}
      }else{
        //try to match an input, we assume this is successful because isEnabled was already called.
        findInput(edges[*edgeit].source, edges[*edgeit].expression, vars);
        //inputs are now matched, we can fill in the variables and remove the corresponding token
        places[edges[*edgeit].source].marking[fillVars(edges[*edgeit].expression, vars)]--;
      }
    }
  }
}

void PetriNet::doOutput(unsigned int T, unsigned long long int cnt){
  #if DEBUG >= 5
  fprintf(stderr, "Transition %s fires %u times (%u inputs, %u outputs):\n", transitions[T].name.c_str(), cnt, (unsigned int)transitions[T].inputs.size(), (unsigned int)transitions[T].outputs.size());
  #endif
  std::map<std::string, std::string> vars;
  std::set<unsigned int>::iterator edgeit;
  if (transitions[T].guard.size() > 0){
    return;//we don't support guards just yet...
  }else{
    for (edgeit = transitions[T].outputs.begin(); edgeit != transitions[T].outputs.end(); edgeit++){
      if (edges[*edgeit].multiplicity > 0){
        places[edges[*edgeit].target].iMarking += edges[*edgeit].multiplicity * cnt;
        #if DEBUG >= 5
        fprintf(stderr, "Upped place %s by %u X %u to %u...\n", places[edges[*edgeit].target].name.c_str(), edges[*edgeit].multiplicity, cnt, places[edges[*edgeit].target].iMarking);
        #endif
      }else{
        //fill in the variables and add the corresponding token
        places[edges[*edgeit].target].marking[fillVars(edges[*edgeit].expression, vars)]++;
      }
    }
  }
}

void PetriNet::PrintState(){
  std::map<unsigned int, PetriPlace>::iterator pIter;
  for (pIter = places.begin(); pIter != places.end(); pIter++){
    if (pIter->second.marking.empty()){
      printf("%llu\t", pIter->second.iMarking);
    }else{
      printf("colored_unsupported\t");
    }
  }
  printf("\n");
}

void PetriNet::PrintStateHeader(){
  std::map<unsigned int, PetriPlace>::iterator pIter;
  for (pIter = places.begin(); pIter != places.end(); pIter++){
    printf("%s\t", pIter->second.name.c_str());
  }
  printf("\n");
}

unsigned int PetriNet::findPlace(std::string placename){
  std::map<unsigned int, PetriPlace>::iterator pIter;
  for (pIter = places.begin(); pIter != places.end(); pIter++){
    if (pIter->second.name == placename){return pIter->second.id;}
  }
  return 0;
}

void PetriNet::PrintState(std::map<std::string, unsigned int> & cellnames){
  std::map<std::string, unsigned int>::iterator nIter;
  for (nIter = cellnames.begin(); nIter != cellnames.end(); nIter++){
    if (places[nIter->second].marking.empty()){
      printf("%llu\t", places[nIter->second].iMarking);
    }else{
      printf("colored_unsupported\t");
    }
  }
  printf("\n");
}

void PetriNet::PrintStateHeader(std::map<std::string, unsigned int> & cellnames){
  std::map<std::string, unsigned int>::iterator nIter;
  for (nIter = cellnames.begin(); nIter != cellnames.end(); nIter++){
    printf("%s\t", nIter->first.c_str());
  }
  printf("\n");
}
