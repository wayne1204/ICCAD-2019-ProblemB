#include <iostream>
#include "tdm.h"
#include "table.h"
using namespace std;

int main(int argc, char** argv){
    if(argc != 3){
        cout << "syntax error, usage:" << endl;
        cout << "./tdm [input_file] [output_file] " << endl;
        cout << endl;
        return -1;
    }
    TDM tdm;
    Table table;
    
    if(!tdm.parseFile(argv[1])){
        cout << "file \"" << argv[1] << "\" not found!\n";
        return -1;
    }
    table.constructTable("lookput_table.txt");
    tdm.decomposeNet();
    tdm.showStatus();
    return 0;
}