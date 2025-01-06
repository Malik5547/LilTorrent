#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#define MAX_FILENAME 15

struct HeldFile {
    std::string filename;
    std::vector<std::string> segHashes;
};

std::vector<HeldFile> get_held_files(int rank);

void print_held_files(const std::vector<HeldFile>& held_files);