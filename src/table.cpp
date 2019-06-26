#include <fstream>
#include <iostream>
#include <vector>
#include "table.h"
using namespace std;

// vector<vector<int> > Table::_lookUpTable;

void Table::constructTable(const char* fname){
    fstream fs("lookup_table.txt", ios::in);
    string line, token;
    vector<int> zero;
    zero.push_back(0);
    _lookUpTable.push_back(zero);

    int cnt = 0;
    while(getline(fs, line)){
        vector<int> vec;
        vec.reserve(++cnt);
        size_t begin = 0;
        while(begin != string::npos){
            begin = getToken(begin, line, token);
            int tdm = stoi(token);
            begin = getToken(begin, line, token);
            int times = stoi(token);
            for(int j = 0; j < times; ++j){
                vec.push_back(tdm);
            }
        }
        _lookUpTable.push_back(vec);
    }
}

int Table::getValue (int c, int rank){
    if(c > 10000){
        return c % 2 ? c+1 : c;
    }
    return _lookUpTable[c][rank];
}

size_t Table::getToken(size_t pos, string& s, string& token){
    size_t begin = s.find_first_not_of(' ', pos);
    if(begin == string::npos){
        token = "";
        return begin;
    }
    size_t end = s.find_first_of(' ', begin);
    token = s.substr(begin, end - begin);
    return end;
}
