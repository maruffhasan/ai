#include<iostream>
#include<queue>
#include<vector>
#include<cmath>
#include<unordered_set>
#include<list>
#include<algorithm>

using namespace std;

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


int manhattanDistance(const vector<vector<int>>& board,
                      const vector<vector<int>>& goal) {
    int n = board.size();
    vector<pair<int,int>> goalPos(n * n);
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            goalPos[goal[i][j]] = {i, j};

    int dist = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++) {
            int tile = board[i][j];
            if (tile == 0) continue;
            auto [gi, gj] = goalPos[tile];
            dist += abs(i - gi) + abs(j - gj);
        }
    return dist;
}

int heuristic(vector<vector<int>> grid,const vector<vector<int>> &goal) {
    return manhattanDistance(grid, goal);
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

    // count is even.
    if (n % 2 == 1) {
        return inversions % 2 == 0;
    } 
    else {
        int blank_row_from_buttom = n - blank_row;
        if (blank_row_from_buttom % 2 == 1) {
            return inversions % 2 == 0;
        }
        else {
            return inversions % 2 == 1;
        }
    }
}

int main () {

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

    node_pool.push_back({grid, 0, 0, nullptr});
    open_list.push(&node_pool.back());
    auto goal = build_goal(n);

    while (!open_list.empty()) {

        node* current_node = open_list.top(); open_list.pop();

        string key = make_key(current_node->grid);
        if (closed_list.count(key)) continue;
        closed_list.insert(key);

        int x = -1, y;
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
            swap(new_grid[x][y], new_grid[x + dx][y + dy]);

            if (closed_list.count(make_key(new_grid))) continue;

            if (current_node->grid == goal) {
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

            int priority_value = current_node->move_count + heuristic(new_grid, goal);

            node_pool.push_back({new_grid, current_node->move_count + 1, priority_value, current_node});
            open_list.push(&node_pool.back());
        }
    }
}
