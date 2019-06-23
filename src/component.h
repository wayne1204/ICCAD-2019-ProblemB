#ifndef COMPONENT_H
#define COMPONENT_H

#include <vector>
#include <fstream>
using namespace std;

class FPGA;
class Edge;
class Net;
class NetGroup;

class FPGA
{
public:
    FPGA(unsigned id){
        _uid = id;
    }
    unsigned getId() {return _uid;}
    void connect(Net* n) {_connections.push_back(n);}

private:
    unsigned _uid;
    vector<Net*> _connections;
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
    int getWeight(){return _weight;}
    int getCongestion(){return _congestion;}
    void updateWeight(int iteration);
    void addCongestion(){_congestion++;}
    void initializeCongestion(){_congestion = 0;}

private:
    unsigned _uid;
    FPGA* _source;
    FPGA* _target;
    int _weight;
    int _congestion;
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
    //GetSubnetNum
private:
    unsigned _uid;
    FPGA* _source;
    vector<FPGA*> _targets;
    //vector for Subnet
    vector<Edge*> _cur_route;
    vector<Edge*> _min_route;
    int _TDM;
};

//need class Subnet

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
