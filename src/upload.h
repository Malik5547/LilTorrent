#pragma once

#include <mpi.h>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "constants.h"

struct HeldFile {
    std::string filename;
    std::vector<std::string> segHashes;
};

struct HashStatus {
    unsigned long int index;
    bool status;
};

struct PeerData {
    int rank;
    std::vector<HeldFile> held_files;
    std::vector<std::string> wanted_files;
    std::map<std::string, std::map<std::string, HashStatus>> downloaded_files;
};

void get_held_and_wanted_files(std::vector<HeldFile>& held_files, std::vector<std::string>& wanted_files, int rank);

void send_held_files_data(const std::vector<HeldFile>& held_files, int rank, int tracker_rank);

void wait_start_message(int rank, int tracker_rank);

void serialize_held_file(const HeldFile& held_file, std::vector<char>& buffer);
void deserialize_held_file(HeldFile& held_file, const std::vector<char>& buffer);

bool has_file_seg(const PeerData* peer_data, const std::string& filename, const std::string& segHash);

void print_held_files(const std::vector<HeldFile>& held_files);