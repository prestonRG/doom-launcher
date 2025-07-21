#include "launcher.h"
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

    vanillaPlus = {
        doomDir + "/gzd/mods/vanilla_plus/nashgore.pk3",
        doomDir + "/gzd/mods/vanilla_plus/Beautiful_Doom_716.pk3",
        doomDir + "/gzd/mods/vanilla_plus/flashlight_plus_plus_v9_1.pk3",
        doomDir + "/gzd/mods/music/DOOM Metal X IDKFA Soundtrack.pk3",
        doomDir + "/gzd/mods/HUDs/Cats_CRT_HUD v1.0.1.pk3"
    };

    brutalMods = {
        doomDir + "/gzd/mods/bd22Test5RC2.pk3",
        doomDir + "/gzd/mods/music/DOOM Metal X IDKFA Soundtrack.pk3",
        doomDir + "/gzd/mods/HDTextures.pk3",
        doomDir + "/gzd/mods/mymod.wad",
        doomDir + "/gzd/mods/HUDs/Cats_CRT_HUD v1.0.1.pk3"
    };

    brutalPlatinum = {
        doomDir + "/gzd/mods/BrutalDoomPlatinumv3.1.1.pk3",
        doomDir + "/gzd/mods/music/DOOM Metal X IDKFA Soundtrack.pk3",
        doomDir + "/gzd/mods/mymod.wad",
        doomDir + "/gzd/mods/HUDs/Cats_CRT_HUD v1.0.1.pk3"
    };

    projectBrutality = {
        doomDir + "/gzd/mods/PB-0_3_1-alpha.pk3",
        doomDir + "/gzd/mods/music/DOOM Metal X IDKFA Soundtrack.pk3"
    };

    brutalDoom64 = {
        doomDir + "/gzd/mods/Brutal_Doom_64_v2.5_u30.12/bd64game_v2.5.pk3",
        doomDir + "/gzd/mods/bd64maps_v2.5.pk3",
        doomDir + "/gzd/mods/music/DOOM Metal X IDKFA Soundtrack.pk3"
    };

    zandronum = {
        doomDir + "/zand/mods/bd22Test5RC2.pk3",
        doomDir + "/zand/mods/music/DOOM Metal X IDKFA Soundtrack.pk3",
        doomDir + "/zand/mods/mymod.wad"
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
    menuState = 0;

    std::system("clear");

    while (true) {
        std::system("clear");

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

    if (choice == "1") {
        doomEngine = "gzdoom";
        return 1;  // Go to mod menu
    }
    else if (choice == "2") {
        doomEngine = "zandronum";
        // Set zandronum mods here
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
// Mod selection menu
void DoomLauncher::modVisualMenu() {
    std::cout << "1 VANILLA" << std::endl;
    std::cout << "2 VANILLA PLUS" << std::endl;
    std::cout << "3 BRUTAL DOOM" << std::endl;
    std::cout << "4 BRUTAL DOOM 64" << std::endl;
    std::cout << "5 BRUTAL DOOM PLATINUM" << std::endl;
    std::cout << "6 PROJECT BRUTALITY" << std::endl;
    std::cout << "7 VOXEL DOOM" << std::endl;
    std::cout << std::endl;
    std::cout << "#B) Make a backup" << std::endl;
    std::cout << "#0) Exit" << std::endl;
}
int DoomLauncher::modSelections() {
    std::string choice;
    std::cout << "Select a modpack to use (1-7): ";
    std::cin >> choice;

    if (choice == "1") {
        modList.clear();  // Empty for vanilla
        return 2;   // Go to wad menu
    }
    else if (choice == "2") {
        modList = vanillaPlus;
        return 2;
    }
    else if (choice == "3") {
        modList = brutalMods;
        return 2;
    }
    else if (choice == "4") {
        modList = brutalPlatinum;
        return 2;
    }
    else if (choice == "5") {
        modList = projectBrutality;
        return 2;
    }
    else if (choice == "6") {
        modList = brutalDoom64;
        return 2;
    }
    else if (choice == "7") {
        modList = zandronum;
        return 2;
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
