#include "petricalc.h"
#include <deque>
#include <algorithm>

unsigned char protColR[3]={255,0,0};
unsigned char protColG[3]={0,255,0};
unsigned char protColB[3]={0,0,255};
unsigned int spacing=30;	//between cells.
float MAX_PROTEIN = 1000;
double diffusionConstant=3.0f;

char maimColors(int input){
  if (input == 0) return 25;
  if (input < 50) return 25+((float)input/1.0f);
  if (input < 100) return 50+((float)(input-50)/2.0f);
  if (input < 200) return 75+((float)(input-100)/4.0f);
  if (input < 400) return 100+((float)(input-200)/8.0f);
  if (input < 800) return 125+((float)(input-400)/16.0f);
  if (input < 1600) return 150+((float)(input-800)/32.0f);
  if (input < 3200) return 175+((float)(input-1600)/64.0f);
  if (input < 6400) return 200+((float)(input-3200)/128.0f);
  if (input < 12800) return 225+((float)(input-6400)/256.0f);
  return 255;
}

void PetriNet::updateColors(){
  static float lastmax, currmax;
  std::map<std::string, unsigned int>::iterator it;
  currmax = 0;
  if (lastmax < 50) lastmax=50;
  for(unsigned int i=0; i< Cells.size();i++){
    Cells[i].r=0;
    Cells[i].g=0;	
    Cells[i].b=0;
    unsigned int j=0;
    //for(; j< Cells[i].protein.size();j++){		
    for( it=Cells[i].protein.begin(); it != Cells[i].protein.end(); it++ ){
      Cells[i].r+= protColR[j]* it->second;
      Cells[i].g+= protColG[j]* it->second;
      Cells[i].b+= protColB[j]* it->second;
      j++;
    }
    Cells[i].r= maimColors(Cells[i].r);
    Cells[i].g= maimColors(Cells[i].g);
    Cells[i].b= maimColors(Cells[i].b);
  }
  lastmax = currmax;
}

void PetriNet::createNet(int sx, int sy, int sz){
  Cells.resize( sx*sy*sz );
  double connectedCells = 26.0f; //number of other cells: 8
  double weightPerCell= diffusionConstant*connectedCells;
  int i=0;
  for(int x=0;x<sx;x++){
    for(int z=0;z<sz;z++){
      for(int y=0;y<sy;y++){
        Cells[i].y=y*spacing;
        Cells[i].x=x*spacing;
        Cells[i].z=z*spacing;
        i++;
      }
    }
  }
  for(int x=0;x<sx;x++){
    for(int z=0;z<sz;z++){
      for(int y=0;y<sy;y++){
        for (int j=-1;j<2;j++){
          for (int k=-1;k<2;k++){
            for (int l=-1;l<2;l++){
              if ( (x+j > -1) && (z+k>-1) && (y+l>-1) && (x+j < sx) && (z+k < sz) && (y+l < sy) ){
                if (!(j==0 && k ==0 && l ==0)){	Cells[(x*sz*sy)+(z*sy)+y].Connect( &Cells[((x+j)*sz*sy)+((z+k)*sy)+(y+l)] ,weightPerCell); }
              }
            }
          }
        }
      }
    }
  }
}

void PetriNet::createNet(int sx, int sy){
  Cells.resize( sx*sy );
  double connectedCells = 8.0f; //number of other cells: 8
  double weightPerCell= diffusionConstant*connectedCells;
  int i=0;
  for(int x=0;x<sx;x++){
    for(int y=0;y<sy;y++){
      Cells[i].y=0;
      Cells[i].x=x*spacing;
      Cells[i].z=y*spacing;
      i++;
    }
  }
  for(int x=0;x<sx;x++){
    for(int y=0;y<sy;y++){
      for (int j=-1;j<2;j++){
        for (int l=-1;l<2;l++){
          if ( (x+j > -1) && (y+l>-1) && (x+j < sx) && (y+l < sy) ){
            if (!(j==0 && l ==0)){	Cells[(x*sy)+y].Connect( &Cells[((x+j)*sy)+(y+l)] ,weightPerCell); }
          }
        }
      }
    }
  }
}

void PetriNet::createNet(int sx){
  Cells.resize(sx);
  double connectedCells = 2.0f;
  double weightPerCell= diffusionConstant*connectedCells;
  int i=0;
  for(int x=0;x<sx;x++){
    Cells[i].y=0;
    Cells[i].z=0;
    Cells[i].x=x*spacing;
    i++;
  }
  for(int x=0;x<sx;x++){
    for (int l=-1;l<2;l++){
      if ( (x+l > -1) && (x+l < sx) ){
        if (!(l ==0)){	Cells[x].Connect( &Cells[(x+l)] ,weightPerCell); }
      }
    }
  }
}

void PetriNet::addProteinType(string name){
  for(unsigned int i=0; i< Cells.size();i++){
    Cells[i].protName.push_back(name);
    Cells[i].protein.insert( pair<string, unsigned int>(name, 0) );
  }
}

//not stubs! Yay! need to be moved to a .cpp file, sometime...
void Cell::Connect(Cell * other, double weight){Weight[other] = weight;}
void Cell::Connect(unsigned int count, Cell ** other, double weight){for (unsigned int i = 0; i < count; i++){Connect(other[i], weight);}}
Cell::Cell(int sx, int sy, int sz){x = sx; y = sy; z= sz;}
Cell::Cell(){x = 0; y = 0; z= 0;}

PetriNet::PetriNet(std::string XML){
  myXML = TiXmlDocument(XML);
  myXML.LoadFile();
  TiXmlElement * e = myXML.RootElement();
  TiXmlNode * c = e->FirstChild("nodeclasses");
  if (!c){
    printf("Error: Parsed file is not a valid snoopy petri net\n");
    exit(42);
  }
  parseNodes(c);
  c = e->FirstChild("edgeclasses");
  if (!c){
    printf("Error: Parsed file is not a valid snoopy petri net\n");
    exit(42);
  }
  parseEdges(c);
  c = e->FirstChild("metadataclasses");
  if (!c){
    printf("Error: Parsed file is not a valid snoopy petri net\n");
    exit(42);
  }
  parseMeta(c);
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
    if (name == "MarkingList"){
      TiXmlNode * g = 0, * s = c->FirstChild()->LastChild();//get the table body
      while ((g = s->IterateChildren(g))){
        std::string marking;
        unsigned int mcount = 0;
        marking = g->FirstChild()->ToElement()->GetText();
        mcount = atoi(g->LastChild()->ToElement()->GetText());
        printf("  Token for next place: %s, %u times\n", marking.c_str(), mcount);
        P.marking.insert(std::pair<std::string, unsigned int>(marking, mcount));
        initial += mcount;
      }
    }
  }
  places.insert(std::pair<unsigned int, PetriPlace>(P.id, P));
  printf("Added place ID %u, color %s, %u initial tokens\n", P.id, P.color.c_str(), initial);
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
  printf("Added transition ID %u, guard %s\n", T.id, T.guard.c_str());
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
  }
  edges.insert(std::pair<unsigned int, PetriEdge>(R.id, R));
  printf("Added edge from %u to %u, expression %s\n", R.source, R.target, R.expression.c_str());
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
        printf("Added colorset %s, type %s, value %s\n", C.name.c_str(), C.type.c_str(), C.value.c_str());
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
        printf("Added variable %s, type %s\n", name.c_str(), type.c_str());
      }
    }
  }
}

void PetriNet::CalculateStep(){
  std::map<unsigned int, PetriTrans>::iterator T;
  std::deque<unsigned int> enabled;
  std::map<unsigned int, unsigned int> counter;
  std::map<unsigned int, unsigned int>::iterator cit;
  std::map<unsigned int, PetriPlace> TMPplaces = places;//make backup for restoring later
  
  
  //Every transition is checked for enabledness, and made part of a subset consisting of only enabled transitions.
  for (T = transitions.begin(); T != transitions.end(); T++){
    if (isEnabled(T->first)){enabled.push_back(T->first);}
  }
  printf("Stepping - %u transitions enabled\n", (unsigned int)enabled.size());
  
  //A random transition is picked from this subset, it's input tokens are removed, but no output is done yet. This transition is now marked. (A transition can be marked multiple times)
  //Repeat step 1 and 2, until no more transitions are enabled.
  while (enabled.size() > 0){
    std::random_shuffle(enabled.begin(), enabled.end());
    if (!isEnabled(enabled[0])){
      enabled.erase(enabled.begin());
    }else{
      counter[enabled[0]]++;//mark transition
      doInput(enabled[0]);
      if (!isEnabled(enabled[0])){enabled.erase(enabled.begin());}
    }
  }

  //cancel operations so far...
  places = TMPplaces;
  
  //For every marking on every transition, calculate appropiate output tokes and output them.
  for (cit = counter.begin(); cit != counter.end(); cit++){
    for (unsigned int i = 0; i < cit->second; ++i){
      doInputOutput(cit->first);
    }
  }
  
  //Clear all markings, we are now ready for the next calculating step.
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

bool PetriNet::isEnabled(unsigned int T){
  std::map<std::string, std::string> vars;
  std::map<unsigned int, PetriEdge>::iterator edgeit;
  if (transitions[T].guard.size() > 0){
    return false;//we don't support guards just yet...
  }else{
    for (edgeit = edges.begin(); edgeit != edges.end(); edgeit++){
      if (edgeit->second.target == T){//this is an input edge
        //try to match an input
        if (!findInput(edgeit->second.source, edgeit->second.expression, vars)){
          //if no match can be made, this transition can't fire
          return false;
        }
      }
    }
  }
  return true;
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

void PetriNet::doInput(unsigned int T){
  std::map<std::string, std::string> vars;
  std::map<unsigned int, PetriEdge>::iterator edgeit;
  if (transitions[T].guard.size() > 0){
    return;//we don't support guards just yet...
  }else{
    for (edgeit = edges.begin(); edgeit != edges.end(); edgeit++){
      if (edgeit->second.target == T){//this is an input edge
        //try to match an input, we assume this is successful because isEnabled was already called.
        findInput(edgeit->second.source, edgeit->second.expression, vars);
        //inputs are now matched, we can fill in the variables and remove the corresponding token
        places[edgeit->second.source].marking[fillVars(edgeit->second.expression, vars)]--;
      }
    }
  }
}

void PetriNet::doInputOutput(unsigned int T){
  std::map<std::string, std::string> vars;
  std::map<unsigned int, PetriEdge>::iterator edgeit;
  if (transitions[T].guard.size() > 0){
    return;//we don't support guards just yet...
  }else{
    for (edgeit = edges.begin(); edgeit != edges.end(); edgeit++){
      if (edgeit->second.target == T){//this is an input edge
        //try to match an input, we assume this is successful because isEnabled was already called.
        findInput(edgeit->second.source, edgeit->second.expression, vars);
        //inputs are now matched, we can fill in the variables and remove the corresponding token
        places[edgeit->second.source].marking[fillVars(edgeit->second.expression, vars)]--;
      }
      if (edgeit->second.source == T){//this is an output edge
        //fill in the variables and add the corresponding token
        places[edgeit->second.target].marking[fillVars(edgeit->second.expression, vars)]++;
      }
    }
  }
}

void PetriNet::addProduction(string name, int cell, int amount) {
  Cells[cell].Produce[name] = amount;
}


void PetriNet::addDegradation(string name, int percentage) {
  std::vector<Cell>::iterator it;
  for ( it = Cells.begin(); it != Cells.end(); it++ ){
    (*it).Degrade[name] = percentage;
  }
}
