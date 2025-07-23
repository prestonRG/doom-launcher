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

    // Default doom path.
    std::string doomDir;
    std::string gzBackupDir;
    std::string zandBackupDir;
    std::string setupDoomDirectory();

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
    void engineMenu();
    void gameMenu();
    void modMenu();
    void megawadMenu();
    void backupMenu();

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
    [[noreturn]] void launchDoom(const std::string& iwad, const std::vector<std::string>& pickedMegawads);
    void launchBD64();
};
