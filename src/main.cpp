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
    Table* table = new Table;
    table->constructTable("lookput_table.txt");
    tdm.setTable(table);

    if(!tdm.parseFile(argv[1])){
        cout << "file \"" << argv[1] << "\" not found!\n";
        return -1;
    }
    tdm.decomposeNet();
    tdm.global_router();
    tdm.showStatus();
    tdm.outputFile(argv[2]);
    return 0;
}
