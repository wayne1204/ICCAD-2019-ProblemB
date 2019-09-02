#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <algorithm>
#include "math.h"
#include <utility>
#include "component.h"
#include <cassert>

float    Edge::_AvgWeight = 0.0;
unsigned FPGA::_globalVisit = 0;
double   Edge::_kRatio = 1;
int      Net::_edge_tdm_size = 0;

bool netCompare(Net* a, Net* b) { 
    return (a->getWeight() > b->getWeight());
}

bool netSubnetComp(Net* a, Net* b) { 
    return (a->getSubnetNum() > b->getSubnetNum());
}

void FPGA::setConnection(Edge* e, FPGA* f){
    _connection.push_back(make_pair(f,e));
}

void FPGA::showInfo(){
    cout << "[ FPGA #" << _uid <<  " ]\n";
    cout << "number of edges: " << getEdgeNum() << endl;
    cout << "(fpga id, edge id): ";
    for(int i = 0; i < getEdgeNum(); i++){
        cout << "(" << _connection[i].first->getId() << "," << _connection[i].second->getId() << ") ";
    }
    cout << endl;
}

void Net::calculateTDM(){
    _TDM = 0;
    for(size_t i = 0; i < _edge_tdm.size(); ++i){
        _TDM += _edge_tdm[i];
    }
}

void Net::calculateMinTDM(){
    _TDM = 0;
    for(size_t i = 0; i < Min_edge_tdm.size(); ++i){
        _TDM += Min_edge_tdm[i];
    }
}

void Net::decomposition(){
    if(_targets.size() == 1){
        SubNet* sn = new SubNet(_source, _targets[0]);
        _subnets.push_back(sn);
        return;
    }
    //construct MST
     /*queue<FPGA*> Q;
     Q.push(_source);
     _source->setVisited(true);
     while(!Q.empty()){
         FPGA* f = Q.front();
         Q.pop();
         for(int i = 0; i < f->getEdgeNum(); ++i){
             if(!f->getConnectedFPGA(i)->isVisited()){
                 Q.push(f->getConnectedFPGA(i));
                 f->getConnectedFPGA(i)->setParent(f);
                 f->getConnectedFPGA(i)->setVisited(true);
             }
         }
     }

     set<FPGA*> mst_set;
     mst_set.insert(_source);*/
    for(size_t i = 0; i < _targets.size(); ++i){
         /*FPGA* f = _targets[i];
         while(f->getParent() != NULL){
             if(mst_set.find(f->getParent()) != mst_set.end())
                 break;
             f = f->getParent();
         }
         mst_set.insert(_targets[i]);
         SubNet* sb = new SubNet( f->getParent(), _targets[i]);
         cout << f->getParent()->getId() << " " << _targets[i]->getId() <<endl;*/
        SubNet* sb = new SubNet( _targets[i], _source);
        _subnets.push_back(sb);
    }
}

void Net::setWeight(double w){
    if(w > _weight)
        _weight = w ;
}


void Net::resetEdgeTDM(){
    // _edge_tdm.clear();  // [Note] only calling resize() will not reset all values to 0
    _edge_tdm.resize(_edge_tdm_size, 0);
}

void Net::showInfo(){
    // cout << "[ Net #" << _uid <<  " ]\n";
    cout << "number of subnets: " << getSubnetNum() << endl;
    cout << "(fpga id, fpga id): ";
    for(int i = 0; i < getSubnetNum(); i++){
        cout << "(" << _subnets[i]->getSource()->getId() << "," << _subnets[i]->getTarget()->getId() << ") ";
    }
    cout << endl;
}

void Edge::distributeTDM(){

    // sort(_route.begin(), _route.end(), netCompare);
    double total_sum = 0;
    for(size_t i = 0; i < _route.size() ; ++i){
        // if(!_route[i]->isDominant()){
        //     sum += _route[i]->getWeight() * _route[i]->getX();
        // }
        total_sum += _route[i]->getWeight() * _route[i]->getX();
    }  

    // double total_TDM = 1;
    size_t i;
    for(i = 0; i < _route.size() ; ++i){
        // if(_route[i]->isDominant()){
            unsigned new_tdm = ceil(total_sum/(_route[i]->getWeight() * _route[i]->getX()));
            new_tdm = (new_tdm % 2 == 0) ? new_tdm : new_tdm + 1;
            assert(new_tdm > 0);
            _route[i]->setedgeTDM(_uid, new_tdm);
            // total_TDM -= 1.0/(double)new_tdm;
        // }
        // else break;
    }
    // for(; i < _route.size() ; ++i){
    //     long long int new_tdm = ceil( (sum/(_route[i]->getWeight() * _route[i]->getX()))*(1.0/total_TDM) );
    //     new_tdm = (new_tdm % 2 == 0) ? new_tdm : new_tdm + 1;
    //     assert(new_tdm > 0);
    //     _route[i]->setedgeTDM(_uid, new_tdm);
    // }
}

int Edge::getTableValue(int congestion, int rank){
    if(congestion % 2 == 0){
        return congestion;
    }
    else{
        if(rank < (congestion+1)/2 ){
            return congestion + 1;
        }
        else
            return congestion - 1; 
    }
    
}

void Edge::removeNet(Net* n){
    auto it = std::find(_route.begin(), _route.end(), n);
    _route.erase(it);
}

void NetGroup::updateTDM(){
    _TDM = 0;
    for (size_t i = 0; i < _nets.size(); ++i){
        _TDM += _nets[i]->getTDM();
    }
    
}


void NetGroup::sumSubnetNum(){
    int ret = 0;
    for(size_t i = 0; i < _nets.size(); ++i){
        ret += _nets[i]->getSubnetNum();
    }
    _subnetnum = ret;
}


void NetGroup::setDominant(){
    _isDominant = true;
    for(size_t i = 0; i < _nets.size(); ++i){
        _nets[i]->setDominant();
    }
}
