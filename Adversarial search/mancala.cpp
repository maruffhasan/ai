#include <bits/stdc++.h>
using namespace std;

const int num_cells = 14;
const int stones_per_pit_start = 4;
const int INF = 1e9;

struct board {
    array<int, num_cells> cell;
    board() { 
        cell.fill(stones_per_pit_start); 
        cell[6] = 0;
        cell[13] = 0;
    }
};

struct weights { 
    double w1 = 1.0, w2 = 2.0, w3 = 3.0, w4 = 4.0; 
};

struct ai_config {
    int heuristic_type = 4;
    struct weights weights;
    int max_depth = 6;
};

int game_delay_ms = 800;

void delay(int multiplier = 1) {
    if (game_delay_ms > 0) {
        this_thread::sleep_for(chrono::milliseconds(game_delay_ms * multiplier));
    }
}

int store_index(int player)      { return player == 0 ? 6 : 13; }
int opp_store_index(int player)  { return player == 0 ? 13 : 6; }
int opponent(int player)         { return 1 - player; }

bool is_own_pit(int idx, int player) {
    return player == 0 ? (idx >= 0 && idx <= 5) : (idx >= 7 && idx <= 12);
}

int opposite_pit(int idx)       { return 12 - idx; }
int pit_range_start(int player) { return player == 0 ? 0 : 7; }
int pit_range_end(int player)   { return player == 0 ? 5 : 12; }

void print_board(const board& b) {
    cout << "\n+------------------------------------------+\n";
    cout << "|              PLAYER 1 (AI)               |\n";
    cout << "|    [12]  [11]  [10]  [ 9]  [ 8]  [ 7]    |\n";
    cout << "|    ";
    for (int i = 12; i >= 7; --i) cout << "(" << setw(2) << b.cell[i] << ")  ";
    cout << "  |\n";
    cout << "| (" << setw(2) << b.cell[13] << ")                                (" << setw(2) << b.cell[6] << ") |\n";
    cout << "|    ";
    for (int i = 0; i <= 5; ++i) cout << "(" << setw(2) << b.cell[i] << ")  ";
    cout << "  |\n";
    cout << "|    [ 0]  [ 1]  [ 2]  [ 3]  [ 4]  [ 5]    |\n";
    cout << "|            PLAYER 0 (HUMAN)              |\n";
    cout << "+------------------------------------------+\n\n";
}

vector<int> valid_moves(const board& b, int player) {
    vector<int> moves;
    for (int i = pit_range_start(player); i <= pit_range_end(player); ++i) {
        if (b.cell[i] > 0) moves.push_back(i);
    }
    return moves;
}

bool side_empty(const board& b, int player) {
    for (int i = pit_range_start(player); i <= pit_range_end(player); ++i) {
        if (b.cell[i] > 0) return false;
    }
    return true;
}

void finalize_game(board& b) {
    int s0 = 0, s1 = 0;
    for (int i = 0; i <= 5; ++i)  {
        s0 += b.cell[i]; 
        b.cell[i] = 0;
    }
    for (int i = 7; i <= 12; ++i) {
        s1 += b.cell[i];
        b.cell[i] = 0; 
    }
    b.cell[6]  += s0;
    b.cell[13] += s1;
}

bool is_game_over(const board& b) {
    return side_empty(b, 0) || side_empty(b, 1);
}

bool apply_move(board& b, int player, int from, int* captured_out = nullptr) {
    int stones = b.cell[from];
    b.cell[from] = 0;
    int idx = from;
    int opp_store = opp_store_index(player);

    while (stones > 0) {
        idx = (idx + 1) % num_cells;
        if (idx == opp_store) continue;
        b.cell[idx]++;
        stones--;
    }

    int last = idx;
    if (captured_out) *captured_out = 0;

    if (is_own_pit(last, player) && b.cell[last] == 1) {
        int opp = opposite_pit(last);
        if (b.cell[opp] > 0) {
            int captured = b.cell[last] + b.cell[opp];
            b.cell[store_index(player)] += captured;
            b.cell[last] = 0;
            b.cell[opp] = 0;
            if (captured_out) *captured_out = captured;
        }
    }

    if (is_game_over(b)) finalize_game(b);

    return last == store_index(player);
}

int side_sum(const board& b, int player) {
    int s = 0;
    for (int i = pit_range_start(player); i <= pit_range_end(player); ++i) 
        s += b.cell[i];
    return s;
}

bool lands_in_own_store(const board& b, int player, int from) {
    int stones = b.cell[from];
    if (stones == 0) return false;
    int idx = from;
    int opp_store = opp_store_index(player);
    while (stones > 0) {
        idx = (idx + 1) % num_cells;
        if (idx == opp_store) continue;
        stones--;
    }
    return idx == store_index(player);
}

int extra_move_opportunities(const board& b, int player) {
    int count = 0;
    for (int i = pit_range_start(player); i <= pit_range_end(player); ++i) {
        if (b.cell[i] > 0 && lands_in_own_store(b, player, i)) 
            count++;
    }
    return count;
}

int capture_opportunity_value(const board& b, int player) {
    int best = 0;
    for (int i = pit_range_start(player); i <= pit_range_end(player); ++i) {
        if (b.cell[i] == 0) continue;
        board copy = b;
        int captured = 0;
        apply_move(copy, player, i, &captured);
        best += captured;
    }
    return best;
}

double evaluate(const board& b, int player, int heuristic_type, const weights& w) {
    int opp = opponent(player);
    double store_diff = b.cell[store_index(player)] - b.cell[store_index(opp)];

    if (heuristic_type == 1) return store_diff;

    double side_diff = side_sum(b, player) - side_sum(b, opp);
    if (heuristic_type == 2) return w.w1 * store_diff + w.w2 * side_diff;

    double additional_move_earned = extra_move_opportunities(b, player);
    if (heuristic_type == 3) return w.w1 * store_diff + w.w2 * side_diff + w.w3 * additional_move_earned;

    double stones_captured = capture_opportunity_value(b, player);
    return w.w1 * store_diff + w.w2 * side_diff + w.w3 * additional_move_earned + w.w4 * stones_captured;
}

double minimax(board b, int current_player, int ai_player, int depth,
               double alpha, double beta, const ai_config& cfg, long long& node_count) {
    node_count++;

    if (depth == 0 || is_game_over(b)) {
        return evaluate(b, ai_player, cfg.heuristic_type, cfg.weights);
    }

    vector<int> moves = valid_moves(b, current_player);
    if (moves.empty()) {
        return evaluate(b, ai_player, cfg.heuristic_type, cfg.weights);
    }

    bool maximizing = (current_player == ai_player);

    sort(moves.begin(), moves.end(), [&](int a, int c) {
        return lands_in_own_store(b, current_player, a) > lands_in_own_store(b, current_player, c);
    });

    if (maximizing) {
        double best = -INF;
        for (int m : moves) {
            board next = b;
            bool extra_turn = apply_move(next, current_player, m);
            int next_player = extra_turn ? current_player : opponent(current_player);
            
            double val = minimax(next, next_player, ai_player, depth - 1, alpha, beta, cfg, node_count);
            
            best = max(best, val);
            alpha = max(alpha, best);
            if (beta <= alpha) break;
        }
        return best;
    } else {
        double best = INF;
        for (int m : moves) {
            board next = b;
            bool extra_turn = apply_move(next, current_player, m);
            int next_player = extra_turn ? current_player : opponent(current_player);
            
            double val = minimax(next, next_player, ai_player, depth - 1, alpha, beta, cfg, node_count);
            
            best = min(best, val);
            beta = min(beta, best);
            if (beta <= alpha) break;
        }
        return best;
    }
}

int choose_move(const board& b, int player, const ai_config& cfg, long long& node_count) {
    vector<int> moves = valid_moves(b, player);
    if (moves.empty()) return -1;
    if (moves.size() == 1) return moves[0];

    double alpha = -INF;
    double beta  =  INF;
    double best = -INF;
    int best_move = moves[0];

    vector<int> ordered = moves;
    stable_sort(ordered.begin(), ordered.end(), [&](int a, int c) {
        return lands_in_own_store(b, player, a) > lands_in_own_store(b, player, c);
    });

    for (int m : ordered) {
        board next = b;
        bool extra_turn = apply_move(next, player, m);
        int next_player = extra_turn ? player : opponent(player);
        
        double val = minimax(next, next_player, player, cfg.max_depth - 1, alpha, beta, cfg, node_count);
        
        if (val > best) { 
            best = val; 
            best_move = m; 
        }
        alpha = max(alpha, best);
    }

    return best_move;
}

void play_human_vs_computer(const ai_config& ai_cfg) {
    board b;
    int human_player = 0;
    int ai_player = 1;
    int current_player = human_player;

    cout << "           WELCOME TO HUMAN VS AI MANCALA               \n";
    cout << "You are Player 0 (pits 0 to 5). AI is Player 1 (pits 7 to 12).\n";
    cout << "You move FIRST!\n";

    delay(1);

    while (!is_game_over(b)) {
        print_board(b);
        delay(1);

        if (current_player == human_player) {
            vector<int> valid = valid_moves(b, human_player);
            int chosen_pit = -1;
            string input;

            while (true) {
                cout << "Your turn! Choose pit (0 - 5): ";
                cin >> input;

                if (input.length() == 1 && input[0] >= '0' && input[0] <= '5') {
                    chosen_pit = input[0] - '0';

                    if (find(valid.begin(), valid.end(), chosen_pit) != valid.end()) {
                        break;
                    }
                    cout << "Invalid move. Pick a non-empty pit on your side.\n";
                } else {
                    cout << "Invalid input. Enter a number between 0 and 5.\n";
                }
            }

            cout << ">> You choose pit " << chosen_pit << ". Sowing stones...\n";
            delay(1);

            int captured = 0;
            bool extra_turn = apply_move(b, human_player, chosen_pit, &captured);
            
            if (captured > 0) {
                cout << ">> Great move! You captured " << captured << " stones!\n";
                delay(1);
            }
            if (extra_turn) {
                cout << ">> You landed in your store! YOU GET AN EXTRA TURN!\n";
                delay(1);
            } else {
                current_player = ai_player;
            }
        } else {
            cout << "AI is thinking...\n";
            delay(2);

            long long nodes_visited = 0;
            int ai_move = choose_move(b, ai_player, ai_cfg, nodes_visited);
            
            int captured = 0;
            bool extra_turn = apply_move(b, ai_player, ai_move, &captured);

            cout << ">> AI chose pit " << ai_move << "\n";
            delay(1);

            if (captured > 0) {
                cout << ">> AI captured " << captured << " stones!\n";
                delay(1);
            }
            if (extra_turn) {
                cout << ">> AI landed in its store! AI GETS AN EXTRA TURN!\n";
                delay(1);
            } else {
                current_player = human_player;
            }
        }
    }

    cout << "\nSweeping remaining stones off the board...\n";
    delay(2);

    print_board(b);
    int human_score = b.cell[6];
    int ai_score = b.cell[13];

    cout << "               GAME OVER \n";
    delay(1);
    cout << "Final Score:\n";
    cout << "  Human (Player 0) : " << human_score << "\n";
    cout << "  AI    (Player 1) : " << ai_score << "\n";
    delay(1);

    if (human_score > ai_score) {
        cout << "\nCongratulations! YOU WON!\n";
    } else if (ai_score > human_score) {
        cout << "\nAI Wins! Better luck next time.\n";
    } else {
        cout << "\nIt's a DRAW!\n";
    }
}

int play_game(const ai_config& cfg0, const ai_config& cfg1,
              long long* total_nodes_out = nullptr) {
    board b;
    int current_player = 0;
    long long total_nodes = 0;

    while (!is_game_over(b)) {
        vector<int> moves = valid_moves(b, current_player);
        if (moves.empty()) break;

        const ai_config& cfg = (current_player == 0) ? cfg0 : cfg1;
        long long nodes_visited = 0;
        int move = choose_move(b, current_player, cfg, nodes_visited);
        total_nodes += nodes_visited;
        if (move < 0) break;

        bool extra_turn = apply_move(b, current_player, move);

        if (!extra_turn) current_player = opponent(current_player);
    }

    if (!is_game_over(b)) finalize_game(b);
    if (total_nodes_out) *total_nodes_out = total_nodes;

    if (b.cell[6] > b.cell[13]) return 0;
    if (b.cell[13] > b.cell[6]) return 1;
    return -1;
}

void run_experiment(int num_games, const ai_config& cfg0, const ai_config& cfg1) {
    int wins0 = 0, wins1 = 0, draws = 0;
    long long node_total = 0;

    for (int g = 0; g < num_games; ++g) {
        long long nodes = 0;
        int result = play_game(cfg0, cfg1, &nodes);
        node_total += nodes;
        if (result == 0) wins0++;
        else if (result == 1) wins1++;
        else draws++;

        if (num_games > 1 && (g + 1) % max(1, num_games / 10) == 0) {
            cout << "  ...completed " << (g + 1) << "/" << num_games << " games\n";
        }
    }

    cout << "\nExperiment Results\n\n";
    cout << "Games played: " << num_games << "\n";
    cout << "Player 0 (heuristic-" << cfg0.heuristic_type << ", depth=" << cfg0.max_depth << "): "
         << wins0 << " wins (" << (100.0 * wins0 / num_games) << "%)\n";
    cout << "Player 1 (heuristic-" << cfg1.heuristic_type << ", depth=" << cfg1.max_depth << "): "
         << wins1 << " wins (" << (100.0 * wins1 / num_games) << "%)\n";
    cout << "Draws: " << draws << " (" << (100.0 * draws / num_games) << "%)\n";
    cout << "Average search nodes/game: " << (node_total / num_games) << "\n";
    cout << "\n";
}

void print_help() {
    cout <<
        "Mancala Adversarial Search - Usage:\n"
        "  --play           Play interactively vs Computer\n"
        "  --ai_heuristic H Heuristic choice for AI in play mode: 1-4 (default 4)\n"
        "  --games N        Number of games to auto-play in AI vs AI mode (default 100)\n"
        "  --p0 H           Heuristic type for player 0 in AI vs AI mode (default 4)\n"
        "  --p1 H           Heuristic type for player 1 in AI vs AI mode (default 1)\n"
        "  --depth D        Search depth limit for AI players (default 6)\n"
        "  --help           Show this message\n";
}

int main(int argc, char** argv) {
    bool play_mode = false;
    int num_games = 100;

    ai_config cfg0, cfg1;
    cfg0.heuristic_type = 4;
    cfg1.heuristic_type = 1;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        auto next = [&](const char* name) -> string {
            if (i + 1 >= argc) { cerr << "Missing value for " << name << "\n"; exit(1); }
            return argv[++i];
        };

        if (arg == "--help") { print_help(); return 0; }
        else if (arg == "--play") play_mode = true;
        else if (arg == "--ai_heuristic") cfg1.heuristic_type = stoi(next("--ai_heuristic"));
        else if (arg == "--games") num_games = stoi(next("--games"));
        else if (arg == "--p0") cfg0.heuristic_type = stoi(next("--p0"));
        else if (arg == "--p1") cfg1.heuristic_type = stoi(next("--p1"));
        else if (arg == "--depth") { int d = stoi(next("--depth")); cfg0.max_depth = d; cfg1.max_depth = d; }
        else if (arg == "--w1") { double v = stod(next("--w1")); cfg0.weights.w1 = v; cfg1.weights.w1 = v; }
        else if (arg == "--w2") { double v = stod(next("--w2")); cfg0.weights.w2 = v; cfg1.weights.w2 = v; }
        else if (arg == "--w3") { double v = stod(next("--w3")); cfg0.weights.w3 = v; cfg1.weights.w3 = v; }
        else if (arg == "--w4") { double v = stod(next("--w4")); cfg0.weights.w4 = v; cfg1.weights.w4 = v; }
    }

    if (play_mode) {
        play_human_vs_computer(cfg1);
    } else {
        run_experiment(num_games, cfg0, cfg1);
    }

    return 0;
}