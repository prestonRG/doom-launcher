# Doom Launcher

A cross-platform Doom launcher for organizing and launching games, megawads, and mods.

## What it does

- Organizes your Doom mods and megawads in a clean directory structure
- Lets you reorder mods
- Launches Doom with your selected engine, IWAD, megawads, and mods
- Saves and loads reusable launch profiles
- Launches configured installations of SLADE and Ultimate Doom Builder
- Lets you back up your config files

## Requirements

- Windows or Linux
- Qt 6 Widgets
- GZDoom, UZDoom, Chocolate Doom, Zandronum, or another compatible engine installed
- Doom WAD files (doom.wad, doom2.wad, etc.)
- C++17 compiler and CMake 3.16 or newer

## Directory Structure

The launcher creates this structure in `~/Games/DOOM/` (the home directory is resolved
appropriately on Windows and Linux):

```
~/Games/DOOM/
├── iwads/          # Put your official WAD files here (doom.wad, doom2.wad, etc.)
├── megawads/       # Put large map packs here
├── gzd/
│   └── mods/       # Put GZDoom/UZDoom mods here
├── zand/
│   └── mods/       # Put Zandronum-specific mods here
├── mods/           # Put mods for other engines here
└── profiles/       # Saved launch profiles
```

## Building

Install CMake, a C++17 compiler, and Qt 6 Widgets for the same compiler. Keep separate
build directories for each operating system. For example, on Windows with Qt 6.12 and
Visual Studio 2022:

```powershell
cmake -S . -B build-windows -G "Visual Studio 17 2022" -A x64 `
  -DCMAKE_PREFIX_PATH="C:/Qt/6.12.0/msvc2022_64"
cmake --build build-windows --config Debug
```

On Linux:

```bash
cmake -S . -B build-linux
cmake --build build-linux
```

On Windows, the build automatically deploys Qt runtime DLLs and the platform plugin next
to the executable so it can start without Qt added to `PATH`.

## Usage

Run `build-windows/Debug/doom-launcher.exe` on Windows, or `build-linux/doom-launcher` on
Linux, to open the desktop launcher. The engine field accepts an executable
name found on `PATH` or a full executable path; use **Browse...** to select one. Put IWADs
in `iwads/`, megawads in `megawads/`, and mods in the folder corresponding to the selected
engine. Select multiple entries with Ctrl/Shift; drag mod entries to change their load order.
Save the current selection as a named profile to reuse it later.

The **External tools** section lets you select and launch installed copies of SLADE
and Ultimate Doom Builder. Their executable paths are saved in the application settings.

The previous terminal interface remains available with `doom-launcher --cli`.

## Current Status

The initial Qt Widgets desktop interface supports engine and WAD selection, mod load order,
launch profiles, launching games, and launching configured external tools. ModDB
discovery/downloads, tool downloads, direct editor integration, and additional
engine-specific launch options are not implemented yet.
