#include "download.h"

set<int> request_file_peers(const std::string& filename, int rank, int tracker_rank){
    MPI_Status status;
    int peer_count;
    vector<int> peers;

    int filename_size = filename.size() + 1; // +1 for the NULL TE

    MPI_Send(&filename_size, 1, MPI_INT, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD);
    MPI_Send((void*) filename.c_str(), filename.size() + 1, MPI_CHAR, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD);
   
    MPI_Recv(&peer_count, 1, MPI_INT, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD, &status);
    peers.resize(peer_count);

    MPI_Recv(peers.data(), peer_count, MPI_INT, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD, &status);

    return set<int>(peers.begin(), peers.end());
}

void send_download_end_message(int rank, int tracker_rank){
    MPI_Send(NULL, 0, MPI_INT, tracker_rank, DOWNLOAD_END_TAG, MPI_COMM_WORLD);
}