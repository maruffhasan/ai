# CSE 318 — Assignment 02: Solving Max-Cut with GRASP

Implementation of the **Maximum Cut (MAX-CUT)** problem using randomized, greedy, semi-greedy,
local search, and **GRASP (Greedy Randomized Adaptive Search Procedure)** heuristics, evaluated
on the standard G-set benchmark instances.

## Problem Statement

Given an undirected graph `G = (V, U)` with edge weights `w_uv`, the Max-Cut problem asks for a
nonempty proper subset of vertices `S ⊂ V` such that the total weight of edges crossing between
`S` and its complement `S̄` is maximized:

```
w(S, S̄) = Σ w_uv   for u ∈ S, v ∈ S̄
```

## Algorithms Implemented

| Algorithm | Description |
|---|---|
| **Randomized** | Each vertex is assigned to partition `X` or `Y` independently with probability 1/2. Run `n` times and averaged to estimate expected cut weight. |
| **Greedy** | Starts by placing the two endpoints of the maximum-weight edge into separate partitions, then greedily assigns each remaining vertex to whichever partition maximizes its immediate contribution to the cut. |
| **Semi-Greedy** | A randomized variant of Greedy. At each step, a Restricted Candidate List (RCL) is built using the **value-based method**, with cutoff `μ = w_min + α(w_max − w_min)`, and a vertex is chosen uniformly at random from the RCL. |
| **Local Search** | Starting from a feasible cut, repeatedly moves the vertex with the best improving gain `δ(v)` to the opposite partition until no improving move remains (local optimum). |
| **GRASP** | Iteratively combines a Semi-Greedy construction phase with Local Search, keeping the best solution found across `MaxIterations` runs. |

## Input Format

```
n m
V1 V2 Wt
V1 V2 Wt
...
```

- `n` — number of vertices
- `m` — number of edges
- Each of the following `m` lines gives an edge `(V1, V2)` with integer weight `Wt`
- Graphs are undirected

## Benchmark Data Set

54 benchmark graphs from the standard G-set collection are used for evaluation. Known best
solutions / upper bounds are available for 24 of them (e.g., G1, G11, G14, G22, G32, G35, G43, G48, …)
and are used as a reference point when comparing algorithm performance.

## Repository Structure

```
2205***/
├── 2205***.{cpp,py,...}   # Source code (algorithm implementations)
├── 2205***.csv            # Results across all 54 benchmark graphs
├── 2205***.pdf            # Report (descriptions, comparisons, plots)
└── README.md
```

## Results (CSV Format)

Results for all 54 graphs are recorded in `2205***.csv`, capturing for each graph:

- `|V|`, `|E|`
- Constructive algorithm results: Simple Randomized, Simple Greedy, Semi-Greedy (with α)
- Local Search: number of iterations, average value
- GRASP: number of iterations, best value found
- Known best solution / upper bound (where available)

## Report

The accompanying report (`2205***.pdf`) includes:

- A high-level description of each algorithm (Randomized, Greedy, Semi-Greedy, Local Search, GRASP)
- A comparison of algorithm performance across the benchmark graphs
- Plots visualizing how each algorithm performs relative to the others and to known best solutions

## Notes

- Semi-Greedy uses the **value-based RCL** construction method.
- GRASP combines Semi-Greedy construction with Local Search refinement over multiple iterations,
  keeping track of the best solution found so far.

## Assignment Info

- **Course:** CSE 318
- **Assignment:** 02 — Solving the Max-Cut problem by GRASP
