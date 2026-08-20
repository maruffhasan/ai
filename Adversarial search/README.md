# Mancala Adversarial Search Agent

An implementation of an AI agent for **Mancala** using the Minimax search algorithm with Alpha-Beta pruning. This repository includes custom heuristic functions, move-ordering optimizations, and execution modes for both interactive Human vs. AI play and automated AI vs. AI performance evaluation.

---

## Table of Contents
- [Game Overview](#game-overview)
- [Features & Implementation](#features--implementation)
- [Heuristic Functions](#heuristic-functions)
- [Requirements & Compilation](#requirements--compilation)
- [Usage](#usage)
  - [1. Interactive Mode (Human vs. AI)](#1-interactive-mode-human-vs-ai)
  - [2. Automated AI vs. AI Experiments](#2-automated-ai-vs-ai-experiments)
- [CLI Flag Reference](#cli-flag-reference)
- [Project Structure](#project-structure)

---

## Game Overview

Mancala is a two-player turn-based strategy game played on a board with 12 pits (6 per player) and 2 storage bins (Mancala).

* **Start:** Each pit starts with 4 stones, and both stores are empty.
* **Gameplay:** Players take turns selecting a non-empty pit on their side to sow stones counter-clockwise around the board.
* **Extra Turn:** If the last stone lands in the player's own store, the player gets an extra turn.
* **Capture:** If the last stone lands in an empty pit on the player's side and the opponent's opposite pit contains stones, both the last stone and the opposite stones are captured into the player's store.
* **End Game:** When one player's pits are completely empty, all remaining stones on the opponent's side are swept into their respective store. The player with the most stones in their store wins.

---

## Features & Implementation

* **Adversarial Search:** Minimax search integrated with **Alpha-Beta Pruning** for branch elimination[cite: 1].
* **Move Ordering Optimization:** Uses `std::stable_sort` to prioritize moves that grant immediate extra turns, maximizing Alpha-Beta pruning efficiency while keeping state evaluation deterministic across equal-valued paths.
* **4 Heuristic Functions:** Implements multi-tier strategic evaluation models ranging from basic score differences to advanced capture and turn-earning strategies[cite: 1].
* **Zero-Allocation Board State:** Built using a stack-allocated `std::array<int, 14>` board structure to minimize memory overhead and eliminate dynamic heap allocations during deep state expansions.
* **Flexible Execution Modes:** Features an interactive CLI for Human vs. AI play and an automated batch runner for experimental win-loss evaluations[cite: 1].

---

## Heuristic Functions

The AI includes 4 evaluation functions to score favorable board states[cite: 1]:

| Heuristic | Evaluation Formula | Strategic Focus |
| :--- | :--- | :--- |
| **Heuristic 1**[cite: 1] | $\Delta\text{Store}$ | **Store Difference:** Evaluates pure score differential between stores[cite: 1]. |
| **Heuristic 2**[cite: 1] | $W_1 \cdot \Delta\text{Store} + W_2 \cdot \Delta\text{Side}$ | **Side Control:** Adds remaining pit stone differential to encourage board dominance[cite: 1]. |
| **Heuristic 3**[cite: 1] | $W_1 \cdot \Delta\text{Store} + W_2 \cdot \Delta\text{Side} + W_3 \cdot \text{ExtraTurns}$ | **Turn Advantage:** Evaluates moves that grant extra turn opportunities[cite: 1]. |
| **Heuristic 4**[cite: 1] | $W_1 \cdot \Delta\text{Store} + W_2 \cdot \Delta\text{Side} + W_3 \cdot \text{ExtraTurns} + W_4 \cdot \text{Captures}$ | **Tactical Capture:** Combines score, side control, extra turns, and capture potential[cite: 1]. |

> **Note:** $W_1, W_2, W_3, W_4$ represent feature weight multipliers customizable via command-line arguments.

---

## Requirements & Compilation

### Requirements
* C++11 compatible compiler (GCC / Clang / MSVC)
* Standard C++ libraries (`iostream`, `vector`, `array`, `thread`, etc.)

### Compilation
Compile the program using `g++` with optimization flags enabled:

```bash
g++ -O3 -std=c++11 main.cpp -o mancala_ai
