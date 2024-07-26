# Mufasa

<div align="center">
<img src="./assets/icon.png" style="width:25%;">
<br>
<b><i>Chess engine written in C++</i></b>
</div>

## Overview

Mufasa (wordplay on soundalike phrase **"move faster"**) is a fast single threaded [bitboard](https://www.chessprogramming.org/Bitboards) chess engine written in C++17 with the help of CMake and GTest framework.

Mufasa's move generator can traverse over **80,000,000** nodes (moves) per second during bulk counted [perft](https://www.chessprogramming.org/Perft).

## Compilation

Make sure you have C++17 and CMake installed on your desktop.

Clone the repository and run
```sh
    cmake . -DCMAKE_BUILD_TYPE=Release && cmake --build .
```

## Testing

If you want to compile the binary with tests run
```sh
    cmake . -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON && cmake --build .
```

## Contributing

### Reporting issues

If you find a bug with move generation or search results feel free to report them on the [issue tracker](https://github.com/kanin9/mufasa/issues).

### Submitting pull requests

If you want to submit a pull request make sure you provide a clean description of your addition and a test suite if applicable.  

## Features

### Move generation
- Bitboard representation
- Magic tables for sliding pieces
- Precomputed tables for other pieces

### Search
- Negamax with A/B pruning
- Quiescence search
- Iterative deepening
- Move ordering with MVV/LVA

### Evaluation
- Material counting
- Piece Square Tables

## Inspirations

- [Stockfish](https://github.com/official-stockfish/Stockfish)
- [Boychesser](https://github.com/analog-hors/Boychesser)
- [Gigantua](https://github.com/Gigantua/Gigantua)
- [CodingAdventureBot](https://github.com/SebLague/Chess-Coding-Adventure)

## License

Mufasa uses [GNU GPLv3](https://choosealicense.com/licenses/gpl-3.0/), which means you can do anything with the project, **except** distributing closed source versions.