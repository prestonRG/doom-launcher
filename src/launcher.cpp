#include "launcher.h"
#include "mod_scanner.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <chrono>

std::string DoomLauncher::setupDoomDirectory() {
    const char* home = std::getenv("HOME");

    if (!home) {
        throw std::runtime_error("Could not find home directory...");
    }

    std::filesystem::path doomPath = std::filesystem::path(home) / "Games" / "DOOM";

    if (!std::filesystem::exists(doomPath)) {
        std::filesystem::create_directories(doomPath);
        std::cout << "Created DOOM directory at: " << doomPath << std::endl;
    }

    std::filesystem::create_directories(doomPath / "gzd" / "mods");
    std::filesystem::create_directories(doomPath / "iwads");
    std::filesystem::create_directories(doomPath / "megawads");
    std::filesystem::create_directories(doomPath / "zand" / "mods");

    return doomPath.string();
}

// Functions to find files
std::vector<std::string> DoomLauncher::gzdScanner() {
    std::filesystem::path modPath = std::filesystem::path(setupDoomDirectory()) / "gzd" / "mods";

    modList = findMods(modPath);

    return modList;
}
std::vector<std::string> DoomLauncher::zandScanner() {
    std::filesystem::path modPath = std::filesystem::path(setupDoomDirectory()) / "zand" / "mods";

    modList = findMods(modPath);

    return modList;
}
std::vector<std::string> DoomLauncher::megawadScanner() {
    std::filesystem::path megawadPath = std::filesystem::path(setupDoomDirectory()) / "megawads";

    megawadList = findMegawads(megawadPath);

    return megawadList;
}

// Constructor - initialize mod arrays
DoomLauncher::DoomLauncher() {
    // Setup directories
    doomDir = setupDoomDirectory();

    const char* home = std::getenv("HOME");

    if (!home) {
        throw std::runtime_error("Could not find home directory...");
    }

    std::string homeStr = std::string(home);
    gzBackupDir = homeStr + "/.config/gzdoom/gzdoom.ini";
    zandBackupDir = homeStr + "/.config/zandronum/zandronum.ini";
}

// Launches doom with all selected settings.
// Needs to be fixed but I am too tired right now 7/22/25 10:23pm
void DoomLauncher::launchDoom(const std::string& iwad, const std::vector<std::string>& pickedMegawads) {
    std::string command = doomEngine + " -iwad " + doomDir + "/iwads/" + iwad + " -file";

    // Adds mods from modList if there are any.
    if (!modList.empty()) {
        for (const std::string& mod : modList) {
            command += " \"" + mod + "\"";
        }
    }

    // Adds megawads if any were selected.
    if (!pickedMegawads.empty()) {
        for (const std::string& megawad : pickedMegawads) {
            command += " \"" + megawad + "\"";
        }
    }

    std::cout << "Launching with command: " << command << std::endl;

    // Execute command
    std::system(command.c_str());
    exit(0);
};

// Engine menu
void DoomLauncher::engineMenu() {
    std::cout << "[1] GZDOOM" << std::endl;
    std::cout << "[2] ZANDRONUM" << std::endl;
    std::cout << std::endl;
    std::cout << BLUE << "[B] Make a backup" << RESET << std::endl;
    std::cout << RED << "[0] Exit" << RESET << std::endl;

    std::string choice;
    std::cout << "Select an engine to use [1-2]: ";
    std::cin >> choice;

    // Immediately gets the relevant mods when an engine is selected.
    if (choice == "1") {
        doomEngine = "gzdoom";
        gzdScanner();
        gameMenu();
    }
    else if (choice == "2") {
        doomEngine = "zandronum";
        zandScanner();
        gameMenu();
    }
    else if (choice == "0") {
        std::cout << "Exiting..." << std::endl;
        exit(0);
    }
    else if (choice == "B") {
        backupMenu();  // Go to backup menu
    }
    else {
        std::cout << "Invalid selection! Try again." << std::endl;
        engineMenu();  // Stay on engine menu
    }
}

// Game menu
// Could make it dynamic, but these are the only iwads I have and use
// IDK how many iwads there are, but it might be easier to just add more elements to
//  this menu than making it dynamic, at least for now.
void DoomLauncher::gameMenu() {
    std::cout << "[1] Doom" << std::endl;
    std::cout << "[2] Doom II" << std::endl;
    std::cout << "[3] Plutonia" << std::endl;
    std::cout << "[4] TNT" << std::endl;
    std::cout << std::endl;
    std::cout << BLUE << "[B] Make a backup" << RESET << std::endl;
    std::cout << RED << "[0] Exit" << RESET << std::endl;

    std::string choice;
    std::cout << "Select a game to use [1-4]: ";
    std::cin >> choice;

    if (choice == "1") {
        iwad = "doom.wad";
        megawadMenu();
    }
    else if (choice == "2") {
        iwad = "doom2.wad";
        megawadMenu();
    }
    if (choice == "3") {
        iwad = "plutonia.wad";
        megawadMenu();
    }
    else if (choice == "4") {
        iwad = "tnt.wad";
        megawadMenu();
    }
    else if (choice == "0") {
        std::cout << "Exiting..." << std::endl;
        exit(0);
    }
    else if (choice == "B") {
        backupMenu();  // Go to backup menu
    }
    else {
        std::cout << RED << "Invalid selection! Try again." << RESET << std::endl;
        gameMenu();  // Stay on game menu
    }
}
// Megawad menu
void DoomLauncher::megawadMenu() {
    megawadScanner();
    // Display megawad options.
    std::cout << CYAN << "Available megawads:" << RESET << std::endl;
    for (int i = 0; i < megawadList.size(); i++) {
        int position = i + 1;
        std::cout << "[" << position << "] ";
        std::cout << megawadList[i] << std::endl;
    }
    std::cout << std::endl;
    // doesnt work right now in this menu because of int choice
    //std::cout << "#B) Make a backup" << std::endl;
    std::cout << RED << "[0] Exit" << RESET << std::endl;

    int choice1;
    std::cout << "Select a megawad to play with or type \"100\" to continue: ";
    std::cin >> choice1;

    if (choice1 == 100) {
        modMenu();
    }
    else if (choice1 >= 1 && choice1 <= megawadList.size()) {
        int selected = choice1 - 1;

        std::string chosenWad = megawadList[selected];
        std::cout << "You have selected " << megawadList[selected] << std::endl;

        pickedMegawads.push_back(chosenWad);
        
    }
    else if (choice1 == 0) {
        std::cout << "Exiting..." << std::endl;
        exit(0);
    }
    else {
        std::cout << "Invalid selection! Try again." << std::endl;
        megawadMenu();
    }

    std::string choice2;
    std::cout << "Select another " << GREEN << "(y)?" << RESET << " Or press 'c' to continue: ";
    std::cin >> choice2;

    if (choice2 == "y") {
        megawadMenu();
    }
    else if (choice2 == "c") {
        modMenu();
    }
    else {
        std::cout << RED << "Invalid selection! Try again." << RESET << std::endl;
        megawadMenu();
    }
}

// Configure mods is a submenu of modMenu()
int DoomLauncher::configureMods() {
    // Visually list mods in current order again.
    for (int i = 0; i < modList.size(); i++) {
        int position = i + 1;
        std::cout << "[" << position << "] ";
        std::cout << modList[i] << std::endl;
    }
    // Prompts user to select a mod to swap.
    int choice;
    std::cout << "Select a mod " << GREEN << "[1-" << modList.size() << "]: " << RESET;
    std::cin >> choice;

    if (choice >= 1 && choice <= modList.size()) {
        int selected = choice - 1;

        // Store number selected AND modList[selected]. Or, just use selected and modList[selected]. Those should be the two values I need.
        // Consider updating the visual menu after to show the mod that has been selected on the menu, not just by text. Might be too complex for now and
        //  not needed for functionality.
        std::string firstMod = modList[selected];
        int firstNumber = selected;

        std::cout << "You selected: " << modList[selected] << std::endl;

        // Prompt user again for mod to swap with.
        // Not sure if choice2 is the best naming scheme... Should first be choice1?
        int choice2;
        std::cout << "Select a mod to swap with" << GREEN << "[1-" << modList.size() << "]: " << RESET;
        std::cin >> choice2;

        if (choice2 >= 1 && choice2 <= modList.size()) {
            int selected2 = choice2 - 1;
            // Store second number and mod and swap places with the first selection. The numbers WILL NOT move. They are there as a reference
            //  for where to place the two stored mods.
            std::string secondMod = modList[selected2];
            int secondNumber = selected2;

            // Mods swap positions in the vector.
            modList[firstNumber] = secondMod;
            modList[secondNumber] = firstMod;
            std::cout << "Swapped mod " << firstMod << " with " << secondMod << std::endl;

            // Displays modList again so the user can see their new configuration.
            for (int i = 0; i < modList.size(); i++) {
                int position = i + 1;
                std::cout << position << ") ";
                std::cout << modList[i] << std::endl;
            }

            // Prompts to either continue configuring mods or continue with current configuration.
            std::cout << "[1] Swap another mod." << std::endl;
            std::cout << "[2] Done." << std::endl;

            int whatToDo;
            std::cout << "What would you like to do?: ";
            std::cin >> whatToDo;

            if (whatToDo == 1) {
                configureMods();
            }
            else if (whatToDo == 2) {
                launchDoom(iwad, pickedMegawads);
            }
            else {
                // Make these either run configureMods() again or figure out how to just prompt for reselect without having to run from scratch.
                configureMods();
                std::cout << "Invalid choice!" << std::endl;
            }

        }
        else {
            configureMods();
            std::cout << "Invalid choice!" << std::endl;
        }
    }
    else {
        configureMods();
        std::cout << "Invalid choice!" << std::endl;
    }

    return 0;
}

// Mod menu
void DoomLauncher::modMenu() {
    // Creates the list of mods
    std::cout << "Available mods:" << std::endl;
    for (int i = 0; i < modList.size(); i++) {
        int position = i + 1;
        std::cout << position << ") ";
        std::cout << modList[i] << std::endl;
    }
    std::cout << std::endl;
    std::cout << "[1] Run with selected mod order" << std::endl;
    std::cout << "[2] Configure mod order" << std::endl;
    std::cout << BLUE << "B) Make a backup" << RESET << std::endl;
    std::cout << RED << "0) Exit" << RESET << std::endl;

    std::string choice;
    std::cout << "What do you want to do?: ";
    std::cin >> choice;

    if (choice == "1") {
        launchDoom(iwad, pickedMegawads);
    }
    else if (choice == "2") {
        configureMods();
    }
    else if (choice == "B") {
        backupMenu();
    }
    else if (choice == "0") {
        exit(0);
    }
    else {
        std::cout << RED << "Invalid selection! Try again." << RESET << std::endl;
        modMenu();  // Stay on mod menu
    }
    
}
// Backup menu
void DoomLauncher::backupMenu() {
    std::cout << BLUE << "[G] Backup gzdoom.ini" << RESET << std::endl;
    std::cout << BLUE << "[Z] Backup zandronum.ini" << RESET << std::endl;
    std::cout << std::endl;
    std::cout << RED << "[0] Exit" << RESET << std::endl;

    std::string choice;
    std::cout << "Choose which game to make a backup of: ";
    std::cin >> choice;

    // G or Z copies the selected .ini file into an easy to access backup directory.
    if (choice == "G") {
        std::cout << "Backing up GZDoom config to ~/Games/DOOM/gzd/backups/gzdoom.ini" << std::endl;
        std::filesystem::copy_file(
            gzBackupDir,
            doomDir + "/gzd/gzdoom.ini"
        );
        engineMenu();
    }
    else if (choice == "Z") {
        std::cout << "Backing up GZDoom config to ~/Games/DOOM/zand/backups/zandronum.ini" << std::endl;
        std::filesystem::copy_file(
            zandBackupDir,
            doomDir + "/zand/zandronum.ini"
        );
        engineMenu();
    }
    else if (choice == "0") {
        std::cout << "Exiting..." << std::endl;
        exit(0);
    }
    else {
        std::cout << RED << "Invalid selection! Try again." << RESET << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        backupMenu();  // Stay on backup menu
    }
}

// Main loop
void DoomLauncher::run() {
    engineMenu();
}
