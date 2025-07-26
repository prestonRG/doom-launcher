#pragma once
#include <vector>
#include <string>
#include <iostream>

class DoomLauncher {
private:
    int menuState = 0;
    std::string doomEngine;
    std::vector<std::string> modList;
    std::vector<std::string> megawadList;
    std::vector<std::string> pickedMegawads;
    std::string iwad;

    // Default doom path
    std::string doomDir;
    std::string gzBackupDir;
    std::string zandBackupDir;
    std::string setupDoomDirectory();

    // Save config
    void saveConfig(const std::string& configName);
    void loadConfig(const std::string& configName);

    // Mod vectors
    std::vector<std::string> megawads;
    std::vector<std::string> vanillaPlus;
    std::vector<std::string> brutalMods;
    std::vector<std::string> brutalPlatinum;
    std::vector<std::string> projectBrutality;
    std::vector<std::string> brutalDoom64;
    std::vector<std::string> zandronum;

public:
    DoomLauncher();

    void run();

    // Menus
    // I have the menus in this order with the idea that -
    // 1. you first pick the engine you want to use
    // 2. next, you pick the game (iwad) you want to play on that engine
    // 3. then, you pick the megawads you want to use (if any) as these will use the iwad that was just selected (most use doom2.wad)
    // 4. finally, you can select the mods you want to play with your chosen game
    // So its like 3 choices to make over 4 steps (for example, if you want to play doom ii, you pick doom2.wad. if you want to play eviternity,
    //  you have to still pick doom2.wad and then pick eviternity.)
    // backupMenu() is optional and available to select from any other menu (except for a few maybe. i think you cant from modMenu's submenu,
    //  modConfiguration())
    void engineMenu();
    void gameMenu();
    void megawadMenu();
    void modMenu();
    void backupMenu();
    //enginemenu > pick gzdoom or zandronum (or whatever else)
    //gamemenu > pick game > doom.wad doom2.wad etc.
    //megawadmenu > pick map pack (if any)
    //modmenu > pick mods
    //backupmenu > option menu for backups (,ini files, saves, whatever else)

    // Selection menu functions
    int engineSelections();
    int modSelections();
    int wadSelections();
    int backupSelections();
    int configureMods();

    std::vector<std::string> gzdScanner();
    std::vector<std::string> zandScanner();
    std::vector<std::string> megawadScanner();
    void loadOrderMenu();

    // Launch functions
    [[noreturn]] void launchDoom(const std::string& iwad, const std::vector<std::string>& pickedMegawads, const std::vector<std::string>& modList);
    void launchBD64();

    // Colors
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string RESET = "\033[0m";
};
