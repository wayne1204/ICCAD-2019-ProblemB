#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <utility>
#include "component.h"

#define pIN pair<int,Net*>

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
    for(unsigned int i=0; i<_cur_route.size();i++){
        // _TDM += lookupTable[_cur_route[i]->getCongestion()][0]; //check lookuptable name
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
    _source->setVisited();
    while(!Q.empty()){
        FPGA* f = Q.front();
        Q.pop();
        for(int i = 0; i < f->getEdgeNum(); ++i){
            if(!f->getConnectedFPGA(i)->isVisited()){
                Q.push(f->getConnectedFPGA(i));
                f->getConnectedFPGA(i)->setParent(make_pair(f ,f->getEdge(i)));
                f->getConnectedFPGA(i)->setVisited();
            }
        }
    }

    set<Edge*> subnets;
    int count = 0;
    for(int i = 0; i < _targets.size(); ++i){
        FPGA* f = _targets[i];
        while(f->getParent().first != NULL){ 
            Edge* e = f->getParent().second;
            if(subnets.find(e) == subnets.end()){
                subnets.insert(e);
                SubNet* sb = new SubNet(count++, this, f->getParent().first, f);
                _subnets.push_back(sb);
            }
            f = f->getParent().first;
        }
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
    _weight = (_weight*iteration + _congestion)/(iteration+1);
}

void Edge::distributeTDM(vector<bool>* calculated){

    set<pIN> sortedNet;
    int rank = 0;
    //insert
    for(unsigned int i=0; i<_route.size();i++){

        Net* nn = _route[i];
        NetGroup* ng = nn->getNetGroup();    //TODO getNetGroup()??
        int cost = ng->getTDM();
        sortedNet.insert(pIN(cost,nn));
    }

    for (std::set<pIN>::iterator it=sortedNet.begin(); it!=sortedNet.end(); ++it){

        Net* nn = it->second;
        int id = nn->getId();
        int tdm = 0;//lookuptable[_congestion][rank];

        if(!calculated->at(id)){
            nn->setTDM(tdm);
        }
        else{
            tdm = nn->getTDM() + tdm;
            nn->setTDM(tdm);
        }

        rank++;
    }


}


