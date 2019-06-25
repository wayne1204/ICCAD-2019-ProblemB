#ifndef TABLE_H
#define TABLE_H
#include <vector>
using namespace std;

class Table{
public:
    void constructTable(const char* fname);
     int getValue (int c, int rank) {return _lookUpTable[c][rank]; }

private:
    size_t getToken(size_t pos, string& s, string& token);
     vector<vector<int> > _lookUpTable;
};

#endif
// 0
// 2 0
// 2 2
// 4 4 2
// 4 4 4 4
// 6 6 6 4 4
