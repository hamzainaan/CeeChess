# CeeChess
Hi! I am a bot written in C, heavily inspired by the Vice engine and video series done by Bluefever! If you want to try your hand at facing me, I am occasionally on;

- https://lichess.org/@/seeChessBot

## Features and Techniques

### Search Techniques
- **Alpha-Beta Pruning** - Core search algorithm with fail-soft implementation
- **Iterative Deepening** - Progressively deeper searches with previous results
- **Principal Variation Search** - Optimized alpha-beta for PV-nodes
- **Quiescence Search** - Extends search in tactically volatile positions
- **Null Move Pruning** - Skip a turn to quickly identify non-zugzwang positions
- **Late Move Reduction (LMR)** - Reduces search depth for likely poor moves
- **Aspiration Windows** - Narrow alpha-beta bounds for faster searching
- **Razoring** - Early pruning of likely poor positions
- **Futility Pruning** - Skip moves unlikely to improve position
- **Probcut** - Probabilistic forward pruning technique
- **Singular Extensions** - Extends search for moves that appear much better than alternatives
- **Mate Distance Pruning** - Prunes paths that can't improve existing mate scores

### Evaluation Features
- **Piece-Square Tables** - Position-dependent piece values
- **Material Balance** - Basic piece counting with accurate values
- **Pawn Structure Analysis**:
  - Isolated pawns
  - Connected pawns
  - Passed pawns
  - Connected passed pawns
- **King Safety** - Evaluates king position and nearby pawn shield
- **Piece Mobility** - Rewards pieces with more available moves
- **Open File Control** - Bonuses for rooks and queens on open/semi-open files
- **King Tropism** - Rewards pieces that are close to the enemy king
- **Bishop Pair Bonus** - Bonus for having both bishops
- **Tapered Evaluation** - Smoothly transitions between middlegame and endgame values
- **Material Draw Recognition** - Identifies theoretically drawn endgame positions

### Engine Features
- **UCI Protocol Support** - Compatible with UCI chess GUIs
- **Transposition Table** - Efficiently caches previously evaluated positions
- **Multi-threading** - Parallel search capability
- **Pondering** - Thinks during opponent's time
- **Time Management** - Adjustable aggression levels and Adaptive time usage based on position complexity
- **Chess960 Support** - Supports Fischer Random Chess variant

## Build

CeeChess can be built on Windows, Linux, and Android platforms.

### Prerequisites
- GCC or compatible C compiler
- Make

### Building from Source

```bash
# Navigate to the CeeChess directory
cd CeeChess

# Build for your current platform
make

# Or specify a target platform
make TARGET_OS=windows  # For Windows
make TARGET_OS=linux    # For Linux
make TARGET_OS=android  # For Android
```

### Customizing the Build

You can customize the build by overriding variables:

```bash
# Customize name and version
make NAME=cee VERSION=2.0

# Cross-compile for Windows on Linux
make TARGET_OS=windows CC=x86_64-w64-mingw32-gcc

# For Android
make TARGET_OS=android CC=/path/to/android-ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android21-clang
```

### Cleaning
```bash
make clean
```

For more build options, run `make help`.

## Rating
**Rating:**
The rating for the latest release of the engine (v1.4), scores ~150 elo better in self-play to v1.3.2, and should play at ~2300 CCRL (since self-play typically inflates ratings). This compares roughly to FIDE 2500, although there is no real 1-1 correspondence between these rating systems.

See the current CCRL rating here: https://www.computerchess.org.uk/ccrl/4040/cgi/engine_details.cgi?match_length=30&print=Details&each_game=0&eng=CeeChess%201.4%2064-bit#CeeChess_1_4_64-bit

*Update*: v1.4 has broken 2400 CCRL! Turns out in this update, self play was deflating the true improvement somehow

Self play ratings for all versions, anchored at SeeChess 1.0 (1.4 is the newest version):   
(note, self-play tests were conducted at low time controls, and elo may be inflated in comparison to play against a gauntlet variety of engines. If I had to guess, CeeChess 1.4 will likely land ~2400 elo CCRL)
```
Rank      Name             Elo
 1    CeeChess-v1.4    :  ~2480
 2    CeeChess-v1.3.2  :  ~2330
 3    CeeChess-v1.3.2  :  ~2330
 4    CeeChess-v1.3.1  :  ~2315
 5    CeeChess-v1.3    :  ~2310
 6    SeeChess-v1.2    :  ~2200
 7    SeeChess-v1.1.3  :  ~2180
 8    SeeChess-v1.1.2  :  ~2165
 9    SeeChess-v1.1.1  :  ~2150
10    SeeChess-v1.1    :  ~2140
11    SeeChess-v1.0    :  ~2060
```
Most recent gauntlet with an assortment of engines:   
Time Control: (1 min, 0.5sec inc), with elo centered around the v1.4 release (ratings from bayeselo):
| Rank | Name                      | Elo  |  +  |  -  | Games | Score | Oppo. | Draws |
|------|---------------------------|------|-----|-----|-------|-------|-------|-------|
|   1  | Barbarossa-0.6.0         | 38  |  34 |  33 |  240  |  55%  |   95  |  23%  |
|   2  | CeeChess-v1.4    |  0  |  13 |  13 | 1664  |  65%  |  -13  |  26%  |
|   3  | Barbarossa-0.5.0-win10-64|  -34  |  33 |  33 |  240  |  45%  |   95  |  28%  |
|   4  | Kingfisher.v1.1.1        | -107  |  32 |  33 |  240  |  34%  |   95  |  36%  |
|   5  | gopher_check             | -146  |  34 |  35 |  238  |  29%  |   95  |  26%  |
|   6  | CeeChess 1.3.2           | -149  |  34 |  36 |  238  |  29%  |   95  |  25%  |
   ...
   
Since CCRL ratings got adjusted down recently (stockfish went from 3900 CCRL to ~3630 afaik), this no longer breaks the CCRL 2400 barrier, but comparing the results here to the old ratings of Barbarossa-0.6.0(2468), Barbarossa-0.5.0(~2375ish i believe?) and the others suggests that this release would have broken that barrier. I now expect the engine to land in the range of 2300-2350, given Barbarossa-0.6.0 has a new rating of 2355

## Disclaimer
None of the code I write is copyrighted or protected in any way, and you may make use of all that you wish. You do not have to credit me if you use any of the code I write, but it would be great if you did!