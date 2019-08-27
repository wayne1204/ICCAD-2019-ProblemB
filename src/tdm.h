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
    TDM(){
        _domiantGroupCount = 0;
    }
    bool parseFile(const char* fname);
    void findDominantGroup();
    bool outputFile(const char* fname);
    void showStatus(const char* fname);
    void updatekRatio();  
    void decomposeNet(char* fname);
    void global_router(char* fname);

    private:
    size_t getToken(size_t pos, string& s, string& token);
    stack<pFE> Dijkstras(FPGA* source,FPGA* target,unsigned int num);
    void Dijkstras(FPGA* source,FPGA* target,unsigned int num, stack<pFE>&route);
    void local_router(bool b, set<pIN>&);
    void ripup_reroute(set<pIN>&);
    int _domiantGroupCount;  // number of dominat count
    vector<FPGA*> _FPGA_V;
    vector<vector<unsigned char> > _distance;
    vector<Edge*> _edge_V;
    vector<Net*> _net_V;
    vector<NetGroup*> _group_V;
    vector<bool> _pathcheck_V; // for each net to check whether the FPGA is connected or not, then we can stop Dijkstras
};

#endif
