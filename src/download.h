#pragma once

#include <mpi.h>
#include <vector>
#include <set>

#include "tags.h"

using namespace std;

set<int> request_file_peers(const std::string& filename, int rank, int tracker_rank);

void send_download_end_message(int rank, int tracker_rank);