

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <iomanip>

using namespace std;
using clock_type = chrono::high_resolution_clock;

struct sa_result {
    vector<int> initial_tour;
    double initial_cost = 0.0;
    vector<int> best_tour;
    double best_cost = 0.0;
    long long total_iterations = 0;
    long long accepted_moves = 0;
    long long worse_accepted = 0;
    double exec_time_seconds = 0.0;
    double t0 = 0.0;
    double alpha = 0.0;
};

struct sa_params {
    double t0 = 1000.0;
    double alpha = 0.995;
    double t_min = 0.001;
    long long iters_per_temp = 100;
    long long max_iters = 100000;
    unsigned long seed = random_device{}();
    string init_method = "random"; // "random" or "greedy"
};


// Reads n and the n x n cost matrix 
vector<vector<double>> read_input(istream &in, int &num_cities) {

    in >> num_cities;
    vector<vector<double>> cost(num_cities, vector<double>(num_cities));
    for (int i = 0; i < num_cities; i++) {
        for (int j = 0; j < num_cities; j++) {
            in >> cost[i][j];
        }
    }
    return cost;
}

double calculate_tour_cost(const vector<int> &tour, const vector<vector<double>> &cost) {
    double total = 0.0;
    int num_cities = (int)tour.size();
    for (int i = 0; i < num_cities; i++) {
        int from = tour[i];
        int to = tour[(i + 1) % num_cities];
        total += cost[from][to];
    }
    return total;
}

// Validates that a tour starts at city 0, visits every city exactly once.
bool validate_tour(const vector<int> &tour, int num_cities) {
    if ((int)tour.size() != num_cities) return false;
    if (tour[0] != 0) return false;
    vector<bool> seen(num_cities, false);
    for (int c : tour) {
        if (c < 0 || c >= num_cities) return false;
        if (seen[c]) return false;
        seen[c] = true;
    }
    for (bool s : seen) if (!s) return false;
    return true;
}

// Greedy Nearest Neighbor construction 
vector<int> greedy_tsp(const vector<vector<double>> &cost, int start = 0) {
    int num_cities = (int)cost.size();
    vector<bool> visited(num_cities, false);
    vector<int> tour;
    tour.reserve(num_cities);

    int current = start;
    visited[current] = true;
    tour.push_back(current);

    for (int step = 1; step < num_cities; step++) {
        int best = -1;
        double best_cost = -1.0;
        for (int j = 0; j < num_cities; j++) {
            if (!visited[j]) {
                if (best == -1 || cost[current][j] < best_cost) {
                    best = j;
                    best_cost = cost[current][j];
                }
            }
        }
        visited[best] = true;
        tour.push_back(best);
        current = best;
    }
    return tour;
}

// random permutation of cities
vector<int> generate_random_tour(int num_cities, mt19937 &rng) {
    vector<int> tour(num_cities);
    for (int i = 0; i < num_cities; i++) tour[i] = i;
    shuffle(tour.begin() + 1, tour.end(), rng);
    return tour;
}

// Generates a neighboring tour using 2-opt
vector<int> generate_neighbor(const vector<int> &tour, mt19937 &rng) {
    int num_cities = tour.size();
    vector<int> neighbor = tour;

    uniform_int_distribution<int> dist(1, num_cities - 1);
    int i = dist(rng);
    int j = dist(rng);
    while (j == i) j = dist(rng);
    if (i > j) swap(i, j);

    reverse(neighbor.begin() + i, neighbor.begin() + j + 1);
    return neighbor;
}

// Simulated Annealing
sa_result simulated_annealing(const vector<vector<double>> &cost,
                            vector<int> initial_tour,
                            const sa_params &params) {
    sa_result result;
    int num_cities = (int)cost.size();

    auto start_time = clock_type::now();

    mt19937 rng(params.seed);
    uniform_real_distribution<double> unif01(0.0, 1.0);

    result.initial_tour = initial_tour;
    result.initial_cost = calculate_tour_cost(initial_tour, cost);
    result.t0 = params.t0;
    result.alpha = params.alpha;

    vector<int> current_tour = initial_tour;
    double current_cost = result.initial_cost;

    vector<int> best_tour = current_tour;
    double best_cost = current_cost;

    double temp = params.t0;
    long long iterations = 0;
    long long accepted = 0;
    long long worse_accepted = 0;

    while (temp > params.t_min && iterations < params.max_iters) {
        for (long long k = 0; k < params.iters_per_temp && iterations < params.max_iters; k++) {
            vector<int> neighbor = generate_neighbor(current_tour, rng);
            double neighbor_cost = calculate_tour_cost(neighbor, cost);
            double delta = neighbor_cost - current_cost;

            bool accept = false;
            if (delta <= 0) {
                accept = true;
            } else {
                double prob = exp(-delta / temp);
                double r = unif01(rng);
                if (r < prob) {
                    accept = true;
                    worse_accepted++;
                }
            }

            if (accept) {
                current_tour = neighbor;
                current_cost = neighbor_cost;
                accepted++;
                if (current_cost < best_cost) {
                    best_cost = current_cost;
                    best_tour = current_tour;
                }
            }
            iterations++;
        }
        temp *= params.alpha;
    }

    auto end_time = clock_type::now();

    result.best_tour = best_tour;
    result.best_cost = best_cost;
    result.total_iterations = iterations;
    result.accepted_moves = accepted;
    result.worse_accepted = worse_accepted;
    result.exec_time_seconds = chrono::duration<double>(end_time - start_time).count();

    return result;
}


string tour_to_string(const vector<int> &tour) {
    ostringstream oss;
    for (size_t i = 0; i < tour.size(); i++) {
        oss << tour[i] << " -> ";
    }
    oss << tour[0];
    return oss.str();
}

void print_greedy_results(const vector<int> &tour, double cost, double exec_time) {
    cout << "---------- GREEDY METHOD ----------\n";
    cout << "Tour:\n" << tour_to_string(tour) << "\n";
    cout << "Total Cost: " << fixed << setprecision(2) << cost << "\n";
    cout << "Execution Time: " << setprecision(6) << exec_time << " seconds\n\n";
}

void print_sa_results(const sa_result &r, const sa_params &params) {
    cout << "---------- SIMULATED ANNEALING ----------\n";
    cout << "Initialization Method: " << params.init_method << "\n";
    cout << "Initial Solution:\n" << tour_to_string(r.initial_tour) << "\n";
    cout << "Initial Cost: " << fixed << setprecision(2) << r.initial_cost << "\n\n";
    cout << "Best Tour Found:\n" << tour_to_string(r.best_tour) << "\n";
    cout << "Best Cost: " << setprecision(2) << r.best_cost << "\n\n";
    cout << "Initial Temperature: " << setprecision(3) << r.t0 << "\n";
    cout << "Cooling Rate: " << r.alpha << "\n";
    cout << "Total Iterations: " << r.total_iterations << "\n";
    cout << "Accepted Moves: " << r.accepted_moves << "\n";
    cout << "Worse Moves Accepted: " << r.worse_accepted << "\n";
    cout << "Execution Time: " << setprecision(6) << r.exec_time_seconds << " seconds\n\n";
}

void print_results(int num_cities,
                  const vector<int> &greedy_tour, double greedy_cost, double greedy_time,
                  const sa_result &sa, const sa_params &params) {
    cout << "TRAVELLING SALESMAN PROBLEM\n";
    cout << "Number of Cities: " << num_cities << "\n\n";

    print_greedy_results(greedy_tour, greedy_cost, greedy_time);
    print_sa_results(sa, params);

    cout << "---------- COMPARISON ----------\n";
    cout << "Greedy Cost: " << fixed << setprecision(2) << greedy_cost << "\n";
    cout << "Simulated Annealing Cost: " << sa.best_cost << "\n";

    double improvement_pct = 0.0;
    if (greedy_cost > 0) {
        improvement_pct = (greedy_cost - sa.best_cost) / greedy_cost * 100.0;
    }
    cout << "Improvement over Greedy: " << setprecision(2) << improvement_pct << "%\n";

    cout << "Best Method for This Run: ";
    if (sa.best_cost < greedy_cost - 1e-9) {
        cout << "Simulated Annealing\n";
    } else if (greedy_cost < sa.best_cost - 1e-9) {
        cout << "Greedy\n";
    } else {
        cout << "Both methods produced the same cost.\n";
    }

    // validation check 
    bool greedy_valid = validate_tour(greedy_tour, num_cities);
    bool sa_valid = validate_tour(sa.best_tour, num_cities);
    cout << "\n[Validation] Greedy tour valid: " << (greedy_valid ? "YES" : "NO")
         << " | SA tour valid: " << (sa_valid ? "YES" : "NO") << "\n";
}




int main(int argc, char *argv[]) {
    if (argc < 2) {
        cout << "a.out <input file>\n";
        return 1;
    }

    string input_file = argv[1];
    ifstream fin(input_file);
    int num_cities;
    vector<vector<double>> cost = read_input(fin, num_cities);

    sa_params params {
        .t0 = 1000.0,
        .alpha = 0.995,
        .t_min = 0.001,
        .iters_per_temp = 100,
        .max_iters = 100000,
        .seed = random_device{}(),
        .init_method = "random",
    };


    //  Greedy 
    auto g_start = clock_type::now();
    vector<int> greedy_tour = greedy_tsp(cost, 0);
    auto g_end = clock_type::now();
    double greedy_time = chrono::duration<double>(g_end - g_start).count();
    double greedy_cost = calculate_tour_cost(greedy_tour, cost);

    //  Simulated Annealing 
    mt19937 rng(params.seed);
    vector<int> init_tour;
    if (params.init_method == "greedy") {
        init_tour = greedy_tour;
    } else {
        init_tour = generate_random_tour(num_cities, rng);
    }

    sa_result sa_res = simulated_annealing(cost, init_tour, params);

    print_results(num_cities, greedy_tour, greedy_cost, greedy_time, sa_res, params);

    return 0;
}