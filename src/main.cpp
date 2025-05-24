#include <iostream>
#include "Game.h"
#include <filesystem>


int main() {
    namespace fs = std::filesystem;
    std::cout << "Current path: " << fs::current_path() << std::endl;
    fs::current_path(fs::current_path().parent_path());
    std::cout << "New current path: " << fs::current_path() << std::endl;
    Game(1280, 900, "RayStrike");
    return 0;
}

