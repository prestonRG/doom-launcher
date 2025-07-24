doom mod launcher
made by preston

# Doom Launcher

A mod launcher for Doom games that helps organize and launch mods with GZDoom and Zandronum.

I plan to give this a gui at some point, but probably not until I feel it is in a good state.

## What it does

- Organizes your Doom mods and megawads in a clean directory structure
- Lets you reorder mods
- Launches Doom with your selected engine, game, megawads, and mods
- Lets you backup up your config files

## Requirements

- Linux system
- GZDoom and/or Zandronum installed
- Doom WAD files (doom.wad, doom2.wad, etc.)
- C++ compiler with C++17 support

I would like to add support for windows and mac but am only making it for linux currently as that is my os.
I think the only thing that would prevent it from working on another os atm is the way the script currently finds the users folders.

## Directory Structure

The launcher creates this structure in `~/Games/DOOM/`:

```
~/Games/DOOM/
├── iwads/          # Put your official WAD files here (doom.wad, doom2.wad, etc.)
├── megawads/       # Put large map packs here
├── gzd/
│   └── mods/       # Put GZDoom-specific mods here
└── zand/
    └── mods/       # Put Zandronum-specific mods here
```

## Building

```bash
g++ -std=c++17 -o doom_launcher main.cpp launcher.cpp mod_scanner.cpp -lstdc++fs
```

## Usage

Right now the user would have to build the project themselves with cmake and then launch it from the terminal to use this.

## About

I started making this after falling in love with the original dooms and then modding them. I know there are other mod loaders out there, but I have never used them. I am pretty new to modding, and at first, I was launching my mods by copying and pasting the commands in my terminal. I eventually made a script to make it easier for myself, but kept wanting to add more and more features. I decided to turn this into an actual project and use it to finally start learning how to program. If my code, structure, or methods seem ridiculous, that's probably why.

## Contributing

This is just a personal project, but suggestions and feedback are welcome!

==================================================================================================================

-preston
