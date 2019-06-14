#include <fstream>
#include <iostream>
#include "tdm.h"
using namespace std;

// parse input file
bool TDM::parseFile(const char* fname){
    int nums[4];
    string line, token;
    fstream fs(fname, ios::in);

    if(!fs.is_open()){
        return false;
    }

    for(int i = 0; i < 4; ++i){
        fs >> token;
        nums[i] = stoi(token);
    }
    getline(fs, line);
    cout << line <<endl;;

    // fpga
    _FPGA_V.reserve(nums[0]);
    for(int i = 0; i < nums[0]; ++i){
        _FPGA_V.push_back(new FPGA(i));
    }

    // edge
    _edge_V.reserve(nums[1]);
    for(int i = 0; i < nums[1]; ++i){
        size_t begin = 0;
        unsigned f1, f2;
        getline(fs, line);
        begin = getToken(0, line, token);
        f1 = stoi(token);
        begin = getToken(begin, line, token);
        f2 = stoi(token);
        Edge* e = new Edge(i);
        e->setVertex(_FPGA_V[f1], _FPGA_V[f2]);
        _edge_V.push_back(e);
    }

    // nets
    _net_V.reserve(nums[2]);
    for(int i = 0; i < nums[2]; ++i){
        getline(fs, line);
        Net* n = new Net(i);
        size_t begin = getToken(0, line, token);
        n->setSource( _FPGA_V[stoi(token)] );
        while(begin != string::npos){
            begin = getToken(begin, line, token);
            n->setTarget( _FPGA_V[stoi(token)] );
        }
        _net_V.push_back(n);
    }

    // groups
    _net_V.reserve(nums[3]);
    for(int i = 0; i < nums[3]; ++i){
        getline(fs, line);
        NetGroup* g = new NetGroup(i);
        size_t begin = 0;
        while(begin != string::npos){
            begin = getToken(begin, line, token);
            g->addNet( _net_V[stoi(token)] );
        }
        _group_V.push_back(g);
    }
    cout << "finish parsing! \n";
    return true;
}


void TDM::showStatus(){
    cout <<" ============[ status ]============ \n";
    cout <<"    #FPGAs   | " << _FPGA_V.size() << endl;
    cout <<"    #edge    | " << _edge_V.size() << endl;
    cout <<"    #nets    | " << _net_V.size() << endl;
    cout <<" #net groups | " << _group_V.size() << endl;
}

// cut a string from space  
size_t TDM::getToken(size_t pos, string& s, string& token){
    size_t begin = s.find_first_not_of(' ', pos);
    if(begin == string::npos){
        token = "";
        return begin;
    }
    size_t end = s.find_first_of(' ', begin);
    token = s.substr(begin, end - begin);
    return end; 
}