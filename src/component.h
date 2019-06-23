#ifndef COMPONENT_H
#define COMPONENT_H

#include <vector>
#include <fstream>
#include <utility>

#define pIF pair<int,FPGA*>
#define pFE pair<FPGA*,Edge*>

using namespace std;

class FPGA;
class Edge;
class Net;
class SubNet;
class NetGroup;

class FPGA
{
public:
    FPGA(unsigned id){
        _uid = id;
        _visited = false;
        pFE p(NULL, NULL);
        _parent = p;
    }
    unsigned getId() {return _uid;}
    // for decompostion
    void setVisited() {_visited = true; }
    bool isVisited() {return _visited; }
    void setParent(pFE p) {_parent = p;}
    pFE getParent() {return _parent;}

    void setConnection(Edge* e, FPGA* f);
    int getEdgeNum() {return _connection.size(); }
    Edge* getEdge(unsigned i) {return _connection[i].second; }
    FPGA* getConnectedFPGA(unsigned i) {return _connection[i].first; }
    pFE  getConnection(unsigned i ) {return _connection[i];}
    void showInfo();

private:
    bool _visited;
    unsigned _uid;
    pFE _parent;
    vector<pFE> _connection;
};

// FPGA connections
class Edge
{
public:
    Edge(unsigned id){
        _uid = id;
        _weight = 1;
        _congestion = 0;
    }
    void setVertex(FPGA* f1, FPGA* f2) {_source = f1; _target = f2;}
    unsigned getId() {return _uid;}
    int getWeight(){return _weight;}
    int getCongestion(){return _congestion;}
    void updateWeight(int iteration);
    void addCongestion(){_congestion++;}
    void initializeCongestion(){_congestion = 0;}

private:
    unsigned _uid;
    int _weight;
    int _congestion;
    FPGA* _source;
    FPGA* _target;
};

class Net
{
public:
    Net(unsigned id){
        _uid = id;
    }
    void setSource(FPGA* f) {_source = f;}
    void setTarget(FPGA* f) {_targets.push_back(f);}
    void addEdgetoCur_route(Edge* e){_cur_route.push_back(e);}
    void updateMin_route(){_min_route = _cur_route;}
    void initializeCur_route(){_cur_route.clear();}
    int getTDM(){return _TDM;}
    void setTDM(int t){_TDM = t;}
    void calculateTDM();
    void decomposition();
    int getSubnetNum() { return _subnets.size(); }
    SubNet* getSubNet(unsigned i) { return _subnets[i]; }
    void showInfo();

private:
    unsigned _uid;
    int _TDM;
    FPGA* _source;
    vector<FPGA*> _targets;
    vector<SubNet*> _subnets;     //vector for Subnet
    vector<Edge*> _cur_route;
    vector<Edge*> _min_route;
};

class SubNet
{
public:
    SubNet(unsigned id, Net* net, FPGA* s, FPGA* t)
    {
        _uid = id;
        _net = net;
        _source = s;
        _target = t;
    }

    unsigned getId() {return _uid;}
    Net* getNet() {return _net;}
    FPGA* getSource() {return _source;}
    FPGA* getTarget() {return _target;}
private:
    unsigned _uid;
    Net* _net;
    FPGA* _source;
    FPGA* _target;
    
};

class NetGroup
{
public:
    NetGroup (unsigned id){
        _uid = id;
        _TDM = 0;
    }
    void addNet(Net* n) {_nets.push_back(n);}
    int getTDM(){return _TDM;}
    void calculateTDM();
private:
    unsigned _uid;
    vector<Net*> _nets;
    int _TDM;
};


#endif
