#include <fstream>
#include <iostream>
#include <set>
#include <math.h>
#include <limits>
#include <set>
#include "tdm.h"


using namespace std;

// parse input file
bool TDM::parseFile(const char* fname){
    int nums[4];
    string line, token;
    fstream fs(fname, ios::in);

    if(!fs.is_open()){
        return false;
    }

    for(int i = 0; i < 4; ++i){
        fs >> token;
        nums[i] = stoi(token);
    }
    getline(fs, line);
    cout << line <<endl;;

    // fpga
    _FPGA_V.reserve(nums[0]);
    for(int i = 0; i < nums[0]; ++i){
        _FPGA_V.push_back(new FPGA(i));
    }

    // edge
    _edge_V.reserve(nums[1]);
    for(int i = 0; i < nums[1]; ++i){
        size_t begin = 0;
        unsigned f1, f2;
        getline(fs, line);
        begin = getToken(0, line, token);
        f1 = stoi(token);
        begin = getToken(begin, line, token);
        f2 = stoi(token);
        Edge* e = new Edge(i);
        e->setVertex(_FPGA_V[f1], _FPGA_V[f2]);
        _edge_V.push_back(e);
        _FPGA_V[f1]->setConnection(e, _FPGA_V[f2]);
        _FPGA_V[f2]->setConnection(e, _FPGA_V[f1]);
    }

    // nets
    _net_V.reserve(nums[2]);
    for(int i = 0; i < nums[2]; ++i){
        getline(fs, line);
        Net* n = new Net(i);
        size_t begin = getToken(0, line, token);
        n->setSource( _FPGA_V[stoi(token)] );
        while(begin != string::npos){
            begin = getToken(begin, line, token);
            n->setTarget( _FPGA_V[stoi(token)] );
        }
        _net_V.push_back(n);
    }

    // groups
    _net_V.reserve(nums[3]);
    for(int i = 0; i < nums[3]; ++i){
        getline(fs, line);
        NetGroup* g = new NetGroup(i);
        size_t begin = 0;
        while(begin != string::npos){
            begin = getToken(begin, line, token);
            g->addNet( _net_V[stoi(token)] );
            _net_V[stoi(token)]->addGroup(g);
        }
        _group_V.push_back(g);
    }
    cout << "finish parsing! \n";
    return true;
}

void TDM::decomposeNet(){
    for(int i = 0; i < _net_V.size(); ++i){
        _net_V[i]->decomposition();
    }
}

void TDM::showStatus(){
    cout <<" ============[ status ]============ \n";
    cout <<"    #FPGAs   | " << _FPGA_V.size() << endl;
    cout <<"    #edge    | " << _edge_V.size() << endl;
    cout <<"    #nets    | " << _net_V.size() << endl;
    cout <<" #net groups | " << _group_V.size() << endl;

    cout <<endl;
    for(int i = 0; i < _FPGA_V.size(); ++i){
        _FPGA_V[i]->showInfo();
    }

    cout <<endl;
    for(int i = 0; i < _net_V.size(); ++i){
        _net_V[i]->showInfo();
    }
}


//phase1
void TDM::global_router(){

 int iteration = 0;
 int minimumTDM = numeric_limits<int>::max();
 int terminateConditionCount = 0;
 while(1){
    //Initialize Edge's congestion to zero every iteration
    for(unsigned int i=0;i<_edge_V.size();i++){
        _edge_V[i]->initializeCongestion();
    }

    //Use shortest path algorithm to route all nets
    local_router();

    //Calculate group's total TDM
    int maxTDM = 0;
    for(unsigned int i=0;i<_net_V.size();i++){
        _net_V[i]->calculateTDM();
    }
    for(unsigned int i=0;i<_group_V.size();i++){
         _group_V[i]->calculateTDM();
         int t = _group_V[i]->getTDM();
         if(t>maxTDM)maxTDM = t;

    }

    //Terminate condition : Compare with minimum answer. If we can't update  minimum answer more than N times, terminate global router.
    if(maxTDM < minimumTDM){
        //update minimum answer
        minimumTDM = maxTDM;
        for(unsigned int i=0;i<_net_V.size();i++){
            _net_V[i]->updateMin_route();
        }
        terminateConditionCount = 0;
    }
    else{
        terminateConditionCount++;
        if(terminateConditionCount > 3)break;
    }

     //Update edge's weight for next iteration
    for(unsigned int i=0;i<_edge_V.size();i++){
        _edge_V[i]->updateWeight(iteration);
    }
    iteration++;
 }
}

// cut a string from space
size_t TDM::getToken(size_t pos, string& s, string& token){
    size_t begin = s.find_first_not_of(' ', pos);
    if(begin == string::npos){
        token = "";
        return begin;
    }
    size_t end = s.find_first_of(' ', begin);
    token = s.substr(begin, end - begin);
    return end;
}

// struct pIFcomp {
//   bool operator() (const pIF& lhs, const pIF& rhs) const
//   {return lhs.first < rhs.first;}
// };

//single source single target shortest path
stack<pFE> TDM::Dijkstras(FPGA* source,FPGA* target,unsigned int num){
    set<pIF> Q;
    int maxI = numeric_limits<int>::max();
    vector<int>d (num,maxI); //distance
    vector<pFE> parent(num); //Record the parentId and edge of each FPGA after Dijkstras
    d[source->getId()] = 0;
    parent[source->getId()] = pFE(source,NULL);
    Q.insert(pIF(d[source->getId()],source));

    FPGA* a;
    while(!Q.empty())
    {
        pIF top = *Q.begin();
        Q.erase(Q.begin());
        a = top.second;
        if(a==target)break; //Find the target
        else if(_pathcheck_V[a->getId()])break; // Find the steiner point

        int w;
        FPGA* b;
        Edge* e;
        for(unsigned int i=0; i<a->getEdgeNum(); i++){
            b = a->getConnectedFPGA(i);
            e = a->getEdge(i);
            w = e->getWeight();
            if(d[a->getId()] + w < d[b->getId()]){
                if(d[b->getId()]!=maxI){
                    Q.erase(Q.find(pIF(d[b->getId()], b)));
                }
                d[b->getId()] = d[a->getId()] + w;
                parent[b->getId()] = pFE(a,e);
                Q.insert(pIF(d[b->getId()],b));
            }
        }
    }
    stack<pFE> route;
    route.push(pFE(a,NULL));
    while(a != parent[a->getId()].first){
        route.push(parent[a->getId()]);
        a = parent[a->getId()].first;
    }
    return route;
}

//route all the nets by Dijkstras algorithm
void TDM::local_router(){
    for(unsigned int i=0; i < _net_V.size(); i++){
        Net* n = _net_V[i];
        _pathcheck_V.resize(_FPGA_V.size(),false);
        n->initializeCur_route();
        for(unsigned int j=0; j < n->getSubnetNum(); j++){
            SubNet* sn = n->getSubNet(i);
            FPGA* source = sn->getSource();
            FPGA* target = sn->getTarget();
            if(_pathcheck_V[source->getId()] && _pathcheck_V[target->getId()])continue;
			else if(_pathcheck_V[source->getId()] && !_pathcheck_V[target->getId()]){ // We can swap source and target in order to efficiently find steiner point
                FPGA* temp = source;
                source = target;
                target = temp;
            }
            stack<pFE> route_S = Dijkstras(source,target,_FPGA_V.size());

            FPGA* connectFPGA;
            Edge* connectEdge;
            while(!route_S.empty()){
                pFE p = route_S.top();
                route_S.pop();
                connectFPGA = p.first;
                connectEdge = p.second;
                _pathcheck_V[connectFPGA->getId()] = true;
                if(connectEdge!=NULL){
                    connectEdge->addCongestion();
                    n->addEdgetoCur_route(connectEdge);
					connectEdge->addNet(n);
                }
            }
            if(!_pathcheck_V[target->getId()]){ // target is not connected
                route_S = Dijkstras(target,source,_FPGA_V.size());
                while(!route_S.empty()){
                    pFE p = route_S.top();
                    route_S.pop();
                    connectFPGA = p.first;
                    connectEdge = p.second;
                    _pathcheck_V[connectFPGA->getId()] = true;
                    if(connectEdge!=NULL){
                        connectEdge->addCongestion();
                        n->addEdgetoCur_route(connectEdge);
						connectEdge->addNet(n);
                    }
                }
            }
        }
    }
}
