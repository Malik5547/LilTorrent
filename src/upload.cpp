#include "upload.h"

#define DEBUG 1

std::vector<HeldFile> get_held_files(int rank) {
    std::vector<HeldFile> held_files;

    // Create filename string "in" + rank + ".txt"
    std::ostringstream filename_stream;

    if (DEBUG) {
        filename_stream << "../checker/tests/test1/";
    }

    filename_stream << "in" << rank << ".txt";

    std::string filename = filename_stream.str();

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file " + filename);
    }

    int num_files;
    file >> num_files;

    for (int i = 0; i < num_files; ++i) {
        HeldFile held_file;
        file >> held_file.filename;

        int num_chunks;
        file >> num_chunks;

        for (int j = 0; j < num_chunks; ++j) {
            std::string hash;
            file >> hash;
            held_file.segHashes.push_back(hash);
        }

        held_files.push_back(held_file);
    }

    return held_files;
}

void print_held_files(const std::vector<HeldFile>& held_files) {
    for (const auto& held_file : held_files) {
        std::cout << "Filename: " << held_file.filename << "\n";
        std::cout << "Chunks: " << std::endl;
        for (const auto& hash : held_file.segHashes) {
            std::cout << hash << std::endl;
        }
        std::cout << "\n";
    }
}