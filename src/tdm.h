#ifndef TDM_H
#define TDM_H
#include <vector>
#include <stack>
#include <utility>
#include "component.h"

using namespace std;

class TDM
{
    public:
    bool parseFile(const char* fname);
    void showStatus();
    void decomposeNet();
    void global_router();

    private:
    size_t getToken(size_t pos, string& s, string& token);
    stack<pair<FPGA*,Edge*> > Dijkstras(FPGA* source,FPGA* target,unsigned int num);
    void local_router();
    vector<FPGA*> _FPGA_V;
    vector<Edge*> _edge_V;
    vector<Net*> _net_V;
    vector<NetGroup*> _group_V;
    vector<bool> _pathcheck_V; // for each net to check whether the FPGA is connected or not, then we can stop Dijkstras
};

#endif
