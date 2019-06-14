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
    }
    void setVertex(FPGA* f1, FPGA* f2) {_source = f1; _target = f2;}

private:
    unsigned _uid;
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
private:
    unsigned _uid;
    FPGA* _source;
    vector<FPGA*> _targets;
};

class NetGroup
{
public:
    NetGroup (unsigned id){
        _uid = id;
    }
    void addNet(Net* n) {_nets.push_back(n);}
private:
    unsigned _uid;
    vector<Net*> _nets;
};


#endif
