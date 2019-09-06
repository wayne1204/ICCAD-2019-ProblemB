#include <fstream>
#include <iostream>
#include <queue>
#include <set>
#include <algorithm>
#include "math.h"
#include <utility>
#include "component.h"
#include <mutex>

std::mutex mtx;

float    Edge::_AvgWeight = 0.0;
double   Edge::_penalty_weight = 0.0;
unsigned FPGA::_globalVisit = 0;
double   Edge::_kRatio = 1;
int      Net::_edge_tdm_size = 0;
int      Net::_fpga_size = 0;
vector<vector<unsigned char> > Net::_distance;

struct pdfCompare{
    bool operator()(pDF a, pDF b){
        return (a.first > b.first);
    }
};
typedef __gnu_pbds::priority_queue<pDF, pdfCompare> my_pq;
/*struct pifCompare{
    bool operator()(pIF a, pIF b){
        if(a.first == b.first){
            return (a.second->getEdgeNum() < b.second->getEdgeNum());
        }
        return (a.first > b.first);
    }
};*/

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

void Net::Dijkstras(FPGA *source, FPGA *target)
{
    double maxf = numeric_limits<double>::max();
    my_pq Q;
    my_pq::point_iterator iter_V [_fpga_size];
    vector<double> d(_fpga_size, maxf); // distance
    vector<pFE> parent(_fpga_size);     // record parentId and edge to parent
    vector<bool> visited(_fpga_size, false);

    d[source->getId()] = 0.0;
    parent[source->getId()] = pFE(source, NULL);
    iter_V[source->getId()] = Q.push(pDF(0, source));

    FPGA *a = NULL;
    // FPGA::setGlobalVisit();
    while (!Q.empty())
    {
        pDF top = Q.top();
        Q.pop();
        visited[top.second->getId()] = true;
        
        a = top.second;
        if (a == target){
            break; // Find the target
        }
        else if ( _pathCheck[a->getId()] ){
            break; // Find the steiner point
        }
        double w;
        FPGA *b;
        Edge *e;
        for (int i = 0; i < a->getEdgeNum(); i++)
        {
            b = a->getConnectedFPGA(i);
            if(visited[b->getId()])
                continue;
            e = a->getEdge(i);
            w = e->getWeight() + _distance[b->getId()][source->getId()];

            if (d[a->getId()] + w < d[b->getId()])
            {
                double distance = d[b->getId()];
                d[b->getId()] = d[a->getId()] + w;
                parent[b->getId()] = pFE(a, e);

                if (distance == maxf){
                    iter_V[b->getId()] = Q.push(pDF(d[b->getId()], b));
                }
                else{
                    Q.modify(iter_V[b->getId()], pDF(d[b->getId()], b));
                }
            }
        }
    }

    _pathCheck[a->getId()] = true;
    while (a != parent[a->getId()].first)
    {
        FPGA* connectFPGA = parent[a->getId()].first;
        Edge* connectEdge = parent[a->getId()].second;
        _pathCheck[connectFPGA->getId()] = true;
        _edgeNum++;

        mtx.lock();
        connectEdge->addCongestion();
        connectEdge->addNet(this);
        mtx.unlock();


        a = parent[a->getId()].first;
    }
}

void Net::routeSubNet(){

    for (unsigned int i = 0; i < _subnets.size(); i++){
        SubNet *sn = _subnets[i];
        FPGA *source = sn->getSource();
        FPGA *target = sn->getTarget();
        if (_pathCheck[source->getId()] && _pathCheck[target->getId()])
            continue;
        else if (_pathCheck[source->getId()] && !_pathCheck[target->getId()]){ 
            // We can swap source and target to find steiner point efficiently
            swap(source, target);
        }
        Dijkstras(source, target);
        if (_pathCheck[target->getId()]){ 
            // target is not connected
            Dijkstras(target, source);
        }
    }
}

void Net::setWeight(double w){
    if(w > _weight)
        _weight = w ;
}

void Net::updateWeight(){
    _weight *= _x;
    _x = 1;
}
void Net::setX(double x){
    if(x > _x)
        _x = x;
}

void Net::setGroupSubnet(int s){
    if(s > _group_subnet)
        _group_subnet = s;
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
        total_sum += _route[i]->getWeight();
    }  

    // double total_TDM = 1;
    size_t i;
    for(i = 0; i < _route.size() ; ++i){
        // if(_route[i]->isDominant()){
            unsigned new_tdm = ceil(total_sum/(_route[i]->getWeight()));
            new_tdm = (new_tdm % 2 == 0) ? new_tdm : new_tdm + 1;
            _route[i]->setedgeTDM(_uid, new_tdm);
            // total_TDM -= 1.0/(double)new_tdm;
        // }
        // else break;
    }
    // for(; i < _route.size() ; ++i){
    //     long long int new_tdm = ceil( (sum/(_route[i]->getWeight() * _route[i]->getX()))*(1.0/total_TDM) );
    //     new_tdm = (new_tdm % 2 == 0) ? new_tdm : new_tdm + 1;
    //     _route[i]->setedgeTDM(_uid, new_tdm);
    // }
}

double Edge::getWeight(){
    // int power = 3;
    // double x = (_congestion/_AvgWeight) - 1;
    // double b = log(2)*x;
    // double w = 1;
    // double f = 1;

    // for(int i = 0; i < power; ++i){
    //     w = w + 2*pow(b,i)/f;
    //     f *= (i+1);
    // }

    // return (1 + w*2);
    // cout << _congestion/_AvgWeight << endl;
    int penalty = 0;
    if(_congestion > _AvgWeight)
        penalty = _congestion - _AvgWeight;
    return 1 + _congestion + _penalty_weight*penalty;
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
