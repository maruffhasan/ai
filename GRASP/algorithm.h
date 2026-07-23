#include <bits/stdc++.h>

using namespace std;


using Partition = vector<int>;
/*
    0 -> X set
    1 -> Y set
*/

struct Graph {
    int n = 0;
    int m = 0;
    vector<vector<pair<int,int>>> adj; 

    void load(const string &path) {
        ifstream fin(path);
        fin >> n >> m;
        adj.assign(n + 1, {});

        int u, v, w;
        while (fin >> u >> v >> w) {
            adj[u].push_back({v, w});
            adj[v].push_back({u, w});
        }
    }
};


long long cutWeight(const Graph &g, const Partition &part);
Partition randomPartition(const Graph &g, mt19937 &prng);
pair<long long, int> grasp(const Graph &g, int maxIterations, double alpha, mt19937 &prng);
double randomizedMaxCut(const Graph &g, int runs, mt19937 &prng);
Partition greedyConstruct(const Graph &g);
inline Partition semiGreedyConstruct(const Graph &g, double alpha, mt19937 &prng);
pair<Partition, int> localSearch(const Graph &g, Partition part);



long long cutWeight(const Graph &g, const Partition &part) {
    long long total = 0;
    for (int u = 1; u <= g.n; u++) {
        for (auto [v, w] : g.adj[u]) {
            if (u < v && part[u] != part[v]) total += w;
        }
    }
    return total;
}

Partition randomPartition(const Graph &g, mt19937 &prng) {
    uniform_int_distribution<int> coin(0, 1);
    Partition part(g.n + 1);
    for (int v = 1; v <= g.n; v++) part[v] = coin(prng);
    return part;
}


// GRASP
pair<long long, int> grasp(const Graph &g, int maxIterations, double alpha, mt19937 &prng) {
    long long best = LLONG_MIN, itCounts = 0;
    Partition bestPart;
    for (int it = 0; it < maxIterations; it++) {
        auto part = semiGreedyConstruct(g, alpha, prng);
        auto pr = localSearch(g, part);
        long long val = cutWeight(g, pr.first);
        itCounts += pr.second;
        if (val > best) {
            best = val;
            bestPart = part;
        }
    }
    return {best, itCounts / maxIterations};
}
 

// Randomized Heuristic
double randomizedMaxCut(const Graph &g, int runs, mt19937 &prng) {
    long long total = 0;
    for (int i = 0; i < runs; i++) {
        Partition part = randomPartition(g, prng);
        total += cutWeight(g, part);
    }
    return (double)total / runs;
}
 

// Greedy Heuristic
Partition greedyConstruct(const Graph &g) {
    Partition part(g.n + 1, -1); // -1 -> unassigned 
 
    int bu = -1, bv = -1, bw = INT_MIN;
    for (int u = 1; u <= g.n; u++) {
        for (auto &pr : g.adj[u]) {
            int v = pr.first, w = pr.second;
            if (w > bw) { 
                bw = w; bu = u; bv = v;
            }
        }
    }
    if (bu == -1) {
        for (int v = 1; v <= g.n; v++) part[v] = 0;
        return part;
    }
    part[bu] = 0; 
    part[bv] = 1;
 
    vector<long long> W_X(g.n + 1, 0), W_Y(g.n + 1, 0);
    for (auto &pr : g.adj[bu]) W_Y[pr.first] += pr.second; 
    for (auto &pr : g.adj[bv]) W_X[pr.first] += pr.second; 
 
    for (int z = 1; z <= g.n; z++) {
        if (part[z] != -1) continue; 
        int side = (W_X[z] >= W_Y[z]) ? 0 : 1; 
        part[z] = side;
        for (auto &pr : g.adj[z]) {
            int nb = pr.first, w = pr.second;
            if (part[nb] == -1) {
                if (side == 0) W_Y[nb] += w;
                else W_X[nb] += w;
            }
        }
    }
    return part;
}


// semi greedy heuristic
inline Partition semiGreedyConstruct(const Graph &g, double alpha, mt19937 &prng) {
    Partition part(g.n + 1, -1);
 
    int bu = -1, bv = -1, bw = INT_MIN; // bw -> max weight
    for (int u = 1; u <= g.n; u++) {
        for (auto &pr : g.adj[u]) {
            int v = pr.first, w = pr.second;
            if (w > bw) { bw = w; bu = u; bv = v; }
        }
    }
    if (bu == -1) {
        for (int v = 1; v <= g.n; v++) part[v] = 0;
        return part;
    }
    part[bu] = 0;
    part[bv] = 1;
 
    vector<long long> sigmaX(g.n + 1, 0), sigmaY(g.n + 1, 0);
    for (auto &pr : g.adj[bu]) sigmaX[pr.first] += pr.second;
    for (auto &pr : g.adj[bv]) sigmaY[pr.first] += pr.second;
 
    int remaining = g.n - 2;
    
    while (remaining--) {
        vector<int> candidates;
        for (int v = 1; v <= g.n; v++) {
            if (part[v] == -1) candidates.push_back(v);
        }

        long long wmin = LLONG_MAX, wmax = LLONG_MIN;
        for (int v : candidates) {
            wmin = min({wmin, sigmaX[v], sigmaY[v]});
            wmax = max({wmax, sigmaX[v], sigmaY[v]});
        }
        double mu = wmin + alpha * (double)(wmax - wmin);
 
        vector<int> rcl;
        for (int v : candidates) {
            long long gval = max(sigmaX[v], sigmaY[v]);
            if (gval >= mu) rcl.push_back(v);
        }
         
        uniform_int_distribution<int> pick(0, rcl.size() - 1);
        int chosen = rcl[pick(prng)];
 
        int side = (sigmaX[chosen] >= sigmaY[chosen]) ? 1 : 0;
        part[chosen] = side;
  
        for (auto &pr : g.adj[chosen]) {
            int nb = pr.first, w = pr.second;
            if (part[nb] == -1) {
                if (side == 0) sigmaX[nb] += w;
                else sigmaY[nb] += w;
            }
        }
    }
    return part;
}
 

// Local search
pair<Partition, int> localSearch(const Graph &g, Partition part) {
    vector<long long> delta(g.n + 1, 0);
    for (int v = 1; v <= g.n; v++) {
        long long same = 0, diff = 0;
        for (auto &pr : g.adj[v]) {
            int u = pr.first, w = pr.second;
            if (part[u] == part[v]) same += w; 
            else diff += w;
        }
        delta[v] = same - diff;
    }
 
    long long it = 0;
    bool improved = true;
    while (improved) {
        it++;
        improved = false;
        int bestV = -1;
        long long bestDelta = 0;
        for (int v = 1; v <= g.n; v++) {
            if (delta[v] > bestDelta) { 
                bestDelta = delta[v]; 
                bestV = v; 
            }
        }
        if (bestV != -1 && bestDelta > 0) {
            int oldSide = part[bestV];
            part[bestV] = oldSide ^ 1;
            for (auto &pr : g.adj[bestV]) {
                int u = pr.first, w = pr.second;
                if (part[u] == oldSide) {
                    delta[u] -= 2 * w;
                } else {
                    delta[u] += 2 * w;
                }
            }
            delta[bestV] = -bestDelta; 
            improved = true;
        }
    }
    return {part, it};
}   
