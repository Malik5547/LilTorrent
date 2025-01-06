#pragma once

#include <mpi.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#define DEBUG 1


#define MAX_FILENAME 15
#define MAX_BUFFER_SIZE 1024

struct HeldFile {
    std::string filename;
    std::vector<std::string> segHashes;
};

void get_held_and_wanted_files(std::vector<HeldFile>& held_files, std::vector<std::string>& wanted_files, int rank);

void send_held_files_data(const std::vector<HeldFile>& held_files, int rank, int tracker_rank);

void wait_start_message(int rank, int tracker_rank);

void serialize_held_file(const HeldFile& held_file, std::vector<char>& buffer);
void deserialize_held_file(HeldFile& held_file, const std::vector<char>& buffer);

void print_held_files(const std::vector<HeldFile>& held_files);