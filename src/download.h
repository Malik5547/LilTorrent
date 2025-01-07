#pragma once

#include <mpi.h>
#include <vector>
#include <set>
#include <algorithm>

#include "upload.h"
#include "constants.h"


using namespace std;

set<int> request_file_peers(const std::string& filename, int rank, int tracker_rank);
vector<string> request_file_segHashes(const std::string& filename, int rank, int tracker_rank);

bool request_file_seg(int peer_rank, const std::string& filename, const std::string& segHash, int rank);

void send_file_download_end_message(int rank, int tracker_rank);
void send_client_download_end_message(int rank, int tracker_rank);

void save_file(const std::map<std::string, std::map<std::string, HashStatus>>& downloaded_files, const std::string& filename, int rank);