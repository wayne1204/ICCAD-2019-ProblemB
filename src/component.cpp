#include <fstream>
#include <iostream>
#include "component.h"

void Net::calculateTDM(){
    _TDM = 0;
    for(unsigned int i=0; i<_cur_route.size();i++){
        _TDM += lookupTable[_cur_route[i]->getCongestion()]; //check lookuptable name
    }
}
void NetGroup::calculateTDM(){
    _TDM = 0;
    for(unsigned int i=0; i<_nets.size();i++){
        _TDM += _nets[i]->getTDM();
    }

}
void Edge::updateWeight(int iteration){
    _weight = (_weight*iteration + _congestion)/(iteration+1);
}


