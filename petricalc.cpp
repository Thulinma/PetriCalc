#include "petricalc.h"
#include <deque>
#include <algorithm>

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
    }
    if (name == "Transition"){
      d = 0;
      while ((d = c->IterateChildren(d))){addTransition(d);}
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
  }
  transitions.insert(std::pair<unsigned int, PetriTrans>(T.id, T));
  #if DEBUG >= 10
  fprintf(stderr, "Added transition ID %u, guard %s\n", T.id, T.guard.c_str());
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
      while ((d = c->IterateChildren(d))){addEdge(d, EDGE_READ);}
    }
    //other types not supported yet
  }
}

void PetriNet::addEdge(TiXmlNode * N, edgeType E){
  TiXmlElement * e;
  PetriEdge R;
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
  std::map<unsigned int, unsigned int> counter;
  std::map<unsigned int, unsigned int>::iterator cit;
  std::map<unsigned int, PetriPlace> TMPplaces = places;//make backup for restoring later
  
  
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
  
  fprintf(stderr, "Stepping: %u transitions enabled, %u in conflict\n", (unsigned int)enabled.size(), (unsigned int)conflict.size());

  //remove conflicting transitions from set of enabled transitions
  for (dT = conflict.begin(); dT != conflict.end(); dT++){enabled.erase(*dT);}

  //all non-conflicting transitions consume their input as many times as possible
  //no output is done just yet
  while (enabled.size() > 0){
    sT = enabled.begin();
    unsigned int cnt = isEnabled(*sT);
    if (cnt > 0){
      counter[*sT] += cnt;//mark transition
      doInput(*sT, cnt);
    }
    enabled.erase(sT);//transition is never enabled anymore at this point
  }

  //all conflicting transitions consume their input in random order, one time
  while (conflict.size() > 0){
    std::random_shuffle(conflict.begin(), conflict.end());
    unsigned int cnt = isEnabled(conflict[0]);
    if (cnt < 1){
      conflict.erase(conflict.begin());
    }else{
      counter[conflict[0]] += 1;//mark transition
      doInput(conflict[0], 1);
      if (isEnabled(conflict[0]) < 1){conflict.erase(conflict.begin());}
    }
  }

  //cancel operations so far...
  places = TMPplaces;
  
  //For every marking on every transition, calculate appropiate output tokes and output them.
  for (cit = counter.begin(); cit != counter.end(); cit++){
    doInputOutput(cit->first, cit->second);
  }
  
  //Clear all markings, we are now ready for the next calculating step.
  return true;
}

bool PetriNet::conflicts(unsigned int transition, std::set<unsigned int> & checked_transitions){
  std::set<unsigned int>::iterator edgeit;
  std::set<unsigned int>::iterator checkit;
  for (edgeit = transitions[transition].inputs.begin(); edgeit != transitions[transition].inputs.end(); edgeit++){
    if (edges[*edgeit].etype == EDGE_READ){continue;}
    if (edges[*edgeit].multiplicity > 0){
      for (checkit = checked_transitions.begin(); checkit != checked_transitions.end(); checkit++){
        if (*checkit == transition){continue;}//skip self
        if (transitions[*checkit].inputs.find(*edgeit) != transitions[*checkit].inputs.end()){
          return true;//we found a conflict, return true
        }
      }
    }else{
      /// \todo Conflict checking for colored nets is missing
      //assuming always in conflict for now
      return true;
    }
  }
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
unsigned int PetriNet::isEnabled(unsigned int T){
  std::map<std::string, std::string> vars;
  std::set<unsigned int>::iterator edgeit;
  unsigned int fire_count = 0xFFFFFFFF; //start with maximum possible
  if (transitions[T].guard.size() > 0){
    return 0;/// \todo guard support
  }else{
    for (edgeit = transitions[T].inputs.begin(); edgeit != transitions[T].inputs.end(); edgeit++){
      if (edges[*edgeit].multiplicity > 0){
        //no expression, just a number
        if (places[edges[*edgeit].source].iMarking >= edges[*edgeit].multiplicity){
          //if edgetype is EDGE_READ, fire count is not affected as long as it is enabled
          if (edges[*edgeit].etype == EDGE_READ){continue;}
          unsigned int tmp = places[edges[*edgeit].source].iMarking / edges[*edgeit].multiplicity;
          if (tmp < fire_count){fire_count = tmp;}//set fire_count to the minimum count of the input edges
        }else{
          return 0;
        }
      }else{
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
  if (fire_count == 0xFFFFFFFF){fire_count = 1;}//only fire once if fire count is maximum to prevent crazy
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

void PetriNet::doInput(unsigned int T, unsigned int cnt){
  std::map<std::string, std::string> vars;
  std::set<unsigned int>::iterator edgeit;
  #if DEBUG >= 5
  fprintf(stderr, "Doing input step for transition %u...\n", T);
  #endif
  if (transitions[T].guard.size() > 0){
    fprintf(stderr, "Guard unsupported!\n");
    return;//we don't support guards just yet...
  }else{
    for (edgeit = transitions[T].inputs.begin(); edgeit != transitions[T].inputs.end(); edgeit++){
      if (edges[*edgeit].multiplicity > 0){
        //remove input, only if edgetype is not EDGE_READ
        if (edges[*edgeit].etype == EDGE_READ){continue;}
        places[edges[*edgeit].source].iMarking -= edges[*edgeit].multiplicity * cnt;
      }else{
        //try to match an input, we assume this is successful because isEnabled was already called.
        findInput(edges[*edgeit].source, edges[*edgeit].expression, vars);
        //inputs are now matched, we can fill in the variables and remove the corresponding token
        places[edges[*edgeit].source].marking[fillVars(edges[*edgeit].expression, vars)]--;
      }
    }
  }
}

void PetriNet::doInputOutput(unsigned int T, unsigned int cnt){
  #if DEBUG >= 4
  fprintf(stderr, "Transition %u fires %u times (%u inputs, %u outputs):\n", T, cnt, (unsigned int)transitions[T].inputs.size(), (unsigned int)transitions[T].outputs.size());
  #endif
  std::map<std::string, std::string> vars;
  std::set<unsigned int>::iterator edgeit;
  if (transitions[T].guard.size() > 0){
    return;//we don't support guards just yet...
  }else{
    for (edgeit = transitions[T].inputs.begin(); edgeit != transitions[T].inputs.end(); edgeit++){
      if (edges[*edgeit].multiplicity > 0){
        //remove input, only if edgetype is not EDGE_READ
        if (edges[*edgeit].etype == EDGE_READ){continue;}
        places[edges[*edgeit].source].iMarking -= edges[*edgeit].multiplicity * cnt;
        #if DEBUG >= 5
        fprintf(stderr, "Lowered place %s by %u X %u to %u...\n", places[edges[*edgeit].source].name.c_str(), edges[*edgeit].multiplicity, cnt, places[edges[*edgeit].source].iMarking);
        #endif
      }else{
        //try to match an input, we assume this is successful because isEnabled was already called.
        findInput(edges[*edgeit].source, edges[*edgeit].expression, vars);
        //inputs are now matched, we can fill in the variables and remove the corresponding token
        places[edges[*edgeit].source].marking[fillVars(edges[*edgeit].expression, vars)]--;
      }
    }
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
      printf("%u\t", pIter->second.iMarking);
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
      printf("%u\t", places[nIter->second].iMarking);
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
