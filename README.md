# Wilted Chess Engine

A UCI-compliant chess engine by TheTilted096 featuring NNUE evaluation and modern search techniques.

**Current Version:** 1.1.1.5 (NNUE Release - November 2025)

## Features

### Evaluation
- **NNUE** (Efficiently Updatable Neural Network) evaluation

### Search
- **Principal Variation Search (PVS)** with aspiration windows
- **Late Move Reductions (LMR)**
- **Internal Iterative Reductions (IIR)**
- **Late Move Pruning (LMP)**
- **Futility Pruning (FP)**
- **PVS SEE Pruning**
- **Null Move Pruning (NMP)**
- **Reverse Futility Pruning (RFP)**
- **Quiescence Search** with SEE pruning
- **Transposition Table**
- **Move Ordering:**
  - TT move
  - MVV-LVA (Most Valuable Victim - Least Valuable Attacker)
  - Killer move heuristic
  - Quiet history heuristic
  - SEE Bad Captures
  - History Malus
- **Iterative Deepening** with PV table
- **SMP (Symmetric Multi-Processing)** with thread pool

### Move Generation
- PEXT-based sliding piece move generation
- Legal move generation
- Chess960/FRC support

## Building

Requires a C++ compiler with C++20 support and PEXT instruction set support (modern x64 processors).

```bash
make
```

This will produce the `wilted` executable.

## Usage

### UCI Mode
Run the engine in UCI protocol mode:

```bash
./wilted
```

The engine is compatible with any UCI-compliant chess GUI (e.g., CuteChess, Arena, ChessBase).

### Benchmark
Run the built-in performance benchmark:

```bash
./wilted bench
```

### UCI Options
- `Hash` - Transposition table size in MB
- `Threads` - Number of search threads
- `UCI_Chess960` - Enable Chess960/Fischer Random Chess support

## Technical Specifications

- **Protocol:** UCI (Universal Chess Interface)
- **Move Generation:** PEXT-based for sliders
- **FEN Format:** Shredder FEN (required for Chess960)
- **Parallel Search:** Multi-threaded with shared transposition table
- **Time Management:** Adaptive time allocation with soft/hard node limits

### Special Behaviors
- `go depth X` - Depth limit applies to main thread only
- `go nodes X` - Node limit restricts search to single thread
- Chess960 requires `UCI_Chess960` option to be set

## Version History

See [Changelog.txt](Changelog.txt) for complete development history from version 0.0.0.0 to 1.0.0.0.

Key milestones:
- **0.0.0.0** (June 2025) - Initial move generation
- **0.1.0.0** (June 2025) - Basic alpha-beta search
- **0.2.0.0** (July 2025) - MVV-LVA Move Ordering
- **0.3.0.0** (July 2025) - Quiescence search
- **0.4.0.0** (July 2025) - Transposition table
- **0.5.0.0** (August 2025) - Quiet History
- **0.6.0.0** (August 2025) - Reverse Futility Pruning
- **0.7.0.0** (August 2025) - Late Move Reductions
- **0.8.0.0** (August 2025) - Multithreaded Search
- **0.9.0.0** (October 2025) - PSQT tuning
- **1.0.0.0** (November 2025) - NNUE evaluation

Each version has been rigorously tested using SPRT or fixed-game testing.

## Development

### Versioning Scheme
- **1st digit** - Major milestone (e.g., NNUE implementation)
- **2nd digit** - Moderate update (e.g., new search feature)
- **3rd digit** - Small patch (e.g., bug fix or optimization)
- **4th digit** - Non-functional changes (e.g., refactoring)

Development versions use the format `x.x.x.x-dev`.

## Special Thanks To

The Stockfish Community. In particular, sscg13, Ciekce, fury, a_randomnoob, lily, kieren, toanth, matt

## License

Copyright © TheTilted096

## Author

TheTilted096
