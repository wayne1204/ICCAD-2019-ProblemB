#ifndef COMPONENT_H
#define COMPONENT_H

#include <vector>
#include <fstream>
#include <utility>
#include <set>
#include <map>
#include "table.h"
#include "math.h"

#define pIF pair<int,FPGA*>
#define pDF pair<float,FPGA*>
#define pFE pair<FPGA*,Edge*>
#define pIN pair<int,Net*>
#define verbose 0

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
        _parent = NULL;
    }
    unsigned getId() {return _uid;}
    // for decompostion
    void setVisited(bool b) {_visited = b; }
    bool isVisited() {return _visited; }
    void setParent(FPGA* p) {_parent = p;}
    FPGA* getParent() {return _parent;}

    void setConnection(Edge* e, FPGA* f);
    int getEdgeNum() {return _connection.size(); }
    Edge* getEdge(unsigned i) {return _connection[i].second; }
    FPGA* getConnectedFPGA(unsigned i) {return _connection[i].first; }
    pFE  getConnection(unsigned i ) {return _connection[i];}
    void showInfo();

private:
    bool _visited;
    unsigned _uid;
    FPGA* _parent;
    vector<pFE> _connection;
};

// FPGA connections
class Edge
{
public:
    Edge(unsigned id ,Table* t){
        _uid = id;
        _weight = 1.0;
        _congestion = 0;
        _T = t;
    }
    static float _AvgWeight;
    void setVertex(FPGA* f1, FPGA* f2) {_source = f1; _target = f2;}
    unsigned getId() {return _uid;}
    float getWeight(){return _weight+pow(2,_congestion/_AvgWeight);}
    int getCongestion(){return _congestion;}
    void updateWeight(int iteration);
    void addCongestion(){_congestion++;}
    void initializeCongestion(){_congestion = 0;}
    void resetNET(){_route.clear();}
    void distributeTDM();
    void addNet(Net* n){_route.push_back(n);}
    //void construcTable(const char* fname){_T->constructTable(fname);}
    int getMaxTableValue(){return _T->getValue(_congestion,0);}

private:
    //static Table _T;
    Table* _T;
    unsigned _uid;
    float _weight;
    int _congestion;
    FPGA* _source;
    FPGA* _target;
    vector<Net*> _route;
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
    void updateMin_edge_TDM(){Min_edge_tdm = _edge_tdm;}
    //void updateMin_TDM(){_min_TDM = _TDM;}
    void initializeCur_route(){_cur_route.clear();}
    int getTDM(){return _TDM;}
    int getId(){return _uid;}
    int getedgeTDM(int i){return _edge_tdm[i];}
    int getMin_routeNum(){return _min_route.size();}
    void setedgeTDM(int i,int c){_edge_tdm[i] = c;}
    void setTDM(int t){_TDM = t;}
    void incrementTDM(int i) {_TDM = _TDM + i; }
    void calculateTDM();
    void decomposition();
    int getSubnetNum() { return _subnets.size(); }
    SubNet* getSubNet(unsigned i) { return _subnets[i]; }
    void addGroup(NetGroup* g) {_netgroup.push_back(g); }
    NetGroup* getNetGroup(unsigned i) {return _netgroup[i]; }
    int getGroupSize() {return _netgroup.size();}
    void showInfo();
    void setMin_routetoEdge();
    void clearEdgeTDM(){_edge_tdm.clear();}
    map<int,int> Min_edge_tdm; //edgeID -> TDM

private:
    unsigned _uid;
    int _TDM;
    //int _min_TDM;
    FPGA* _source;
    vector<NetGroup*> _netgroup;	//TODO can belong to different NetGroup
    vector<FPGA*> _targets;
    vector<SubNet*> _subnets;     //vector for Subnet
    vector<Edge*> _cur_route;
    vector<Edge*> _min_route;
    //map<int,int> _min_edge_tdm; //edgeID -> TDM
    map<int,int> _edge_tdm;     //edgeID -> TDM
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
    void incrementTDM(int i) {_TDM = _TDM + i; }
    int getTDM(){return _TDM;}
    unsigned getId() {return  _uid;}
    void calculateTDM();
    void updateTDM();
private:
    unsigned _uid;
    vector<Net*> _nets;
    int _TDM;
};


#endif
