#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include "math.h"
#include <utility>
#include "component.h"
#include <cassert>

float Edge::_AvgWeight = 0.0;

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
    for(auto it = _edge_tdm.begin();it!=_edge_tdm.end();++it){
        _TDM += it->second;
    }
}

void Net::calculateminTDM(){
    _TDM = 0;
    for(auto it = Min_edge_tdm.begin();it!=Min_edge_tdm.end();++it){
        _TDM += it->second;
    }
}

void Net::decomposition(){
    if(_targets.size() == 1){
        SubNet* sn = new SubNet(0, this, _source, _targets[0]);
        _subnets.push_back(sn);
        return;
    }
    // construct MST
    queue<FPGA*> Q;
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

    int count = 0;
    set<FPGA*> mst_set;
    mst_set.insert(_source);
    for(unsigned int i = 0; i < _targets.size(); ++i){
        FPGA* f = _targets[i];
        while(f->getParent() != NULL){
            if(mst_set.find(f->getParent()) != mst_set.end())
                break;
            f = f->getParent();
        }
        mst_set.insert(_targets[i]);
        SubNet* sb = new SubNet(count++, this, f->getParent(), _targets[i]);
        // cout << f->getParent()->getId() << " " << f->getId() <<endl;
        _subnets.push_back(sb);
    }
}

void Net::showInfo(){
    cout << "[ Net #" << _uid <<  " ]\n";
    cout << "number of subnets: " << getSubnetNum() << endl;
    cout << "(fpga id, fpga id): ";
    for(int i = 0; i < getSubnetNum(); i++){
        cout << "(" << _subnets[i]->getSource()->getId() << "," << _subnets[i]->getTarget()->getId() << ") ";
    }
    cout << endl;
}


void NetGroup::calculateTDM(){
    _TDM = 0;
    for(unsigned int i=0; i<_nets.size();i++){
        _TDM += _nets[i]->getTDM();
    }

}
void Edge::updateWeight(int iteration){
    _weight = (_weight*(float)iteration + pow(2,(float)_congestion/_AvgWeight))/((float)iteration+1);
    if(_weight <1)_weight  = 1;
    //cout<<"Edge "<<this->getId()<<" : "<<_weight<<endl;
}

void Edge::distributeTDM(){

    multiset<pIN> sortedNet;
    int rank = 0;

    
    //insert
    for(unsigned int i = 0; i < _route.size(); i++){
        Net* nn = _route[i];
        int max_cost = 0;
        for(int j = 0; j < nn->getGroupSize(); ++j){
            NetGroup* ng = nn->getNetGroup(j);    
            int cost = ng->getTDM();
            max_cost = max(cost,max_cost);
        }
        sortedNet.insert(pIN(max_cost,nn));
        
    }

    for (auto it=sortedNet.begin(); it!=sortedNet.end(); ++it){
        Net* nn = it->second;
        int new_tdm = _T->getValue(_congestion,rank);//lookuptable[_congestion][rank];
        nn->setedgeTDM(_uid,new_tdm);
        rank++;
    }

    //TODO update net groupTDM
    
}
void NetGroup::updateTDM(){
    _TDM = 0;
    for (unsigned int i = 0; i < _nets.size(); ++i){
        _TDM += _nets[i]->getTDM();
    }
    
}



void Net::setMin_routetoEdge(){
    Edge* e;
    for(unsigned int i=0;i<_min_route.size();i++){
        e = _min_route[i];
        e->addCongestion();
        e->addNet(this);
    }

}


