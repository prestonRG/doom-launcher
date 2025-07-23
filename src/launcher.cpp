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

// Functions to find mods
std::vector<string> DoomLauncher::gzdScanner() {
    std::filesystem::path modPath = std::filesystem::path(setupDoomDirectory()) / "gzd" / "mods";

    modList = findMods(modPath);

    return modList;
}
std::vector<string> DoomLauncher::zandScanner() {
    std::filesystem::path modPath = std::filesystem::path(setupDoomDirectory()) / "gzd" / "mods";

    modList = findMods(modPath);

    return modList;
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

    // Need to setup a megawad scanner. Shouldn't be too different from the mod scan code. I might even be able to make it one function called doomScanner();
    megawads = {
        // [0-4]
        doomDir + "/megawads/masterlevels.wad",
        doomDir + "/megawads/sigil.wad",
        doomDir + "/megawads/sigil2.wad",
        doomDir + "/megawads/Elementalism_Phase1_Full_Release_v1.3/Elementalism_Phase1_Full_Release_v1.3.pk3",
        doomDir + "/megawads/Eviternity/Eviternity.wad",
        // [5-9]
        doomDir + "/megawads/eviternityii/Eviternity II.wad",
        doomDir + "/megawads/Hellbnd/Hellbnd.wad",
        doomDir + "/megawads/Hellbnd/HBFM29.wad", // Hellbound M29+
        doomDir + "/megawads/ozonia-2.33-RC4-deluxe/ozonia-2.32-RC4.wad",
        doomDir + "/megawads/valiant/Valiant.wad",
        // [10-xx]
        doomDir + "/megawads/Amalgoom_RC3/Amalgoom_RC3.wad",
        doomDir + "/megawads/Amalgoom_RC3/Amalgoom_B-Side_RC3.wad"
    };
}

void DoomLauncher::launchDoom(const std::string& iwad, const std::string& megawad, const std::string& secondMegawad) {
    std::string command = doomEngine + " -iwad " + doomDir + "/iwads/" + iwad + " -file";

    // Add megawads if they exist.
    if (!megawad.empty()) {
        command += " " + megawad;
        if (!secondMegawad.empty()) {
            command += " " + secondMegawad;
        }
    }

    // Add mods from modList vector.
    for (const std::string& mod : modList) {
        command += " " + mod;
    }

    // Execute command
    std::system(command.c_str());

    exit(0);
};

// Main loop
void DoomLauncher::run() {
    // Always starts on the first menu when program is launched.
    menuState = 0;

    // Makes it do hard to debug with these uncommented
    //std::system("clear");

    while (true) {
        //std::system("clear");

        if (menuState == 0) {
            engineVisualMenu();
            menuState = engineSelections();
        }
        else if (menuState == 1) {
            modVisualMenu();
            menuState = modSelections();
        }
        else if (menuState == 2) {
            wadVisualMenu();
            menuState = wadSelections();
        }
        else if (menuState == 3) {
            backupVisualMenu();
            menuState = backupSelections();
        }
    }
}

// Engine selection menu
void DoomLauncher::engineVisualMenu() {
    std::cout << "1 GZDOOM" << std::endl;
    std::cout << "2 ZANDRONUM" << std::endl;
    std::cout << std::endl;
    std::cout << "#B) Make a backup" << std::endl;
    std::cout << "#0) Exit" << std::endl;
}
int DoomLauncher::engineSelections() {
    std::string choice;
    std::cout << "Select an engine to use (1-2): ";
    std::cin >> choice;

    // Immediately gets the relevant mods when an engine is selected.
    if (choice == "1") {
        doomEngine = "gzdoom";
        gzdScanner();
        return 1;  // Go to mod menu
    }
    else if (choice == "2") {
        doomEngine = "zandronum";
        zandScanner();
        return 2;  // Skip to wad menu
    }
    else if (choice == "0") {
        std::cout << "Exiting..." << std::endl;
        exit(0);
    }
    else if (choice == "B") {
        return 3;  // Go to backup menu
    }
    else {
        std::cout << "Invalid selection! Try again." << std::endl;
        return 0;  // Stay on engine menu
    }
}

// Not finished
int DoomLauncher::configureMods() {
    // Visually list mods in current order again.
    for (int i = 0; i < modList.size(); i++) {
        int position = i + 1;
        std::cout << position << ") ";
        std::cout << modList[i] << std::endl;
    }
    // Prompts user to select a mod.
    // After, need to store that mod and allow the user to select another mod location to swap it with.
    // So, maybe have this command take the mod selected, store it in firstMod, then prompt again and store that mod in secondMod.
    // Then, just place them in the right place. So, I should probably also store the users number selections as an int and then use
    //  that number in the vector like vector[userNumber] or something.
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
            modList[firstNumber] = firstMod;
            modList[secondNumber] = secondMod;
            std::cout << "Swapped mod " << firstMod << " with " << secondMod << std::endl;

            // Displays modList again so the user can see their new configuration.
            for (int i = 0; i < modList.size(); i++) {
                int position = i + 1;
                std::cout << position << ") ";
                std::cout << modList[i] << std::endl;
            }

            // Prompts to either continue configuring mods or continue with current configuration.
            std::cout << "1) Swap another mod." << endl;
            std::cout << "2) Done." << endl;

            int whatToDo;
            std::cout << "What would you like to do?: ";
            std::cin >> whatToDo;

            if (whatToDo == 1) {
                configureMods();
            }
            else if (whatToDo == 2) {
                //wadVisualMenu();
            }
            else {
                std::cout << "Invalid choice!" << std::endl;
            }

        }
        else {
            // Make these either run configureMods() again or figure out how to just prompt for reselect without having to run from scratch.
            // Right now, this will just cout and end the function I think, so not in a working state.
            std::cout << "Invalid choice!" << std::endl;
        }
    }
    else {
    std::cout << "Invalid choice!" << std::endl;
    }
}

// Mod selection menu
void DoomLauncher::modVisualMenu() {
    // Creates the list of mods
    for (int i = 0; i < modList.size(); i++) {
        int position = i + 1;
        std::cout << position << ") ";
        std::cout << modList[i] << std::endl;
    }
    std::cout << std::endl;
    std::cout << "#1) Run with selected mod order";
    std::cout << "#2) Configure mod order";
    std::cout << "#B) Make a backup" << std::endl;
    std::cout << "#0) Exit" << std::endl;

    std::string choice;
    std::cout << "What do you want to do?: ";
    std::cin >> choice;

    if (choice == "1") {
        //wadMenu(); need to condense all my mod menus down to one function like this
    }
    else if (choice == "2") {
        configureMods();
    }
    else if (choice == "B") {
        backupVisualMenu();
    }
    else if (choice == "0") {
        exit(0);
    }
    else {
        std::cout << "Invalid selection! Try again." << std::endl;
        return modVisualMenu();  // Stay on mod menu
    }
    
}
/*
int DoomLauncher::modSelections() {
    std::string choice;
    std::cout << "Select a mod" << modList.size() << "): ";
    std::cin >> choice;

    for (int i = 0; i < modList.size(); i++) {
        int number = i + 1;
        if (choice == "")
        //code
        return 2;
    }
    if (choice == "1") {
        modList.clear();  // Empty for vanilla
        return 2;   // Go to wad menu
    }
    else if (choice == "B") {
        return 3;  // Go to backup menu
    }
    else if (choice == "0") {
        exit(0);
    }
    else {
        std::cout << "Invalid selection! Try again." << std::endl;
        return 1;  // Stay on mod menu
    }
}
*/
// Wad selection menu
void DoomLauncher::wadVisualMenu() {
    std::cout << " 1 ULTIMATE DOOM + SIGIL" << std::endl;
    std::cout << " 2 DOOM II: Hell on Earth" << std::endl;
    std::cout << " 3 DOOM II: Master Levels" << std::endl;
    std::cout << " 4 TNT: Evilution" << std::endl;
    std::cout << " 5 The Plutonia Experiment" << std::endl;
    std::cout << " 6 Eviternity" << std::endl;
    std::cout << " 7 Eviternity II" << std::endl;
    std::cout << " 8 Hellbound" << std::endl;
    std::cout << " 9 Ozonia" << std::endl;
    std::cout << "10 Valiant" << std::endl;
    std::cout << "11 Amalgoom" << std::endl;
    std::cout << std::endl;
    std::cout << "#B) Make a backup" << std::endl;
    std::cout << "#0) Exit" << std::endl;
}
int DoomLauncher::wadSelections() {
    std::string choice;
    std::cout << "Select an iwad (1-11): ";
    std::cin >> choice;

    if (choice == "1") {
        launchDoom("doom.wad", megawads[1], megawads[2]);
    }
    else if (choice == "2") {
        launchDoom("doom2.wad");
    }
    else if (choice == "3") {
        launchDoom("doom2.wad", megawads[0]);
    }
    else if (choice == "4") {
        launchDoom("tnt.wad");
    }
    else if (choice == "5") {
        launchDoom("plutonia.wad");
    }
    else if (choice == "6") {
        launchDoom("doom2.wad", megawads[4]);
    }
    else if (choice == "7") {
        launchDoom("doom2.wad", megawads[5]);
    }
    else if (choice == "8") {
        launchDoom("doom2.wad", megawads[6]);
    }
    else if (choice == "9") {
        launchDoom("doom2.wad", megawads[8]);
    }
    else if (choice == "10") {
        launchDoom("doom2.wad", megawads[9]);
    }
    else if (choice == "11") {
        launchDoom("doom2.wad", megawads[10], megawads[11]);
    }
    else if (choice == "B") {
        return 3;  // Go to backup menu
    }
    else if (choice == "0") {
        exit(0);
    }
    else {
        std::cout << "Invalid selection! Try again." << std::endl;
        return 2;  // Stay on wad menu
    }
}
// Backup selection menu
void DoomLauncher::backupVisualMenu() {
    std::cout << "#G) Backup gzdoom.ini" << std::endl;
    std::cout << "#Z) Backup zandronum.ini" << std::endl;
    std::cout << std::endl;
    std::cout << "#0) Exit" << std::endl;
}
int DoomLauncher::backupSelections() {
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
        return 0;
    }
    else if (choice == "Z") {
        std::cout << "Backing up GZDoom config to ~/Games/DOOM/zand/backups/zandronum.ini" << std::endl;
        std::filesystem::copy_file(
            zandBackupDir,
            doomDir + "/zand/zandronum.ini"
        );
        return 0;
    }
    else if (choice == "0") {
        std::cout << "Exiting..." << std::endl;
        exit(0);
    }
    else {
        std::cout << "Invalid selection! Try again." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return 3;  // Stay on backup menu
    }
}
