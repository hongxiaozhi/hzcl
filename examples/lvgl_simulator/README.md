# LVGL PC Simulator — HZCL Demo

PC simulator examples for the HZCL framework, built with **LVGL v9 + SDL2** using **CMake + MinGW GCC**.

## Prerequisites

| Tool | Notes |
|------|-------|
| **CMake** | Build system generator (MSYS2: `pacman -S mingw-w64-x86_64-cmake`) |
| **MinGW GCC** | C compiler (included with MSYS2) |
| **make** | Build driver (MSYS2: `pacman -S mingw-w64-x86_64-make`) |

> If using MSYS2, run pacman commands inside **Mingw64.exe** terminal.

## Build

```bash
# 1. Initialize LVGL submodule
git submodule update --init

# 2. Enter the example directory
cd examples/lvgl_simulator/desk_pet

# 3. Configure and build
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make -j8

# 4. Run
./desk_pet.exe
```

> The first cmake run automatically downloads SDL2 source (release-2.30.9) from GitHub
> and compiles it. Internet connection required.

## Controls

| Button | Action |
|--------|--------|
| 🍖 **Feed** | Feed the pet (lowers hunger) |
| 👋 **Pet**  | Pet the pet (boosts mood) |
| 💤 **Sleep**| Put the pet to sleep (restores energy) |

The pet's expression and state (idle, happy, eating, sleeping, sad) change automatically based on status thresholds.

## Directory structure

```
lvgl_simulator/
├── desk_pet/           Current example
│   ├── CMakeLists.txt
│   ├── inc/lv_conf.h   LVGL configuration
│   └── src/
│       ├── main.c      Entry point and UI
│       └── hz_platform_sdl2.c  SDL2 platform adaptation
└── lvgl/               Git submodule
```
