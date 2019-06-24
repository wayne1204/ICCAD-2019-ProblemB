#include <fstream>
#include <vector>
#include "table.h"
using namespace std;

// vector<vector<int> > Table::_lookUpTable;

void Table::constructTable(const char* fname){
    fstream fs("lookup_table.txt", ios::in);
    string line, token;
    vector<int> zero;
    _lookUpTable.push_back(zero);

    for(int i = 0; i < 1000; ++i){
        getline(fs, line);
        vector<int> vec;
        vec.resize(i+1);
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