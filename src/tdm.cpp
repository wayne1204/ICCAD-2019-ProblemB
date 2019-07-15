#include <fstream>
#include <iostream>
#include <set>
#include <math.h>
#include <limits>
#include <set>
#include "tdm.h"

using namespace std;

// parse input file
bool TDM::parseFile(const char *fname)
{
    int nums[4];
    string line, token;
    fstream fs(fname, ios::in);

    if (!fs.is_open())
    {
        return false;
    }
    cout << " [parsing] \n";
    for (int i = 0; i < 4; ++i)
    {
        fs >> token;
        nums[i] = stoi(token);
    }
    getline(fs, line);

    // fpga
    _FPGA_V.reserve(nums[0]);
    for (int i = 0; i < nums[0]; ++i)
    {
        _FPGA_V.push_back(new FPGA(i));
    }

    // edge
    _edge_V.reserve(nums[1]);
    for (int i = 0; i < nums[1]; ++i)
    {
        size_t begin = 0;
        unsigned f1, f2;
        getline(fs, line);
        begin = getToken(0, line, token);
        f1 = stoi(token);
        begin = getToken(begin, line, token);
        f2 = stoi(token);
        Edge *e = new Edge(i, _T);
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
        Net *n = new Net(i);
        size_t begin = getToken(0, line, token);
        n->setSource(_FPGA_V[stoi(token)]);
        while (begin != string::npos)
        {
            begin = getToken(begin, line, token);
            n->setTarget(_FPGA_V[stoi(token)]);
        }
        _net_V.push_back(n);
    }

    Edge::_AvgWeight = float(nums[2]) / float(nums[1]);

    // groups
    _net_V.reserve(nums[3]);
    for (int i = 0; i < nums[3]; ++i)
    {
        getline(fs, line);
        NetGroup *g = new NetGroup(i);
        size_t begin = 0;
        while (begin != string::npos)
        {
            begin = getToken(begin, line, token);
            g->addNet(_net_V[stoi(token)]);
            _net_V[stoi(token)]->addGroup(g);
        }
        _group_V.push_back(g);
    }
    return true;
}

//output file
bool TDM::outputFile(const char *fname)
{
    fstream fs(fname, ios::out);

    if (!fs.is_open())
    {
        return false;
    }
    for (unsigned int i = 0; i < _net_V.size(); i++)
    {
        Net *n = _net_V[i];
        int edgeNum = n->getMin_routeNum();
        fs << edgeNum << endl;
        pair<int, int> p;
        for (map<int, int>::iterator it = n->Min_edge_tdm.begin(); it != n->Min_edge_tdm.end(); ++it)
        {
            p = *it;
            fs << p.first << " " << p.second << endl;
        }
    }
    fs.close();
    return true;
}

void TDM::decomposeNet()
{
    cout << " [decomposing] \n";
    for (unsigned int i = 0; i < _net_V.size(); ++i)
    {
        for (unsigned int j = 0; j < _FPGA_V.size(); ++j)
        {
            _FPGA_V[j]->setVisited(false);
        }
        _net_V[i]->decomposition();
    }
}

void TDM::showStatus()
{
    cout << " ============[ status ]============ \n";
    cout << "    #FPGAs   | " << _FPGA_V.size() << endl;
    cout << "    #edge    | " << _edge_V.size() << endl;
    cout << "    #nets    | " << _net_V.size() << endl;
    cout << " #net groups | " << _group_V.size() << endl;

    cout << endl;
    cout << "net group size\n";
    set<pLG> sortedGroup;
    double avg_tdm = 0.0;
    int cnt = 0;
    for(size_t i = 0; i < _group_V.size(); ++i){
        sortedGroup.insert(pLG(_group_V[i]->getTDM(), _group_V[i]));
        avg_tdm += ((double)_group_V[i]->getTDM() / (int)_group_V.size());
    }
    for(auto it = sortedGroup.rbegin(); it != sortedGroup.rend(); ++it){
        cout << "tdm:"<<it->second->getTDM() << " #:"<< it->second->getNetNum() << endl;
        if(++cnt > 10)
            break;
    }
    cout << " avg:" << avg_tdm << endl;
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

void TDM::global_router()
{

    //phase1
    int iteration = 0;
    int minimumTDM = numeric_limits<int>::max();
    int terminateConditionCount = 0;
    _pathcheck_V.reserve(_FPGA_V.size());
    while (1) {
        cout << " --------\n";
        cout << "iteration : " << iteration << endl;
        //Initialize Edge's congestion to zero every iteration
        for (unsigned int i = 0; i < _edge_V.size(); i++){
            _edge_V[i]->initializeCongestion();
            _edge_V[i]->resetNET();
        }

        //Use shortest path algorithm to route all nets
        local_router();

        for (unsigned int i = 0; i < _net_V.size(); i++) {
            _net_V[i]->clearEdgeTDM();
        }

        //Distribute all TDM and calculate all TDM
        for (unsigned int i = 0; i < _edge_V.size(); i++){
            _edge_V[i]->distributeTDM();
        }

        for (unsigned int i = 0; i < _net_V.size(); i++){
            _net_V[i]->calculateTDM();
        }

        long long int maxGroupTDM = 0;

        for (unsigned int i = 0; i < _group_V.size(); i++){
            _group_V[i]->updateTDM();
            int t = _group_V[i]->getTDM();
            if (t > maxGroupTDM)
                maxGroupTDM = t;
        }

        //Terminate condition : Compare with minimum answer. If we can't update  minimum answer more than N times, terminate global router.
        if (maxGroupTDM < minimumTDM) {
            //update minimum answer
            minimumTDM = maxGroupTDM;
            for (unsigned int i = 0; i < _net_V.size(); i++){
                _net_V[i]->updateMin_route();
                _net_V[i]->updateMin_edge_TDM();
            }
            terminateConditionCount = 0;
        }
        else{
            terminateConditionCount++;
            if (terminateConditionCount > 3){
                for (unsigned int i = 0; i < _net_V.size(); i++){
                    _net_V[i]->calculateminTDM();
                }

                for (unsigned int i = 0; i < _group_V.size(); i++){
                    _group_V[i]->updateTDM();
                }
                
                break;
            }
                
        }

        //Update edge's weight for next iteration
        for (unsigned int i = 0; i < _edge_V.size(); i++) {
            _edge_V[i]->updateWeight(iteration);
        }
        iteration++;
    }
    cout << "phase1 use " << iteration << " iteration" << endl;
    //Set Net information to Edge before phase2
    // for (unsigned int i = 0; i < _edge_V.size(); i++){
    //     _edge_V[i]->initializeCongestion();
    // }
    // for (unsigned int i = 0; i < _net_V.size(); i++){
    //     _net_V[i]->setMin_routetoEdge();
    // }
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

// struct pIFcomp {
//   bool operator() (const pIF& lhs, const pIF& rhs) const
//   {return lhs.first < rhs.first;}
// };

//single source single target shortest path
stack<pFE> TDM::Dijkstras(FPGA *source, FPGA *target, unsigned int num)
{
    set<pDF> Q;
    float maxf = numeric_limits<float>::max();
    vector<float> d(num, maxf); //distance
    vector<pFE> parent(num);    //Record the parentId and edge of each FPGA after Dijkstras
    d[source->getId()] = 0.0;
    parent[source->getId()] = pFE(source, NULL);
    Q.insert(pDF(d[source->getId()], source));

    FPGA *a = NULL;
    while (!Q.empty())
    {
        pIF top = *Q.begin();
        Q.erase(Q.begin());
        a = top.second;
        if (a == target)
            break; //Find the target
        else if (_pathcheck_V[a->getId()])
            break; // Find the steiner point

        float w;
        FPGA *b;
        Edge *e;
        for (int i = 0; i < a->getEdgeNum(); i++)
        {
            b = a->getConnectedFPGA(i);
            e = a->getEdge(i);
            w = e->getWeight();
            if (d[a->getId()] + w < d[b->getId()])
            {
                if (d[b->getId()] != maxf)
                {
                    Q.erase(Q.find(pDF(d[b->getId()], b)));
                }
                d[b->getId()] = d[a->getId()] + w;
                parent[b->getId()] = pFE(a, e);
                Q.insert(pDF(d[b->getId()], b));
            }
        }
    }
    stack<pFE> route;
    route.push(pFE(a, NULL));
    while (a != parent[a->getId()].first)
    {
        route.push(parent[a->getId()]);
        a = parent[a->getId()].first;
    }
    return route;
}

//route all the nets by Dijkstras algorithm
void TDM::local_router()
{
    set<pIN> sorted_net;
    for (unsigned int i = 0; i < _net_V.size(); i++)
    {
        long long int max_tdm = 0;
        for (int j = 0; j < _net_V[i]->getGroupSize(); ++j)
        {
            max_tdm = max(max_tdm, _net_V[i]->getNetGroup(j)->getTDM());
        }
        sorted_net.insert(pIN(max_tdm, _net_V[i]));
    }
    // Route from the smallest/largest net group tdm value
    for (auto rit = sorted_net.rbegin(); rit != sorted_net.rend(); ++rit)
    {
        Net *n = rit->second;
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
            stack<pFE> route_S = Dijkstras(source, target, _FPGA_V.size());

            FPGA *connectFPGA;
            Edge *connectEdge;
            while (!route_S.empty())
            {
                pFE p = route_S.top();
                route_S.pop();
                connectFPGA = p.first;
                connectEdge = p.second;
                _pathcheck_V[connectFPGA->getId()] = true;
                if (connectEdge != NULL)
                {
                    connectEdge->addCongestion();
                    n->addEdgetoCur_route(connectEdge);
                    connectEdge->addNet(n);
                    // cout<<connectEdge->getId()<<" "<<connectEdge->getCongestion()<<endl;
                }
            }
            if (!_pathcheck_V[target->getId()])
            { // target is not connected
                route_S = Dijkstras(target, source, _FPGA_V.size());
                while (!route_S.empty())
                {
                    pFE p = route_S.top();
                    route_S.pop();
                    connectFPGA = p.first;
                    connectEdge = p.second;
                    _pathcheck_V[connectFPGA->getId()] = true;
                    if (connectEdge != NULL)
                    {
                        connectEdge->addCongestion();
                        n->addEdgetoCur_route(connectEdge);
                        connectEdge->addNet(n);
                    }
                }
            }
        }
    }
}
