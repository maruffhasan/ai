#include <bits/stdc++.h>

using namespace std;

using Partition = vector<int>;

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

long long cutWeight(const Graph &g, const Partition &part) {
    long long total = 0;
    for (int u = 1; u <= g.n; u++) {
        for (auto &pr : g.adj[u]) {
            int v = pr.first, w = pr.second;
            if (u < v && part[u] != part[v]) total += w;
        }
    }
    return total;
}

Partition randomPartition(const Graph &g, mt19937 &rng) {
    uniform_int_distribution<int> coin(0, 1);
    Partition part(g.n + 1);
    for (int v = 1; v <= g.n; v++) part[v] = coin(rng);
    return part;
}


// Randomized Heuristic
double randomizedMaxCut(const Graph &g, int runs, mt19937 &rng) {
    uniform_int_distribution<int> coin(0, 1);
    long long total = 0;
    for (int i = 0; i < runs; i++) {
        Partition part = randomPartition(g, rng);
        total += cutWeight(g, part);
    }
    return (double)total / runs;
}
 

// Greedy Heuristic
Partition greedyConstruct(const Graph &g) {
    Partition part(g.n + 1, -1); // -1 = unassigned
 
    //Find edge (u, v) ∈ E with maximum weight w_uv
    int bu = -1, bv = -1, bw = INT_MIN;
    for (int u = 1; u <= g.n; u++) {
        for (auto &pr : g.adj[u]) {
            int v = pr.first, w = pr.second;
            if (w > bw) { bw = w; bu = u; bv = v; }
        }
    }
    if (bu == -1) { // no edges at all
        for (int v = 1; v <= g.n; v++) part[v] = 0;
        return part;
    }
    part[bu] = 0; // X
    part[bv] = 1; // Y
 
    vector<long long> W_X(g.n + 1, 0), W_Y(g.n + 1, 0);
    for (auto &pr : g.adj[bu]) W_Y[pr.first] += pr.second; // weight to X (bu)
    for (auto &pr : g.adj[bv]) W_X[pr.first] += pr.second; // weight to Y (bv)
 
    for (int z = 1; z <= g.n; z++) {
        if (part[z] != -1) continue; // already bu or bv
        int side = (W_X[z] >= W_Y[z]) ? 0 : 1; // tie -> X
        part[z] = side;
        // update sigma sums for still-unassigned neighbors of z
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
inline Partition semiGreedyConstruct(const Graph &g, double alpha, mt19937 &rng) {
    Partition part(g.n + 1, -1);
 
    // Step 1: max weight edge -> seeds X and Y
    int bu = -1, bv = -1, bw = INT_MIN;
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
        int chosen = rcl[pick(rng)];
 
        int side = (sigmaX[chosen] >= sigmaY[chosen]) ? 0 : 1;
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

