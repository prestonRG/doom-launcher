#include "launcher.h"
#include "mod_scanner.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <thread>
#include <chrono>
#include <fstream>

void DoomLauncher::saveConfig(const std::string& configName) {
    std::string configPath = doomDir + "/" + configName + ".ini";

    // Open/create .ini file
    // This is for writing to the file
    std::ofstream configFile(configPath);

    if (!configFile.is_open()) {
        std::cout << "Error: could not create a config file." << std::endl;
        return;
    }

    // Writes everything into the file.
    // name, engine, and iwad will all have one value while megawads and mods could have none or many.
    configFile << "[Setup]" << std::endl;
    configFile << "name=" << configName << std::endl;
    configFile << "engine=" << doomEngine << std::endl;
    configFile << "iwad=" << iwad << std::endl;
    configFile << std::endl;

    // Write mods section
    configFile << "[Mods]" << std::endl;
    for (int i = 0; i < modList.size(); i++) {
        configFile << "mod" << (i + 1) << "=" << modList[i] << std::endl;
    }
    configFile << std::endl;

    // Write megawads section
    configFile << "[Megawads]" << std::endl;
    for (int i = 0; i < pickedMegawads.size(); i++) {
        configFile << "megawad" << (i + 1) << "=" << pickedMegawads[i] << std::endl;
    }

    configFile.close();

    std::cout << "Configuration saved as: " << configPath << std::endl;
}

// Open the .ini to read what's in it.
void DoomLauncher::loadConfig(const std::string& configName) {
    std::string configPath = doomDir + "/" + configName + ".ini";
    std::ifstream configFile(configPath);

    if (!configFile.is_open()) {
        std::cout << "Error: could not find config file: " << configPath << std::endl;
        return;
    }

    std::string line;
    std::string currentSection = "";

    // Reads line by line
    while (std::getline(configFile, line)) {
        // Skip empty lines
        if (line.empty()) continue;

        // Check for section headers
        if (line[0] == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2); // Removes [ and ]
            continue;
        }

        // Look for key=value pairs
        size_t equals = line.find('=');
        if (equals == std::string::npos) continue; // Skip lines without =

        std::string key = line.substr(0, equals);
        std::string value = line.substr(equals + 1);

        // Handle different sections
        if (currentSection == "Setup") {
            if (key == "engine") {
                doomEngine = value;
            } else if (key == "iwad") {
                iwad = value;
            }
        }
        else if (currentSection == "Mods") {
            if (key.substr(0, 3) == "mod") {
                modList.push_back(value);
            }
        }
        else if (currentSection == "Megawads") {
            if (key.substr(0, 8) == "megawad") {
                pickedMegawads.push_back(value);
            }
        }
    }

    configFile.close();
    
    std::cout << "Configuration loaded: " << configName << std::endl;
    std::cout << "Engine: " << doomEngine << ", IWAD: " << iwad << std::endl;
    std::cout << "Loaded " << modList.size() << " mods and " << pickedMegawads.size() << " megawads" << std::endl;
}



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
    std::cout << "1 GZDOOM" << std::endl;
    std::cout << "2 ZANDRONUM" << std::endl;
    std::cout << std::endl;
    std::cout << "#B) Make a backup" << std::endl;
    std::cout << "#0) Exit" << std::endl;

    std::string choice;
    std::cout << "Select an engine to use (1-2): ";
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
void DoomLauncher::gameMenu() {
    std::cout << "1 Doom" << std::endl;
    std::cout << "2 Doom II" << std::endl;
    std::cout << "3 Plutonia" << std::endl;
    std::cout << "4 TNT" << std::endl;
    std::cout << std::endl;
    std::cout << "#B) Make a backup" << std::endl;
    std::cout << "#0) Exit" << std::endl;

    std::string choice;
    std::cout << "Select a game to use (1-4): ";
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
        std::cout << "Invalid selection! Try again." << std::endl;
        gameMenu();  // Stay on game menu
    }
}
// Megawad menu
void DoomLauncher::megawadMenu() {
    megawadScanner();
    // Display megawad options.
    std::cout << "Available megawads:" << std::endl;
    for (int i = 0; i < megawadList.size(); i++) {
        int position = i + 1;
        std::cout << position << ") ";
        std::cout << megawadList[i] << std::endl;
    }
    std::cout << std::endl;
    // doesnt work right now in this menu because of int choice
    //std::cout << "#B) Make a backup" << std::endl;
    std::cout << "#0) Exit" << std::endl;

    int choice1;
    std::cout << "Select a megawad to play with or press '100' to continue: ";
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
    std::cout << "Select another? Or press 'c' to continue: ";
    std::cin >> choice2;

    if (choice2 == "y") {
        megawadMenu();
    }
    else if (choice2 == "c") {
        modMenu();
    }
    else {
        std::cout << "Invalid selection! Try again." << std::endl;
        megawadMenu();
    }
}

// Configure mods is a submenu of modMenu()
int DoomLauncher::configureMods() {
    // Visually list mods in current order again.
    for (int i = 0; i < modList.size(); i++) {
        int position = i + 1;
        std::cout << position << ") ";
        std::cout << modList[i] << std::endl;
    }
    // Prompts user to select a mod to swap.
    int choice;
    std::cout << "Select a mod" << "(1-" << modList.size() << "): ";
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
        std::cout << "Select a mod to swap with" << "(1-" << modList.size() << "): ";
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
            std::cout << "1) Swap another mod." << std::endl;
            std::cout << "2) Done." << std::endl;

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
    std::cout << "1) Run with selected mod order" << std::endl;
    std::cout << "2) Configure mod order" << std::endl;
    std::cout << "[3] Save current setup" << std::endl;
    std::cout << "B) Make a backup" << std::endl;
    std::cout << "0) Exit" << std::endl;

    std::string choice;
    std::cout << "What do you want to do?: ";
    std::cin >> choice;

    if (choice == "1") {
        launchDoom(iwad, pickedMegawads);
    }
    else if (choice == "2") {
        configureMods();
    }
    else if (choice == "3") {
        std::string configName;
        std::cout << "Type a name for your config: ";
        std::cin.ignore(); // Clears any leftover newline so getline can read the user's input instead.
        std::getline(std::cin, configName);
        saveConfig(configName);
    }
    else if (choice == "B") {
        backupMenu();
    }
    else if (choice == "0") {
        exit(0);
    }
    else {
        std::cout << "Invalid selection! Try again." << std::endl;
        modMenu();  // Stay on mod menu
    }
    
}
// Backup menu
void DoomLauncher::backupMenu() {
    std::cout << "#G) Backup gzdoom.ini" << std::endl;
    std::cout << "#Z) Backup zandronum.ini" << std::endl;
    std::cout << std::endl;
    std::cout << "#0) Exit" << std::endl;

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
        std::cout << "Invalid selection! Try again." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        backupMenu();  // Stay on backup menu
    }
}

// Main loop
void DoomLauncher::run() {
    engineMenu();
}
