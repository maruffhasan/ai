#include "algorithm.h"
#include <filesystem>
#include <map>

using namespace std;

namespace fs = filesystem;

// tunable defaults
static const int RANDOM_RUNS      = 30;   
static const int LOCAL_ITERS      = 30;   
static const int GRASP_ITERS      = 60;   
static const double SEMI_ALPHA    = 0.5; 

map<string, long long> knownBest = {
    {"g1", 12078}, {"g2", 12084}, {"g3", 12077},
    {"g11", 627}, {"g12", 621}, {"g13", 645},
    {"g14", 3187}, {"g15", 3169}, {"g16", 3172},
    {"g22", 14123}, {"g23", 14129}, {"g24", 14131},
    {"g32", 1560}, {"g33", 1537}, {"g34", 1541},
    {"g35", 8000}, {"g36", 7996}, {"g37", 8009},
    {"g43", 7027}, {"g44", 7022}, {"g45", 7020},
    {"g48", 6000}, {"g49", 6000}, {"g50", 5988}
};


struct RowResult {
    string name;
    int n = 0, m = 0;
    double randomizedAvg = 0;
    long long greedyVal = 0;
    long long semiGreedyVal = 0;
    int localIters = 0;
    double localAvg = 0;
    int graspIters = 0;
    long long graspBest = 0;
    long long known = -1; // -1 = not available
};

RowResult runAll(const string &path, const string &name, mt19937 &prng) {
    Graph g;
    g.load(path);

    RowResult r;
    r.name = name;
    r.n = g.n;
    r.m = g.m;

    r.randomizedAvg = randomizedMaxCut(g, RANDOM_RUNS, prng);

    Partition gp = greedyConstruct(g);
    r.greedyVal = cutWeight(g, gp);

    Partition sgp = semiGreedyConstruct(g, SEMI_ALPHA, prng);
    r.semiGreedyVal = cutWeight(g, sgp);

    

    long long sumLocal = 0, itCounts = 0;
    for (int i = 0; i < LOCAL_ITERS; i++) {
        Partition part = randomPartition(g, prng);
        auto pr = localSearch(g, part);
        sumLocal += cutWeight(g, pr.first);
        itCounts += pr.second;
    }
    r.localIters = itCounts / LOCAL_ITERS;
    r.localAvg = sumLocal / LOCAL_ITERS;

    auto pr = grasp(g, GRASP_ITERS, SEMI_ALPHA, prng);
    r.graspIters = pr.second;
    r.graspBest = pr.first;

    auto it = knownBest.find(name);
    r.known = (it != knownBest.end()) ? it->second : -1;

    return r;
}

void printCsvHeader(ostream &os) {
    os << "Name,|V| or n,|E| or m,"
       << "Randomized-1 (avg),Greedy-1,Semi-greedy-1 (alpha=" << SEMI_ALPHA << "),"
       << "Local-1 iterations,Local-1 average value,"
       << "GRASP-1 iterations,GRASP-1 best value,"
       << "Known best solution or upper bound\n";
}

void printCsvRow(ostream &os, const RowResult &r) {
    os << r.name << "," << r.n << "," << r.m << ","
       << fixed << setprecision(2) << r.randomizedAvg << ","
       << r.greedyVal << ","
       << r.semiGreedyVal << ","
       << r.localIters << "," << r.localAvg << ","
       << r.graspIters << "," << r.graspBest << ",";
    if (r.known >= 0) os << r.known;
    os << "\n";
}

int main(int argc, char **argv) {
    if (argc < 2) {
        cerr << "Usage:\n"
                  << "  " << argv[0] << " batch <input_dir> <output_csv>\n"
                  << "  " << argv[0] << " single <graph_file> <algorithm>\n"
                  << "     algorithm in {randomized, greedy, semigreedy, localsearch, grasp}\n";
        return 1;
    }

    string mode = argv[1];

    if (mode == "batch") { // <input_dir> <output_csv>
        string inDir = argv[2];
        string outCsv = argv[3];
        unsigned seed = 42;
        mt19937 prng(seed);

        vector<fs::path> files;
        for (auto &entry : fs::directory_iterator(inDir)) {
            if (entry.path().extension() == ".rud") files.push_back(entry.path());
        }
        sort(files.begin(), files.end(), [](const fs::path &a, const fs::path &b) {
            auto num = [](const fs::path &p) {
                string s = p.stem().string(); 
                return stoi(s.substr(1));
            };
            return num(a) < num(b);
        });

        ofstream fout(outCsv);
        printCsvHeader(fout);

        for (auto &f : files) {
            string name = f.stem().string(); 
            RowResult r = runAll(f.string(), name, prng);
            printCsvRow(fout, r);
        }
    } else if (mode == "single") { // <graph_file> <algorithm>
        string path = argv[2];
        string algo = argv[3];
        unsigned seed = 42;
        mt19937 prng(seed);

        Graph g;
        g.load(path);

        if (algo == "randomized") {
            double avg = randomizedMaxCut(g, RANDOM_RUNS, prng);
            cout << "Randomized-1 average cut over " << RANDOM_RUNS
                      << " runs: " << avg << "\n";
        } else if (algo == "greedy") {
            Partition p = greedyConstruct(g);
            cout << "Greedy cut value: " << cutWeight(g, p) << "\n";
        } else if (algo == "semigreedy") {
            Partition p = semiGreedyConstruct(g, SEMI_ALPHA, prng);
            cout << "Semi-Greedy (alpha=" << SEMI_ALPHA << ") cut value: "
                      << cutWeight(g, p) << "\n";
        } else if (algo == "localsearch") {
            Partition p = randomPartition(g, prng);
            long long before = cutWeight(g, p);
            p = localSearch(g, p).first;
            long long after = cutWeight(g, p);
            cout << "Local search: start=" << before << " -> local optimum=" << after << "\n";
        } else if (algo == "grasp") {
            long long best = grasp(g, GRASP_ITERS, SEMI_ALPHA, prng).first;
            cout << "GRASP (" << GRASP_ITERS << " iters) best cut value: " << best << "\n";
        } 
    } 
}
