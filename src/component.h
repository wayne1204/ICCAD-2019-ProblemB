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
        _congestion = 0;
        _capacity = 0;
    }
    // basic info
    void      setVertex(FPGA* f1, FPGA* f2) {_source = f1; _target = f2;}
    unsigned  getId() {return _uid;}
    FPGA*     getSource() {return _source;}
    FPGA*     getTarget() {return _target;}

    // edge weight
    double    getWeight();
    void      addCongestion(int i){_congestion += i;}
    void      doubleCongestion(){_congestion *= 2;}
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
    static double  _penalty_weight;

private:
    unsigned     _uid;
    int          _congestion;
    double       _capacity;
    FPGA*        _source;
    FPGA*        _target;
    vector<Net*> _route;
};

class Net
{
public:
    Net(){
        // _uid = id;
        _isDominant = false;
        _edgeNum = 0;
        _weight = 0;
        _x = 1;
        _group_subnet = 0;
    }
    // basic info
    // int       getId(){ return _uid;}
    void      setSource(FPGA* f) { _source = f;}
    void      setTarget(FPGA* f) { _targets.push_back(f);}
    void      setDominant() {_isDominant = true;}
    bool      isDominant() {return _isDominant;}
    void      setWeight(double w);
    void      updateWeight();
    double    getWeight() {return _weight;}
    void      setX(double x);
    double    getX() {return _x;}       
    FPGA*     getSource() {return _source;}
    FPGA*     getTarget(int i) {return _targets[i];}
    void      showInfo();
    int       getTargetNum(){return _targets.size();}

    // route info
    void        addEdgeNum() {_edgeNum++; }
    void        resetEdgeNum() {_edgeNum = 0;}
    int         getEdgeNum() {return _edgeNum;}
    static void setEdgeSize(int s) {_edge_tdm_size = s;}
    void        initPathCheck(int s) {_pathCheck.resize(s, false); }
    bool        isInPathCheck(int i) { return _pathCheck[i]; }
    void        setPathCheck(int i) {_pathCheck[i] = true;}

    // TDM function
    unsigned  getTDM(){ return _TDM;}
    unsigned  getedgeTDM(int i){ return _edge_tdm[i];}
    void      updateMin_edge_TDM(){ Min_edge_tdm = _edge_tdm;}
    void      setedgeTDM(int i, unsigned c){_edge_tdm[i] = c;}
    void      setTDM(int t){_TDM = t;}
    void      calculateTDM();
    void      calculateMinTDM();
    void      resetEdgeTDM();
    
    // subnet info
    void      decomposition();
    int       getSubnetNum() { return _subnets.size(); }
    SubNet*   getSubNet(unsigned i) { return _subnets[i]; }
    void      setSubNet(SubNet* sn){_subnets.push_back(sn);}

    // group info
    void      setGroupSubnet(int s);
    int       getGroupSubnet(){return _group_subnet;}
    vector<unsigned>Min_edge_tdm;

private:
    // unsigned           _uid;
    bool               _isDominant;
    int                _edgeNum;
    int                _group_subnet;
    static int         _edge_tdm_size;
    unsigned           _TDM;
    double             _weight;
    double             _x;
    FPGA*              _source;
    vector<bool>       _pathCheck;
    vector<FPGA*>      _targets;
    vector<SubNet*>    _subnets;     //vector for Subnet
    vector<unsigned>   _edge_tdm;
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
    NetGroup (){
        // _uid = id;
        _TDM = 0;
        _isDominant = false;
    }
    // basic info
    // unsigned      getId() {return  _uid;}
    void          setDominant(); 
    bool          isDominant() {return _isDominant; }
    void          addNet(Net* n) {_nets.push_back(n);}
    int           getNetNum() {return _nets.size();}
    void          sumSubnetNum();
    int           getSubnetNum(){return _subnetnum;}
    Net*          getNet(int i){return _nets[i];}

    // TDM function
    long long int getTDM(){return _TDM;}
    void          updateTDM();

private:
    // unsigned       _uid;
    bool           _isDominant;  // is dominat group
    int            _subnetnum;
    long long int  _TDM;
    vector<Net*>   _nets;
};


#endif
