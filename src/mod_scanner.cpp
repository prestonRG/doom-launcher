#include "mod_scanner.h"
#include <filesystem>
#include <iostream>

// Finds all of the doom mods in the proper directories.
std::vector<std::string> findMods(std::string modPath) {
    std::vector<std::string> mods;
    std::cout << "Looking in: " << modPath << std::endl;

    // Looks at all of the files in the specified directory and returns relevant ones.
    for (auto& file : std::filesystem::directory_iterator(modPath)) {
        std::string filename = file.path().filename();
        std::string extension = file.path().extension();
        std::cout << "Found: " << filename << std::endl;

        // Checks for any .zip files and, if there are any, unzips them before proceeding (currently only warns).
        if (extension == ".pk3" || extension == ".wad" || extension == ".zip") {
            if (extension == ".zip") {
                // TODO
                std::cout << "Warning: Found .zip file: " << filename << " - mod will likely not work properly." << std::endl;
            }
            mods.push_back(file.path().string());
        }
    }

    return mods;
}

std::vector<std::string> findMegawads(std::string megawadPath) {
    std::vector<std::string> megawads;
    std::cout << "Looking in: " << megawadPath << std::endl;

    // Looks at all of the files in the specified directory and returns relevant ones.
    for (auto& file : std::filesystem::directory_iterator(megawadPath)) {
        std::string filename = file.path().filename();
        std::string extension = file.path().extension();
        std::cout << "Found: " << filename << std::endl;

        // Checks for any .wad files in the specified path.
        if (extension == ".wad") {
            megawads.push_back(file.path().string());
        }
        else {
            std::cout << "Couldn't find anything!" << std::endl;
        }
    }

    return megawads;
}
