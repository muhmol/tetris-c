# tetris-c

A polished terminal-based Tetris clone written in C, built with CMake and designed to be easy to run, extend, and maintain.

![Windows](https://img.shields.io/badge/platform-Windows%2010%2F11-0078D6)
![Language](https://img.shields.io/badge/language-C17-00599C)
![Build](https://img.shields.io/badge/build-CMake-064F8C)
![License](https://img.shields.io/badge/license-MIT-green)

## Overview

`tetris-c` is a classic falling-block puzzle game built entirely in the terminal. The project is organized into small modules for gameplay, rendering, and board logic, making it easy to understand and improve.

## Features

- Full 7-piece tetromino set with rotation logic
- Collision detection for walls, floor, and placed blocks
- Line clearing with scoring and level progression
- Pause, quit, and replay flow for a smooth game loop
- Terminal rendering with color and centered UI layout
- CMake-based build pipeline for repeatable builds

## Quick start

### Requirements

- CMake 3.20 or newer
- A C17-compatible compiler
- Windows 10/11 for the current console implementation

### Configure and build

```bash
git clone https://github.com/muhmol/tetris-c.git
cd tetris-c
cmake --preset default
cmake --build --preset default
```

### Run

```bash
./build/tetris.exe
```

## Project structure

```text
tetris-c/
├── CMakeLists.txt
├── CMakePresets.json
├── README.md
├── LICENSE
├── .gitignore
├── .clang-format
├── .editorconfig
├── .github/
│   └── workflows/
│       └── ci.yml
├── src/
│   ├── main.c
│   ├── board.c
│   ├── board.h
│   ├── game.c
│   ├── game.h
│   ├── piece.c
│   ├── piece.h
│   ├── render.c
│   └── render.h
└── screenshots/
```

## Controls

| Key | Action |
| --- | --- |
| `A` | Move left |
| `D` | Move right |
| `S` | Soft drop |
| `W` | Rotate |
| `Space` | Hard drop |
| `P` | Pause / resume |
| `Q` | Quit |

## Development notes

The project intentionally keeps the major systems separated:

- `piece` handles piece definitions and rotation math
- `board` handles collision, placement, and line clears
- `game` tracks score, level, and state transitions
- `render` owns the console drawing and user prompts
- `main` ties the flow together in the gameplay loop

This keeps the code easier to maintain and makes future improvements such as UI polish, stronger game rules, or portability work more manageable.

## Contributing

Please see [CONTRIBUTING.md](CONTRIBUTING.md) for coding conventions, build expectations, and contribution workflow.

## Roadmap

- Add a next-piece preview panel
- Add hold-piece logic
- Improve rotation behavior near walls and stacked blocks
- Save high scores to disk
- Add cross-platform input handling

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.
