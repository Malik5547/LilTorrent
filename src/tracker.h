#pragma once

#include <mpi.h>
#include <vector>

#include "upload.h"


using namespace std;

struct FileData
{
    vector<int> seedsAndPeers;
    vector<string> segHashes;
};

struct Swarm
{
    map<string, FileData> files;
};



void get_swarm_data(Swarm& swarm, int numtasks);

void send_start_message(int rank, int tracker_rank);