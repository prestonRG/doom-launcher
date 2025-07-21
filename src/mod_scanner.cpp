#include "mod_scanner.h"
#include <filesystem>
#include <iostream>
using namespace std;

// Finds all of the doom mods in the proper directories.
vector<string> findMods(string modPath) {
    vector<string> mods;
    cout << "Looking in: " << modPath << endl;

    // Looks at all of the files in the specified directory and returns relevant ones.
    for (auto& file : filesystem::directory_iterator(modPath)) {
        string filename = file.path().filename();
        string extension = file.path().extension();
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
