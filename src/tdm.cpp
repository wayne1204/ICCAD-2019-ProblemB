#include <fstream>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <set>
#include <math.h>
#include <limits>
#include <unordered_set>
#include <time.h>
#include <ext/pb_ds/priority_queue.hpp>

#include "tdm.h"
#include "../include/ctpl.h"

using namespace std;

bool groupCompare(NetGroup* a, NetGroup* b) { 
    return (a->getSubnetNum() > b->getSubnetNum());
}

bool netSubnetCompare(Net* a, Net* b) { 
    return (a->getSubnetNum() > b->getSubnetNum());
}
struct pdfCompare{
    bool operator()(pDF a, pDF b){
        return (a.first > b.first);
    }
};
typedef __gnu_pbds::priority_queue<pDF, pdfCompare> my_pq;


// parse input file
bool TDM::parseFile(const char *fname)
{
    int nums[4];
    int value;
    clock_t start, end;
    string line, token;
    fstream fs(fname, ios::in);

    start = clock();
    if (!fs.is_open())
    {
        return false;
    }
    cout << "\n [parsing] \n";
    for (int i = 0; i < 4; ++i){
        fs >> nums[i];
    }
    getline(fs, line);

    // fpga
    _FPGA_V.reserve(nums[0]);
    for (int i = 0; i < nums[0]; ++i)
    {
        _FPGA_V.push_back(new FPGA(i));
    }
    _pathcheck_V.resize(_FPGA_V.size(), false);

    // edge
    _edge_V.reserve(nums[1]);
    for (int i = 0; i < nums[1]; ++i)
    {
        // size_t begin = 0;
        unsigned f1, f2;
        fs >> f1 >> f2 ;
        getline(fs, line);
        Edge *e = new Edge(i);
        e->setVertex(_FPGA_V[f1], _FPGA_V[f2]);
        _edge_V.push_back(e);
        _FPGA_V[f1]->setConnection(e, _FPGA_V[f2]);
        _FPGA_V[f2]->setConnection(e, _FPGA_V[f1]);
    }

    // nets
    _net_V.reserve(nums[2]);
    for (int i = 0; i < nums[2]; ++i)
    {
        getline(fs, line);
        Net *n = new Net();
        size_t begin = getToken(0, line, token);
        n->setSource(_FPGA_V[stoi(token)]);
        _FPGA_V[stoi(token)]->addUsage();
        while (begin != string::npos)
        {
            begin = getToken(begin, line, token);
            value = atoi(token.c_str());
            n->setTarget(_FPGA_V[value]);
            _FPGA_V[value]->addUsage();
        }
        n->initEdgeTDM(nums[1]);
        _net_V.push_back(n);
    }

    Edge::_AvgWeight = float(nums[2]) / float(nums[1]);
    cout << " Edge average weight:" << Edge::_AvgWeight <<endl;

    // groups
    _group_V.reserve(nums[3]);
    for (int i = 0; i < nums[3]; ++i)
    {
        getline(fs, line);
        NetGroup *g = new NetGroup();
        size_t begin = 0;
        while (begin != string::npos)
        {
            begin = getToken(begin, line, token);
            value = atoi(token.c_str());
            g->addNet(_net_V[value]);
            _net_V[value]->addGroup(g);
        }
        _group_V.push_back(g);
    }
    // calculate fpga distance
    for(size_t i = 0; i < _FPGA_V.size(); ++i){
        vector<unsigned char> vec(_FPGA_V.size(), 0);
        _distance.push_back(vec);
    }

    for(size_t i = 0; i < _FPGA_V.size(); ++i){
        FPGA::setGlobalVisit();
        queue<FPGA*> Q;
        Q.push(_FPGA_V[i]);
        _FPGA_V[i]->setVisited(true);
        _FPGA_V[i]->setParent(NULL);
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
        for(size_t j = i+1; j < _FPGA_V.size(); ++j){
            FPGA* f = _FPGA_V[j]->getParent();
            int distance = 0; 
            while(f != NULL){
                f = f->getParent();
                ++distance; 
            }
            _distance[i][j] = distance;
            _distance[j][i] = distance;
        }
    }


    cout << " Isolated FPGA:";
    for(size_t i = 0; i < _FPGA_V.size(); ++i){
        if(_FPGA_V[i]->getEdgeNum() == 1)
            cout << i << " ";
    }
    cout << endl;
    end = clock();
    cout << " Time used:" << ((double) (end - start)) / CLOCKS_PER_SEC << endl;
    cout << endl;
    return true;
}


void TDM::preRoute(){
    cout << " [pre-routing] " << endl;
    for(size_t i = 0; i < _FPGA_V.size(); ++i){
        double cap = 0.5*(double)_FPGA_V[i]->getUsage() / _FPGA_V[i]->getEdgeNum();
        for(int j = 0; j < _FPGA_V[i]->getEdgeNum(); ++j){
            _FPGA_V[i]->getEdge(j)->addCapacity(cap);
        }
    }


}


// find domimant group
void TDM::findDominantGroup(){
    sort(_group_V.begin(), _group_V.end(), groupCompare);
    for(size_t i = 0; i < _group_V.size(); ++i){
        _avg_net += ((double)_group_V[i]->getNetNum() / _group_V.size());
        _avg_subnet += ((double)_group_V[i]->getSubnetNum() / _group_V.size());
    }
    int i = 0;

    while(_group_V[i]->getSubnetNum() > 3 * _avg_subnet){
        _group_V[i]->setDominant();
        ++_domiantGroupCount;
        ++i;
    }
  
    for(size_t i = 0; i < _net_V.size(); ++i){
        _total_subnet += _net_V[i]->getSubnetNum();
    }

    for(size_t i = 0; i < _group_V.size(); ++i){
        double w = (double)_group_V[i]->getSubnetNum() / _avg_subnet;
        for(int j = 0; j < _group_V[i]->getNetNum(); ++j){
            _group_V[i]->getNet(j)->setWeight(w);
        }
    }

    cout << " Dominant group subnets:" << _group_V[0]->getSubnetNum() <<endl;
    cout << " avg_net:" << _avg_net << " avg_subnet:"<< _avg_subnet 
         << " total subnet:" << _total_subnet << endl;
    cout << endl;
}


// output file
bool TDM::outputFile(const char *fname)
{
    fstream fs(fname, ios::out);


    clock_t start,end;

    if (!fs.is_open())
    {
        return false;
    }

    cout << "\n [outputting] \n";
    start = clock();

    for (size_t i = 0; i < _net_V.size(); i++)
    {
        Net *n = _net_V[i];
        int edgeNum = n->getMin_routeNum();
        fs << edgeNum << endl;

        
        for(size_t i = 0; i < n->Min_edge_tdm.size();i++){
            if(n->Min_edge_tdm[i]!=0){
                fs << i << " "<< n->Min_edge_tdm[i]<<endl;
            }
        }
    }
    fs.close();

    cout << endl;
    end = clock();
    cout << " Time used:" << ((double) (end - start)) / CLOCKS_PER_SEC << endl;
    cout << endl;

    return true;
}

void TDM::decomposeNet()
{
    cout << " [decomposing] \n";

    clock_t start,end;
    int maxInt = numeric_limits<int>::max();
    vector<FPGA*>parent(_FPGA_V.size(),NULL);
    vector<bool>visited(_FPGA_V.size(),false);
    vector<int>key(_FPGA_V.size(),maxInt);

    start = clock();

    for (size_t i = 0; i < _net_V.size(); ++i)
    {
        Net* n = _net_V[i];
        int target_num = n->getTargetNum();
        if(target_num == 1){
            SubNet* sn = new SubNet(n->getSource(), n->getTarget(0));
            n->setSubNet(sn);
            continue;
        }
        //construct MST
        parent.clear();
        visited.clear();
        key.clear();
        parent.resize(_FPGA_V.size(),NULL);
        visited.resize(_FPGA_V.size(),false);
        key.resize(_FPGA_V.size(),maxInt);

        FPGA* source = n->getSource();

        key[source->getId()] = 0;
        parent[source->getId()] = source;
        visited[source->getId()] = true;
        FPGA* targetFPGA = NULL;
        int id = -1;
        for(int k = 0; k < target_num; ++k){
            targetFPGA = n->getTarget(k);
            id = targetFPGA->getId();

            if(visited[id]==false && _distance[source->getId()][id]<key[id]){
                parent[id] = source;
                key[id] =   _distance[source->getId()][id];
            }
        }
        for(int k = 0; k < target_num; ++k){
            int min = maxInt;
            int minidx = -1;
            FPGA* t;
            for(int j = 0; j < target_num; ++j){
                t = n->getTarget(j);
                if( visited[t->getId()] == false && key[t->getId()] < min){
                    min = key[t->getId()];
                    minidx = t->getId();
                }
            }
            FPGA* minFPGA = _FPGA_V[minidx];
            int u = minFPGA->getId();
            visited[u] = true;
            for(int j = 0; j < target_num; ++j){
                targetFPGA = n->getTarget(j);
                if(targetFPGA == minFPGA)continue;
                id = targetFPGA->getId();
                if(visited[id]==false && _distance[u][id]<key[id]){
                    parent[id] = minFPGA;
                    key[id] =   _distance[u][id];
                }
            }
        }
        vector< vector<FPGA*> >twopin;
        for(size_t i = 0; i < _FPGA_V.size(); ++i){
            vector<FPGA*> s;
            twopin.push_back(s);
        }
        for(int k = 0; k < target_num; ++k){
            twopin[ parent[n->getTarget(k)->getId()]->getId() ].push_back(n->getTarget(k));
        }
        stack<int> popstack;
        FPGA* popFPGA;
        int popidx = source->getId();
        popstack.push(popidx);
        while(!popstack.empty()){
            popidx = popstack.top();
            popstack.pop();
            for(size_t j = 0; j < twopin[popidx].size(); ++j){
                popFPGA = twopin[popidx][j];
                SubNet* sn = new SubNet(_FPGA_V[popidx],popFPGA);
                n->setSubNet(sn);
                popstack.push(popFPGA->getId());
            }
            
        }
    }

    cout << endl;
    end = clock();
    cout << " Time used:" << ((double) (end - start)) / CLOCKS_PER_SEC << endl;
    cout << endl;
}

void TDM::showStatus(const char* fname)
{
    int total_edge = 0;
    cout << "=================[ Info ]================= \n";
    cout << " design name | " << fname << endl;
    cout << "    #FPGAs   | " << _FPGA_V.size() << endl;
    cout << "    #edge    | " << _edge_V.size() << endl;
    cout << "    #nets    | " << _net_V.size() << endl;
    cout << " #net groups | " << _group_V.size() << endl;
    cout << "\n [NetGroup Info]\n";
    for(size_t i = 0; i < _edge_V.size(); ++i){
        total_edge += _edge_V[i]->getNetNum();
    }
    cout << "  dominant group use edges:" << total_edge << endl;


    double avg_tdm = 0.0;
    int cnt = 0;
    set<pLG> sortedGroup;
    for(size_t i = 0; i < _group_V.size(); ++i){
        sortedGroup.insert(pLG(_group_V[i]->getTDM(), _group_V[i]));
        avg_tdm += ((double)_group_V[i]->getTDM() / (int)_group_V.size());
    }

    for(auto it = sortedGroup.rbegin(); it != sortedGroup.rend(); ++it){
        int edgeNum = 0;
        for(int i = 0; i < it->second->getNetNum(); ++i){
            edgeNum += it->second->getNet(i)->getMin_routeNum();
        }
        double ratio = (double)it->second->getTDM() / edgeNum;
        printf(" tdm:%lld net/subnet: %d/%d ratio:%f \n", it->second->getTDM(), 
                it->second->getNetNum(), it->second->getSubnetNum(), ratio);
        if(++cnt > 20)
            break;
    }
    cout << " ......  " <<endl;
    cnt = 0;
    for(auto it = sortedGroup.begin(); it != sortedGroup.end(); ++it){
        int edgeNum = 0;
        for(int i = 0; i < it->second->getNetNum(); ++i){
            edgeNum += it->second->getNet(i)->getMin_routeNum();
        }
        double ratio = (double)it->second->getTDM() / edgeNum;
        printf(" tdm:%lld net/subnet: %d/%d ratio:%f \n", it->second->getTDM(), 
        it->second->getNetNum(), it->second->getSubnetNum(), ratio);
        if(++cnt > 20)
            break;
    }
    cout << "[ avg  ] tdm:" << avg_tdm << " net:" << _avg_net << " subnet:"<< _avg_subnet << endl;
    if (verbose)
    {
        for (size_t i = 0; i < _FPGA_V.size(); ++i)
        {
            _FPGA_V[i]->showInfo();
        }

        cout << endl;
        for (size_t i = 0; i < _net_V.size(); ++i)
        {
            _net_V[i]->showInfo();
        }
    }
}

void TDM::global_router(char* fname)
{
    //phase1
    cout << " [routing] " <<endl;
    long long int minimumTDM = numeric_limits<long long int>::max();
    int iteration = 0, terminateCount = 0, section = 1;

    _pathcheck_V.reserve(_FPGA_V.size());

    set<pIN> sorted_net;
    for (size_t i = 0; i < _net_V.size(); i++)
    {
        int sub_net = 0;
        for (int j = 0; j < _net_V[i]->getGroupSize(); ++j)
        {
            sub_net = max(sub_net, _net_V[i]->getNetGroup(j)->getSubnetNum());
        }
        sorted_net.insert(pIN(sub_net + _net_V[i]->getSubnetNum(), _net_V[i]));
    }    
    
    while(true){
        clock_t start, end;
        start = clock();
        if(iteration == 0){
            //Initialize Edge's congestion to zero every iteration
            for (size_t i = 0; i < _edge_V.size(); i++){
                _edge_V[i]->initCongestion();
                _edge_V[i]->resetNet();
            }
            //Use shortest path algorithm to route all net
            local_router(true, sorted_net);
            local_router(false, sorted_net);
        }


        for (size_t i = 0; i < _net_V.size(); i++) {
            _net_V[i]->clearEdgeTDM();
        }
        // Distribute all TDM and calculate all TDM
        {
            ctpl::thread_pool p(8);
            for (size_t i = 0; i < _edge_V.size(); i++){
                p.push([](int id, Edge* e){e->distributeTDM();}, _edge_V[i]);
            }
        }
        
        for (size_t i = 0; i < _net_V.size(); i++){
            _net_V[i]->calculateTDM();
        }

        long long int maxGroupTDM = 0;
        int group_id = -1;
        double avg_group_tdm = 0;

        for (size_t i = 0; i < _group_V.size(); i++){
            _group_V[i]->updateTDM();
            if(_group_V[i]->getTDM() > maxGroupTDM){
                maxGroupTDM = _group_V[i]->getTDM();
                group_id = i;
            }
            avg_group_tdm += _group_V[i]->getTDM();
        }
        avg_group_tdm /= (int)_group_V.size();
        Net::setAvgGroupTDM(avg_group_tdm);

        for(size_t i = 0; i < _net_V.size(); i++){
            double x = 0.0;
            for(int j = 0; j < _net_V[i]->getGroupSize(); j++){
                x = max(x,_net_V[i]->getNetGroup(j)->getTDM() / avg_group_tdm);
            }
            double prev_x = _net_V[i]->getX();
            _net_V[i]->setX(x*prev_x);
        }
        //Terminate condition : Compare with minimum answer. If we can't update  minimum answer more than N times, terminate global router.
        if ( maxGroupTDM < minimumTDM) {

            minimumTDM = maxGroupTDM;
        }
        else{
            terminateCount++;
            if (terminateCount > 3){
                //update minimum answer
                for (size_t i = 0; i < _net_V.size(); i++){
                    _net_V[i]->updateMin_route();
                    _net_V[i]->updateMin_edge_TDM();
                }
                
                for (size_t i = 0; i < _net_V.size(); i++){
                    _net_V[i]->calculateMinTDM();
                }
                for (size_t i = 0; i < _group_V.size(); i++){
                    _group_V[i]->updateTDM();
                }
                break;
            }       
        }

        end = clock();
        double t =  ((double) (end - start)) / CLOCKS_PER_SEC;
        printf(" #%d section:%d  current: %lld (id:%d)| min: %lld time:%.3f \n", 
        iteration, section, maxGroupTDM, group_id, minimumTDM, t);
        if(iteration++ > 100){
            for (size_t i = 0; i < _net_V.size(); i++){
                _net_V[i]->updateMin_route();
                _net_V[i]->updateMin_edge_TDM();
            }
            break;
        }
            
    }
    cout << endl;
}

// cut a string from space
size_t TDM::getToken(size_t pos, string &s, string &token)
{
    size_t begin = s.find_first_not_of(' ', pos);
    if (begin == string::npos)
    {
        token = "";
        return begin;
    }
    size_t end = s.find_first_of(' ', begin);
    token = s.substr(begin, end - begin);
    return end;
}

struct pDFcomp {
    bool operator() (const pDF& lhs, const pDF& rhs) const
    {
        if(lhs.first == rhs.first) return lhs.second->getId() < rhs.second->getId();
        return lhs.first < rhs.first;
    }
 };


void TDM::buildMST(Net* n){

    FPGA::setGlobalVisit();
    int c = n->isDominant() ? 2: 1;
    double maxVal = numeric_limits<double>::max();
    vector<double> key(_FPGA_V.size(), maxVal);
    vector<pFE> parent(_FPGA_V.size());
    my_pq Q;
    my_pq::point_iterator iter_V [_FPGA_V.size()];

    // Prim MST algorithm
    FPGA* source = n->getSource();
    source->setParent(NULL);
    key[source->getId()] = 0;
    for(size_t i = 0; i < _FPGA_V.size(); ++i){
        iter_V[_FPGA_V[i]->getId()] = Q.push(pDF(maxVal, _FPGA_V[i]));
    }
    Q.modify(iter_V[source->getId()], pDF(0, source));
    while(!Q.empty()){
        pDF top = Q.top();
        _FPGA_V[top.second->getId()]->setVisited(true);
        Q.pop();
        for(int i = 0; i < top.second->getEdgeNum(); ++i){
            FPGA* f = top.second->getConnectedFPGA(i);
            Edge* e = top.second->getEdge(i);
            double w = e->getWeight();
            if(!_FPGA_V[f->getId()]->isVisited() && w < key[f->getId()]){
                parent[f->getId()] = pFE(top.second, e);
                key[f->getId()] = w; 
                Q.modify(iter_V[f->getId()], pDF(w, f));
            }
        }
    }

    // trace back from each target to source
    for(int i = 0; i < n->getTargetNum(); ++i){
        FPGA* f = n->getTarget(i);
        while (f != source)
        {
            FPGA* connectFPGA = parent[f->getId()].first;
            Edge* connectEdge = parent[f->getId()].second;
            _pathcheck_V[connectFPGA->getId()] = true;
            n->addEdgetoCur_route(connectEdge);
            connectEdge->addCongestion(c);
            connectEdge->addNet(n);
            f = parent[f->getId()].first;
        }
    }
}

//single source single target shortest path algortihm
void TDM::Dijkstras(FPGA *source, FPGA *target, unsigned int num, Net* n)
{
    double maxf = numeric_limits<double>::max();
    int c = n->isDominant() ? 2: 1;
    my_pq Q;
    my_pq::point_iterator iter_V [num];
    vector<double> d(num, maxf); //distance
    vector<pFE> parent(num);    //Record the parentId and edge of each FPGA after Dijkstras

    d[source->getId()] = 0.0;
    parent[source->getId()] = pFE(source, NULL);
    iter_V[source->getId()] = Q.push(pDF(0, source));

    FPGA *a = NULL;
    while (!Q.empty())
    {
        pDF top = Q.top();
        Q.pop();
        
        a = top.second;
        if (a == target){
            break; //Find the target
        }
        else if (_pathcheck_V[a->getId()]){
            break; // Find the steiner point
        }
        double w;
        FPGA *b;
        Edge *e;
        for (int i = 0; i < a->getEdgeNum(); i++)
        {
            b = a->getConnectedFPGA(i);
            e = a->getEdge(i);
            w = e->getWeight() + _distance[a->getId()][source->getId()] ;

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

    _pathcheck_V[a->getId()] = true;
    while (a != parent[a->getId()].first)
    {
        FPGA* connectFPGA = parent[a->getId()].first;
        Edge* connectEdge = parent[a->getId()].second;
        _pathcheck_V[connectFPGA->getId()] = true;
        connectEdge->addCongestion(c);
        n->addEdgetoCur_route(connectEdge);
        connectEdge->addNet(n);

        a = parent[a->getId()].first;
    }
}


void TDM::ripup_reroute(set<pIN>& sorted_net)
{
    for (auto it = sorted_net.begin(); it != sorted_net.end(); ++it){
        Net *n = it->second;
        if(!n->isDominant()){
            continue;
        }

        for(int j = 0; j < n->getCur_routeNum(); ++j){
            // n->getCur_route(j)->removeNet(n);
            // n->getCur_route(j)->addCongestion(c*(-1));
        }

        // Net *n = _net_V[i];
        _pathcheck_V.clear();
        _pathcheck_V.resize(_FPGA_V.size(), false);
        n->initializeCur_route();
        // cout<<"Net "<<n->getId()<<endl;
        for (int j = 0; j < n->getSubnetNum(); j++)
        {
            SubNet *sn = n->getSubNet(j);
            FPGA *source = sn->getSource();
            FPGA *target = sn->getTarget();
            if (_pathcheck_V[source->getId()] && _pathcheck_V[target->getId()])
                continue;
            else if (_pathcheck_V[source->getId()] && !_pathcheck_V[target->getId()])
            { // We can swap source and target in order to efficiently find steiner point
                FPGA *temp = source;
                source = target;
                target = temp;
            }
            //stack<pFE> route_S = Dijkstras(source, target, _FPGA_V.size());
            // stack<pFE> route_S;
            Dijkstras(source, target, _FPGA_V.size(), n);

            if (!_pathcheck_V[target->getId()]){ 
                // target is not connected
                Dijkstras(target, source, _FPGA_V.size(), n);
            }
        }
    }

}

//route all nets by Dijkstras algorithm
void TDM::local_router(bool b, set<pIN>& sorted_net)
{
    // Route from the smallest/largest net group tdm value
    for (auto it = sorted_net.begin(); it != sorted_net.end(); ++it){
        Net *n = it->second;
        // int c = n->isDominant() ? 2: 1;
        if(n->isDominant() && !b)
            continue;
        else if(!n->isDominant() && b)
            continue;
        _pathcheck_V.clear();
        _pathcheck_V.resize(_FPGA_V.size(), false);
        n->initializeCur_route();
        for (int j = 0; j < n->getSubnetNum(); j++)
        {
            SubNet *sn = n->getSubNet(j);
            FPGA *source = sn->getSource();
            FPGA *target = sn->getTarget();
            if (_pathcheck_V[source->getId()] && _pathcheck_V[target->getId()])
                continue;
            else if (_pathcheck_V[source->getId()] && !_pathcheck_V[target->getId()])
            { // We can swap source and target in order to find steiner point efficiently
                FPGA *temp = source;
                source = target;
                target = temp;
            }
            Dijkstras(source, target, _FPGA_V.size(), n);
            if (!_pathcheck_V[target->getId()])
            { // target is not connected
                Dijkstras(target, source, _FPGA_V.size(), n);
            }
        }
    }
}