# Mancala Adversarial Search

A C++ implementation of Mancala (Kalah variant) featuring a minimax AI with alpha-beta pruning. Supports both interactive human-vs-computer play and automated AI-vs-AI experiments for comparing heuristics.

## Features

- **Full Mancala rules**: sowing, extra turns on landing in your own store, and the "capture" rule (landing in an empty pit on your side captures that stone plus all stones in the opposite pit).
- **Minimax search with alpha-beta pruning** and move ordering (moves that grant an extra turn are searched first).
- **Four selectable heuristics** of increasing sophistication (see below).
- **Two game modes**:
  - Interactive play against the AI.
  - Batch AI-vs-AI experiments to compare heuristics/depths over many games.
- **Configurable search depth** and heuristic weights via command-line flags.

## Board Layout

```
+------------------------------------------+
|              PLAYER 1 (AI)               |
|    [12]  [11]  [10]  [ 9]  [ 8]  [ 7]    |
|    ( 4)  ( 4)  ( 4)  ( 4)  ( 4)  ( 4)    |
| ( 0)                                ( 0) |
|    ( 4)  ( 4)  ( 4)  ( 4)  ( 4)  ( 4)    |
|    [ 0]  [ 1]  [ 2]  [ 3]  [ 4]  [ 5]    |
|            PLAYER 0 (HUMAN)              |
+------------------------------------------+
```

- Pits `0–5` belong to Player 0 (Human); pit `6` is Player 0's store.
- Pits `7–12` belong to Player 1 (AI); pit `13` is Player 1's store.
- Each pit starts with 4 stones; stores start empty.

## Rules Implemented

1. **Sowing**: pick a non-empty pit on your side; one stone is dropped into each subsequent pit going counter-clockwise, skipping the opponent's store.
2. **Extra turn**: if your last stone lands in your own store, you move again.
3. **Capture**: if your last stone lands in an empty pit on your own side, you capture that stone plus all stones in the directly opposite pit, moving them into your store.
4. **Game end**: when one player's side is completely empty, the other player sweeps all remaining stones on their side into their own store. Higher store total wins.

## AI Heuristics

The AI evaluates board states using one of four heuristic types (`--ai_heuristic`, `--p0`, `--p1`):

| Type | Formula | Description |
|------|---------|--------------|
| 1 | `store_diff` | Difference between the player's store and the opponent's store. |
| 2 | `w1*store_diff + w2*side_diff` | Adds the difference in total stones remaining on each side. |
| 3 | `w1*store_diff + w2*side_diff + w3*extra_moves` | Adds the number of moves available that would grant an extra turn. |
| 4 | `w1*store_diff + w2*side_diff + w3*extra_moves + w4*capture_value` | Adds the total stones capturable this turn across all available moves. |

Weights (`w1`–`w4`) default to `1.0, 2.0, 3.0, 4.0` and can be overridden via command-line flags.

## Building

Requires a C++17 (or later) compiler.

```bash
g++ -std=c++17 -O2 -o mancala mancala.cpp
```

## Usage

```
Mancala Adversarial Search - Usage:
  --play           Play interactively vs Computer
  --ai_heuristic H Heuristic choice for AI in play mode: 1-4 (default 4)
  --games N        Number of games to auto-play in AI vs AI mode (default 100)
  --p0 H           Heuristic type for player 0 in AI vs AI mode (default 4)
  --p1 H           Heuristic type for player 1 in AI vs AI mode (default 1)
  --depth D        Search depth limit for AI players (default 6)
  --help           Show this message
```

Additional flags to override heuristic weights (apply to both players): `--w1`, `--w2`, `--w3`, `--w4`.

### Play against the AI

```bash
./mancala --play
```

You are Player 0 (pits `0–5`) and move first. Enter a pit number `0–5` on your turn. The AI (Player 1) uses heuristic 4 by default; change it with `--ai_heuristic`:

```bash
./mancala --play --ai_heuristic 2
```

### Run AI-vs-AI experiments

Compare two heuristics/configurations over many games (no interactive prompts):

```bash
./mancala --games 200 --p0 4 --p1 1 --depth 6
```

This prints progress updates and a final summary: win counts/percentages for each player, draw rate, and the average number of search nodes visited per game.

### Example: custom heuristic weights

```bash
./mancala --games 100 --p0 4 --p1 4 --w1 1.0 --w2 1.5 --w3 2.0 --w4 3.0
```

## Code Structure

| Component | Purpose |
|-----------|---------|
| `board` | Holds the 14-cell array representing pits and stores. |
| `weights` / `ai_config` | Heuristic weights and AI search configuration (heuristic type, max depth). |
| `apply_move`, `valid_moves`, `is_game_over`, `finalize_game` | Core game mechanics: sowing, capture, turn/end-of-game handling. |
| `evaluate` | Scores a board state for a given player using the selected heuristic. |
| `minimax` | Alpha-beta pruned minimax search over future game states. |
| `choose_move` | Top-level move selection: runs minimax for each candidate move and picks the best. |
| `play_human_vs_computer` | Interactive game loop with console I/O and display. |
| `play_game` / `run_experiment` | Automated AI-vs-AI game loop and batch experiment runner with statistics. |

## Notes

- The `game_delay_ms` global (default `800`) adds artificial pauses during interactive play for readability; it has no effect on AI-vs-AI experiments' timing logic.
- Search node counts are tracked and reported to give a rough sense of search cost across heuristics/depths.
