#pragma once
#include <vector>
#include <string>

std::vector<std::string> findMods(std::string modPath);
std::vector<std::string> findMegawads(std::string megawadPath);

// Colors
const std::string RED = "\033[31m";
const std::string GREEN = "\033[32m";
const std::string YELLOW = "\033[33m";
const std::string BLUE = "\033[34m";
const std::string MAGENTA = "\033[35m";
const std::string CYAN = "\033[36m";
const std::string RESET = "\033[0m";
