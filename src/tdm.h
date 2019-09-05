#ifndef TDM_H
#define TDM_H
#include <vector>
#include <stack>
#include <utility>
#include "component.h"
#include <time.h>


using namespace std;

class TDM
{
    public:
    TDM(){
        _domiantGroupCount = 0;
        _avg_net = 0;
        _avg_subnet = 0;
        _total_subnet = 0;
    }
    bool parseFile(const char* fname);
    void preRoute();
    void findDominantGroup();
    void decomposeNet();
    bool outputFile(const char* fname);
    void buildMST(Net* n);
    void showStatus(const char* fname);
    void global_router(char* fname);
    void ripup_reroute(set<pIN>&);
    void setStarttime(){_start_time = clock();}
    clock_t getStarttime(){return _start_time;}

    private:
    size_t getToken(size_t pos, string& s, string& token);
    //stack<pFE> Dijkstras(FPGA* source,FPGA* target,unsigned int num);
    void Dijkstras(FPGA* source,FPGA* target,unsigned int num, Net* n);
    void local_router(bool b, set<pIN>&);
    
    int               _domiantGroupCount;  // number of dominat count
    int               _iteration_limit;
    double            _avg_net;
    long long int     _total_subnet;
    double            _avg_subnet;
    clock_t           _start_time;
    vector<bool>      _pathcheck_V; // for each net to check whether the FPGA is connected or not, then we can stop Dijkstras
    vector<FPGA*>     _FPGA_V;
    vector<Edge*>     _edge_V;
    vector<Net*>      _net_V;
    vector<NetGroup*> _group_V;
    vector<vector<unsigned char> > _distance;
};

#endif
