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
        //_Edge_avg_weight_for_nondominant = 0;
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
    void local_router(bool b, set<pIN>& sorted_net);


    private:
    size_t getToken(size_t pos, string& s, string& token);
    int    strToInt(string& s);
    
    int               _domiantGroupCount;  // number of dominat count
    int               _iteration_limit;
    double            _avg_net;
    long long int     _total_subnet;
    //long long int     _Edge_avg_weight_for_nondominant;
    double            _avg_subnet;
    // vector<bool>      _pathcheck_V; // for each net to check whether the FPGA is connected or not, then we can stop Dijkstras
    vector<FPGA*>     _FPGA_V;
    vector<Edge*>     _edge_V;
    vector<Net*>      _net_V;
    vector<NetGroup*> _group_V;
    vector<vector<unsigned char> > _distance;
};

#endif
