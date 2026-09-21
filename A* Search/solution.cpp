#include<iostream>
#include<queue>
#include<vector>
#include<cmath>
#include<unordered_set>
#include<list>
#include<algorithm>
#include<chrono>

using namespace std;


enum class Heuristic { HAMMING, MANHATTAN, EUCLIDEAN, LINEAR_CONFLICT, CUSTOM };
Heuristic current_heuristic = Heuristic::LINEAR_CONFLICT; 


struct node {
    vector<vector<int>> grid;
    int move_count, priority_value;
    node* parent;

    bool operator()(const node* x, const node* y) const {
        return x->priority_value > y->priority_value;
    }
};

const vector<pair<int, int>> DIRS = {
    {0, 1},
    {0, -1},
    {1, 0},
    {-1, 0}
};


int hamming_distance(vector<vector<int>>& board,
                     vector<vector<int>>& goal) {
    int n = board.size();
    int dist = 0;

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (board[i][j] != 0 && board[i][j] != goal[i][j]) {
                dist++;
            }
        }
    }
    return dist;
}


int manhattan_distance(vector<vector<int>> &board,
                       vector<vector<int>> &goal) {
    int n = board.size();
    vector<pair<int,int>> goal_position(n * n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            goal_position[goal[i][j]] = {i, j};
        }
    }

    int dist = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            int tile = board[i][j];
            if (tile == 0) continue;
            auto [gi, gj] = goal_position[tile];
            dist += abs(i - gi) + abs(j - gj);
        }
    return dist;
}


double euclidean_distance(vector<vector<int>>& board,
                          vector<vector<int>>& goal) {
    int n = board.size();
    vector<pair<int,int>> goal_position(n * n);

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            goal_position[goal[i][j]] = {i, j};

    double dist = 0.0;

    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            int tile = board[i][j];
            if (tile == 0) continue;

            auto [gi, gj] = goal_position[tile];
            dist += sqrt((i - gi) * (i - gi) + (j - gj) * (j - gj));
        }

    return dist;
}

int linear_conflict(vector<vector<int>>& board,
                    vector<vector<int>>& goal) {
    int n = board.size();

    vector<pair<int,int>> goal_position(n * n);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            goal_position[goal[i][j]] = {i, j}; 
        }
    }

    int conflicts = 0;

    // Row conflicts
    for (int row = 0; row < n; row++) {
        for (int i = 0; i < n; i++) {
            int tile1 = board[row][i];
            if (tile1 == 0) continue;

            auto [r1, c1] = goal_position[tile1];
            if (r1 != row) continue;

            for (int j = i + 1; j < n; j++) {
                int tile2 = board[row][j];
                if (tile2 == 0) continue;

                auto [r2, c2] = goal_position[tile2];
                if (r2 != row) continue;

                if (c1 > c2)
                    conflicts++;
            }
        }
    }

    // Column conflicts
    for (int col = 0; col < n; col++) {
        for (int i = 0; i < n; i++) {
            int tile1 = board[i][col];
            if (tile1 == 0) continue;

            auto [r1, c1] = goal_position[tile1];
            if (c1 != col) continue;

            for (int j = i + 1; j < n; j++) {
                int tile2 = board[j][col];
                if (tile2 == 0) continue;

                auto [r2, c2] = goal_position[tile2];
                if (c2 != col) continue;

                if (r1 > r2)
                    conflicts++;
            }
        }
    }

    return manhattan_distance(board, goal) + 2 * conflicts;
}


int custom_heuristic(vector<vector<int>>& board,
                     vector<vector<int>>& goal) {

    int n = board.size();
    int h = manhattan_distance(board, goal);

    // Four corners
    vector<pair<int,int>> corners = {
        {0, 0},
        {0, n - 1},
        {n - 1, 0},
        {n - 1, n - 1}
    };

    for (auto [i, j] : corners) {
        if (goal[i][j] == 0) continue;       
        if (board[i][j] != goal[i][j]) {
            h++;
        }
    }

    return h;
}



int heuristic(vector<vector<int>> grid, vector<vector<int>> &goal) {
    switch (current_heuristic) {
        case Heuristic::HAMMING:         return hamming_distance(grid, goal);
        case Heuristic::MANHATTAN:       return manhattan_distance(grid, goal);
        case Heuristic::EUCLIDEAN:       return (int) round(euclidean_distance(grid, goal));
        case Heuristic::LINEAR_CONFLICT: return linear_conflict(grid, goal);
        case Heuristic::CUSTOM:          return custom_heuristic(grid, goal);
    }
    return 0;
}


void print_grid(vector<vector<int>> grid) {
    int n = grid.size();

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cout << grid[i][j] << " ";
        }
        cout << "\n";
    }
    cout << "\n";
}

string make_key(vector<vector<int>> &grid) {
    string key = "";
    int n = grid.size();
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            key += to_string(grid[i][j]);
        }
    }
    return key;
}


vector<vector<int>> build_goal(int n) {
    vector<vector<int>> goal(n, vector<int>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            goal[i][j] = (i * n + j + 1) % (n * n);
        }
    }
    return goal;
}

bool is_solvable(vector<vector<int>> &grid) {
    int n = grid.size();
    int blank_row = -1;
    vector<int> flat;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (grid[i][j] == 0) blank_row = i;
            flat.push_back(grid[i][j]);
        }
    }

    int inversions = 0;
    for (int i = 0; i < flat.size(); i++) {
        for (int j = i + 1; j < flat.size(); j++) {
            if (flat[i] && flat[j] && flat[i] > flat[j]) {
                inversions++;
            }
        }
    }

    
    if (n % 2 == 1) {                                 // n odd & inversion count even -> return true
        return inversions % 2 == 0;
    } 
    else {                                            // n even
        int blank_row_from_buttom = n - blank_row;
        if (blank_row_from_buttom % 2 == 1) {
            return inversions % 2 == 0;               // blank is on odd position from buttom & inversion count is even -> return true
        }
        else {
            return inversions % 2 == 1;               // blank is on even position from buttom & inversion count is odd -> return true
        }
    }
}

int main (int argc, char* argv[]) { 

    freopen(argv[1], "r", stdin);

    double W = 1.0;

    if (argc > 2) {
        string h = argv[2];
        if (h == "hamming")          current_heuristic = Heuristic::HAMMING;
        else if (h == "manhattan")   current_heuristic = Heuristic::MANHATTAN;
        else if (h == "euclidean")   current_heuristic = Heuristic::EUCLIDEAN;
        else if (h == "linear_conflict") current_heuristic = Heuristic::LINEAR_CONFLICT;
        else if (h == "custom")      current_heuristic = Heuristic::CUSTOM;
        else {
            cerr << "Unknown heuristic\n";
            return 1;
        }
        if (argc > 3) W = atof(argv[3]);
    }


    // take input 
    int n; cin >> n;
    vector<vector<int>> grid(n, vector<int>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            cin >> grid[i][j];
        }
    }

    if (!is_solvable(grid)) {
        cout << "Unsolvable puzzle\n";
        return 0;
    }

    priority_queue<node*, vector<node*>, node> open_list; // min heap
    unordered_set<string> closed_list;
    list<node> node_pool;

    node_pool.push_back({grid, 0, 0, nullptr});  // node_pool so that pointers of nodes don't get disappeared. (grid, move_count, priority_value, parent)
    open_list.push(&node_pool.back());
    auto goal = build_goal(n);


    long long nodes_expanded = 0;
    auto t_start = chrono::high_resolution_clock::now();


    while (!open_list.empty()) {

        node* current_node = open_list.top(); open_list.pop(); // Remove the node with the lowest priority value

        string key = make_key(current_node->grid);
        if (closed_list.count(key)) continue; // check whether it is already in closed_list 
        closed_list.insert(key); // Add it to the closed list.

        nodes_expanded++;


        if (current_node->grid == goal) { // if this node is goal node, print the hierarchy 
            auto t_end = chrono::high_resolution_clock::now();
            double ms = chrono::duration<double, milli>(t_end - t_start).count();
            cout << "SOLVED cost=" << current_node->move_count
                 << " nodes_expanded=" << nodes_expanded
                 << " time_ms=" << ms << "\n";


            vector<vector<vector<int>>> path;
            for (node* ptr = current_node; ptr != nullptr; ptr = ptr->parent) {
                path.push_back(ptr->grid);
            }
            reverse(path.begin(), path.end());

            cout << "\nMinimum number of moves = " << path.size() - 1 << "\n\n";
            for (auto grid : path) {
                print_grid(grid);
            }
            return 0;
        }

        int x = -1, y; // position of empty grid
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if (current_node->grid[i][j] == 0) {
                    x = i, y = j;
                    break;
                }
            }
            if (x != -1) break;
        }

        for (auto [dx, dy] : DIRS) {
            if (x + dx >= n || x + dx < 0 || y + dy >= n || y + dy < 0) continue;
            
            auto new_grid = current_node->grid;
            swap(new_grid[x][y], new_grid[x + dx][y + dy]); // Generate all valid neighboring board configurations

            if (closed_list.count(make_key(new_grid))) continue; // For each neighbor not in the closed list, insert it into the priority queue.

            int priority_value = current_node->move_count + W * heuristic(new_grid, goal);

            node_pool.push_back({new_grid, current_node->move_count + 1, priority_value, current_node});
            open_list.push(&node_pool.back());
        }
    }
}
