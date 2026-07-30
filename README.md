# <img src="assets/preview/icon_concept1.png" width="30"/> Pixel Pulse

A retro-style game hub with a collection of classic mini games built with C++20, SFML 3.0.2, and CMake.

## Preview

![Menu Preview](assets/preview/homescreen-preview.png)

## Games

| Game | Description |
| ---- | ----------- |
| Snake | Classic snake game with organ-themed food and sprite-based rendering |
| Flappy Bird | Side-scrolling obstacle avoidance with animated backgrounds |
| Rocket Shooter | Grid-based space shooter with asteroids, coins, and power-ups |
| Pac-Man | Full Pac-Man clone with procedurally generated maps, ghost AI, and menu system |
| Dino Run | Endless runner with obstacles, food collection, and dynamic difficulty |

## Built With

![C++](https://img.shields.io/badge/C++-00599C?style=for-the-badge&logo=cplusplus&logoColor=white)
![SFML](https://img.shields.io/badge/SFML-8CC445?style=for-the-badge&logo=sfml&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-064F8C?style=for-the-badge&logo=cmake&logoColor=white)

## Requirements

- Visual Studio 2022+
- CMake 3.28+
- Git

## Team

| Member | Game |
| ------ | ---- |
| <a href="https://github.com/Aashutosh-kc">Aashutosh KC</a> | Snake |
| <a href="https://github.com/AbhiDevkota">Abhi Devkota</a> | Menu, Pac-Man, File Management |
| <a href="https://github.com/PrabinDhungana000">Prabin Dhungana</a> | Flappy Bird |
| <a href="https://github.com/Samyam1070">Samyam Khadka</a> | Rocket Shooter |
| <a href="https://github.com/sunim4125">Sunim Fuyal</a> | Dino Run |

## How to Run

1. Clone the repo
```bash
git clone https://github.com/AbhiDevkota/PixelPulse.git
cd PixelPulse
```
2. Configure and build with CMake
```bash
cmake -S . -B build
cmake --build build
```
3. Run the executable
```bash
./build/bin/Debug/PixelPulse.exe
```

Alternatively, open `CMakeLists.txt` in Visual Studio, let CMake configure, select **PixelPulse.exe** as startup item, and run.

## Documentation

- [File Management System](wiki/file_management.md) — how save/load, asset resolution, and volume persistence work
- [Score & HighScore Refactoring](wiki/score_highscore_refactor.md) — OOP inheritance, polymorphism, and centralized file I/O across all games



