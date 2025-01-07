#pragma once

#include <mpi.h>
#include <vector>
#include <set>  

#include "upload.h"
#include "constants.h"


using namespace std;

struct FileData
{
    set<int> seedsAndPeers;
    vector<string> segHashes;
};

struct Swarm
{
    map<string, FileData> files;
};



void get_swarm_data(Swarm& swarm, int numtasks);

void send_start_message(int rank, int tracker_rank);
void send_file_peers(const Swarm& swarm, const string& filename, int rank);
void send_file_segHashes(const Swarm& swarm, const string& filename, int rank);
void send_end_upload_message(int rank, int tracker_rank);