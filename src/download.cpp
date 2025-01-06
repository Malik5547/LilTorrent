#include "download.h"

set<int> request_file_peers(const std::string& filename, int rank, int tracker_rank){
    MPI_Status status;
    int peer_count;
    vector<int> peers;

    int filename_size = filename.size();

    MPI_Send(&filename_size, 1, MPI_INT, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD);
    MPI_Send((void*) filename.c_str(), filename_size, MPI_CHAR, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD);
   
    MPI_Recv(&peer_count, 1, MPI_INT, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD, &status);
    peers.resize(peer_count);

    MPI_Recv(peers.data(), peer_count, MPI_INT, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD, &status);

    return set<int>(peers.begin(), peers.end());
}

vector<string> request_file_segHashes(const std::string& filename, int rank, int tracker_rank){
    MPI_Status status;
    int segHash_count;
    vector<string> segHashes;
    string buffer;

    int filename_size = filename.size();

    MPI_Send(&filename_size, 1, MPI_INT, tracker_rank, GET_FILE_SEGHASHES_TAG, MPI_COMM_WORLD);
    MPI_Send((void*) filename.c_str(), filename_size, MPI_CHAR, tracker_rank, GET_FILE_SEGHASHES_TAG, MPI_COMM_WORLD);

    MPI_Recv(&segHash_count, 1, MPI_INT, tracker_rank, GET_FILE_SEGHASHES_TAG, MPI_COMM_WORLD, &status);
    segHashes.resize(segHash_count);

    buffer.resize(segHash_count * HASH_LENGTH);

    MPI_Recv((void*) buffer.data(), segHash_count * HASH_LENGTH, MPI_CHAR, tracker_rank, GET_FILE_SEGHASHES_TAG, MPI_COMM_WORLD, &status);

    for(int i = 0; i < segHash_count; i++){
        segHashes[i] = buffer.substr(i * HASH_LENGTH, HASH_LENGTH);
    }

    if (DEBUG){
        cout << "Received " << segHash_count << " hashes for file " << filename << endl;
        cout << "First hash: " << segHashes[0] << endl;
        cout << "Last hash: " << segHashes[segHash_count - 1] << endl;
    }

    return segHashes;
}

void send_download_end_message(int rank, int tracker_rank){
    MPI_Send(NULL, 0, MPI_INT, tracker_rank, DOWNLOAD_END_TAG, MPI_COMM_WORLD);
}