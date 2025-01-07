#include "download.h"

set<int> request_file_peers(const std::string &filename, int rank, int tracker_rank)
{
    MPI_Status status;
    int peer_count;
    vector<int> peers;

    int filename_size = filename.size();

    MPI_Send(&filename_size, 1, MPI_INT, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD);
    MPI_Send((void *)filename.c_str(), filename_size, MPI_CHAR, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD);

    MPI_Recv(&peer_count, 1, MPI_INT, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD, &status);
    peers.resize(peer_count);

    MPI_Recv(peers.data(), peer_count, MPI_INT, tracker_rank, GET_FILE_PEERS_TAG, MPI_COMM_WORLD, &status);

    return set<int>(peers.begin(), peers.end());
}

vector<string> request_file_segHashes(const std::string &filename, int rank, int tracker_rank)
{
    MPI_Status status;
    int segHash_count;
    vector<string> segHashes;
    string buffer;

    int filename_size = filename.size();

    MPI_Send(&filename_size, 1, MPI_INT, tracker_rank, GET_FILE_SEGHASHES_TAG, MPI_COMM_WORLD);
    MPI_Send((void *)filename.c_str(), filename_size, MPI_CHAR, tracker_rank, GET_FILE_SEGHASHES_TAG, MPI_COMM_WORLD);

    MPI_Recv(&segHash_count, 1, MPI_INT, tracker_rank, GET_FILE_SEGHASHES_TAG, MPI_COMM_WORLD, &status);
    segHashes.resize(segHash_count);

    buffer.resize(segHash_count * HASH_LENGTH);

    MPI_Recv((void *)buffer.data(), segHash_count * HASH_LENGTH, MPI_CHAR, tracker_rank, GET_FILE_SEGHASHES_TAG, MPI_COMM_WORLD, &status);

    for (int i = 0; i < segHash_count; i++)
    {
        segHashes[i] = buffer.substr(i * HASH_LENGTH, HASH_LENGTH);
    }

    if (DEBUG)
    {
        cout << "Received " << segHash_count << " hashes for file " << filename << endl;
        cout << "First hash: " << segHashes[0] << endl;
        cout << "Last hash: " << segHashes[segHash_count - 1] << endl;
    }

    return segHashes;
}

bool request_file_seg(int peer_rank, const std::string &filename, const std::string &segHash, int rank)
{
    MPI_Status status;
    vector<char> buffer;

    int filename_size = filename.size();
    int segHash_size = HASH_LENGTH;

    MPI_Send(&filename_size, 1, MPI_INT, peer_rank, GET_FILE_SEG_TAG, MPI_COMM_WORLD);
    MPI_Send((void *)filename.c_str(), filename_size, MPI_CHAR, peer_rank, FILENAME_TAG, MPI_COMM_WORLD);

    MPI_Send((void *)segHash.c_str(), segHash_size, MPI_CHAR, peer_rank, HASH_TAG, MPI_COMM_WORLD);

    buffer.resize(MAX_BUFFER_SIZE);

    MPI_Recv(buffer.data(), MAX_BUFFER_SIZE, MPI_CHAR, peer_rank, RET_FILE_SEG_TAG, MPI_COMM_WORLD, &status);

    // Resize buffer to actual size
    buffer.resize(status._ucount);

    string response(buffer.begin(), buffer.end());

    if (response == SUCCESS_MESSAGE)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void send_file_download_end_message(int rank, int tracker_rank)
{
    MPI_Send(NULL, 0, MPI_INT, tracker_rank, FILE_DOWNLOAD_END_TAG, MPI_COMM_WORLD);
}

void send_client_download_end_message(int rank, int tracker_rank)
{
    MPI_Send(NULL, 0, MPI_INT, tracker_rank, CLIENT_DOWNLOAD_END_TAG, MPI_COMM_WORLD);
}

void save_file(const map<string, map<string, HashStatus>> &downloaded_files, const string &filename, int rank)
{
    ofstream file;
    string file_path = "client" + to_string(rank) + "_" + filename;

    file.open(file_path, ios::binary);

    if (!file.is_open())
    {
        throw runtime_error("Could not open file for writing");
    }

    // Sort the map by index
    vector<pair<string, HashStatus>> sorted_downloaded_files(downloaded_files.at(filename).begin(), downloaded_files.at(filename).end());

    sort(sorted_downloaded_files.begin(), sorted_downloaded_files.end(), [](const pair<string, HashStatus> &a, const pair<string, HashStatus> &b) {
        return a.second.index < b.second.index;
    });

    for (const auto &seg : sorted_downloaded_files)
    {
        file << seg.first << endl;
    }  

    file.close();
}