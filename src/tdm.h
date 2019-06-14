#ifndef TDM_H
#define TDM_H
#include <vector>
#include "component.h"

using namespace std;

class TDM
{
    public:
    bool parseFile(const char* fname);
    void showStatus();

    private:
    size_t getToken(size_t pos, string& s, string& token);
    vector<FPGA*> _FPGA_V;
    vector<Edge*> _edge_V;
    vector<Net*> _net_V;
    vector<NetGroup*> _group_V;
};

#endif