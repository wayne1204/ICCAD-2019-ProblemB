#ifndef COMPONENT_H
#define COMPONENT_H

#include <vector>
#include <fstream>
#include <utility>
#include <set>
#include <map>
#include <unordered_map>
#include "math.h"

#define pIF pair<int, FPGA*>
#define pDF pair<double, FPGA*>
#define pFE pair<FPGA*, Edge*>
#define pIN pair<int, Net*>
#define pIG pair<int, NetGroup*>
#define pLG pair<long long int, NetGroup*>
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
        _visit = 0;
        _usage = 0;
        _parent = NULL;
    }
    // basic info
    unsigned   getId() {return _uid;}
    void       setConnection(Edge* e, FPGA* f);
    int        getEdgeNum() {return _connection.size(); }
    void       addUsage() {++_usage;}
    int        getUsage() {return _usage;}
    Edge*      getEdge(unsigned i) {return _connection[i].second; }
    FPGA*      getConnectedFPGA(unsigned i) {return _connection[i].first; }
    void       showInfo();

    // for decompostion
    static void setGlobalVisit() {_globalVisit++; }
    void       setVisited(bool b) {_visit = _globalVisit; }
    bool       isVisited() {return _visit == _globalVisit; }
    void       setParent(FPGA* p) {_parent = p;}
    FPGA*      getParent() {return _parent;}
    //void       setisTarget(bool b){_isTarget = b;}
    //bool       isTaegrt(){return _isTarget;}


private:
    static  unsigned _globalVisit;
    unsigned     _visit;
    unsigned     _uid;
    int          _usage;
    FPGA*        _parent;
    vector<pFE>  _connection;
    //bool         _isTarget;
};

// FPGA connections
class Edge
{
public:
    Edge(unsigned id){
        _uid = id;
        _weight = 1.0;
        _congestion = 0;
        _capacity = 0;
    }
    // basic info
    void      setVertex(FPGA* f1, FPGA* f2) {_source = f1; _target = f2;}
    unsigned  getId() {return _uid;}
    FPGA*     getSource() {return _source;}
    FPGA*     getTarget() {return _target;}

    // edge weight
    //double    getWeight(){return pow(2, _congestion / _capacity);}
    double    getWeight(){return pow(2, _congestion / _AvgWeight);}
    void      updateWeight(int iteration);
    void      addCongestion(int i){_congestion += i;}
    void      initCongestion(){_congestion = 0;}
    void      addCapacity(double i) {_capacity += i;}
    double    getCapacity() {return _capacity;}
    int       getTableValue(int, int); 
    int       getNetNum() {return _route.size();}

    // route
    void      resetNet(){_route.clear();}
    void      addNet(Net* n){_route.push_back(n);}
    void      removeNet(Net* n);
    void      distributeTDM();

    static float   _AvgWeight;
    static double  _kRatio;   // ratio used in LUT

private:
    unsigned  _uid;
    float     _weight;
    int       _congestion;
    double    _capacity;
    FPGA*     _source;
    FPGA*     _target;
    vector<Net*> _route;
};

class Net
{
public:
    Net(unsigned id){
        _uid = id;
        _isDominant = false;
        _weight = 0;
        _x = 1;
    }
    // basic info
    int       getId(){ return _uid;}
    void      setSource(FPGA* f) { _source = f;}
    void      setTarget(FPGA* f) { _targets.push_back(f);}
    void      setDominant() {_isDominant = true;}
    void      setNonDominat() {_isDominant = false;}
    bool      isDominant() {return _isDominant;}
    void      setWeight(double w);
    double    getWeight() {return _weight;}
    void      setX(double x) { _x = x;}
    double    getX() {return _x;}       
    FPGA*     getSource() {return _source;}
    FPGA*     getTarget(int i) {return _targets[i];}
    void      showInfo();
    int       getTargetNum(){return _targets.size();}

    // route info
    void      addEdgetoCur_route(Edge* e){ _cur_route.push_back(e);}
    void      initializeCur_route(){ _cur_route.clear();}
    int       getCur_routeNum(){ return _cur_route.size();}
    int       getMin_routeNum(){ return _min_route.size();}
    void      updateMin_route(){ _min_route = _cur_route;}
    void      setMin_routetoEdge();
    Edge*     getCur_route(int i){return _cur_route[i];}

    // TDM function
    long long int getTDM(){ return _TDM;}
    int       getedgeTDM(int i){ return _edge_tdm[i];}
    void      updateMin_edge_TDM(){ Min_edge_tdm = _edge_tdm;}
    void      setedgeTDM(int i,long long int c){_edge_tdm[i] = c;}
    void      setTDM(long long int t){_TDM = t;}
    void      calculateTDM();
    void      calculateMinTDM();
    // void      clearEdgeTDM(){_edge_tdm.clear();}
    void      initEdgeTDM(int size);
    void      clearEdgeTDM();
    //void    updateMin_TDM(){_min_TDM = _TDM;}
    
    // subnet info
    void      decomposition();
    int       getSubnetNum() { return _subnets.size(); }
    SubNet*   getSubNet(unsigned i) { return _subnets[i]; }
    void      setSubNet(SubNet* sn){_subnets.push_back(sn);}

    // group info
    void      addGroup(NetGroup* g) {_netgroup.push_back(g); }
    NetGroup* getNetGroup(unsigned i) {return _netgroup[i]; }
    int       getGroupSize() {return _netgroup.size();}
    //unordered_map<int, long long int> Min_edge_tdm; //edgeID -> TDM
    vector<long long int>Min_edge_tdm;

private:
    bool               _isDominant;
    unsigned           _uid;
    double             _weight;
    double             _x;
    FPGA*              _source;
    long long int      _TDM;
    vector<NetGroup*>  _netgroup;	
    vector<FPGA*>      _targets;
    vector<SubNet*>    _subnets;     //vector for Subnet
    vector<Edge*>      _cur_route;
    vector<Edge*>      _min_route;
   //unordered_map<int,long long int>  _edge_tdm;     //edgeID -> TDM
    vector<long long int> _edge_tdm;
};

class SubNet
{
public:
    SubNet(FPGA* s, FPGA* t)
    {
        // _uid = id;
        // _net = net;
        _source = s;
        _target = t;
    }

    // unsigned   getId() {return _uid;}
    // Net*       getNet() {return _net;}
    FPGA*      getSource() {return _source;}
    FPGA*      getTarget() {return _target;}

private:
    // unsigned  _uid;
    // Net*      _net;
    FPGA*     _source;
    FPGA*     _target;

};

class NetGroup
{
public:
    NetGroup (unsigned id){
        _uid = id;
        _TDM = 0;
        _isDominant = false;
    }
    // basic info
    unsigned      getId() {return  _uid;}
    void          setDominant(); 
    bool          isDominant() {return _isDominant; }
    void          addNet(Net* n) {_nets.push_back(n);}
    int           getNetNum() {return _nets.size();}
    int           getSubnetNum();
    Net*          getNet(int i){return _nets[i];}

    // TDM function
    long long int getTDM(){return _TDM;}
    void          updateTDM();

private:
    bool           _isDominant;  // is dominat group
    unsigned       _uid;
    long long int  _TDM;
    vector<Net*>   _nets;
};


#endif
