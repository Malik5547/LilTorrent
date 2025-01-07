#include <mpi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "upload.h"
#include "tracker.h"
#include "download.h"

#define TRACKER_RANK 0
#define MAX_FILES 10
#define MAX_FILENAME 15
#define HASH_SIZE 32
#define MAX_CHUNKS 100

void *download_thread_func(void *arg)
{
    PeerData *peer_data = (PeerData *)arg;

    wait_start_message(peer_data->rank, TRACKER_RANK);

    std::cout << "Rank " << peer_data->rank << " started downloading\n";

    int segments_till_update = 10;

    for (auto filename : peer_data->wanted_files)
    {
        std::cout << "Rank " << peer_data->rank << " downloading file " << filename << std::endl;

        set<int> seedsAndPeers = request_file_peers(filename, peer_data->rank, TRACKER_RANK);
        vector<string> segHashes = request_file_segHashes(filename, peer_data->rank, TRACKER_RANK);

        // Initialize the downloaded_files map
        for (unsigned long int i = 0; i < segHashes.size(); i++)
        {
            HashStatus status{i, false};

            peer_data->downloaded_files[filename][segHashes[i]] = status;
        }

        vector<int> sortedPeers = sortPeersByScore(seedsAndPeers);

        for (auto segHash : segHashes)
        {
            while (peer_data->downloaded_files[filename][segHash].status == false)
            {
                for (auto peer : sortedPeers)
                {
                    if (peer == peer_data->rank)
                    {
                        continue;
                    }

                    if (request_file_seg(peer, filename, segHash, peer_data->rank))
                    {
                        peer_data->downloaded_files[filename][segHash].status = true;

                        if (segments_till_update-- == 0)
                        {
                            seedsAndPeers = request_file_peers(filename, peer_data->rank, TRACKER_RANK);
                            sortedPeers = sortPeersByScore(seedsAndPeers);

                            segments_till_update = 10;
                        } 

                        break;
                    }
                }
            }
        }

        if (DEBUG)
        {
            std::cout << "Rank " << peer_data->rank << " peers for file " << filename << ": ";
            for (auto peer : seedsAndPeers)
            {
                std::cout << peer << " ";
            }

            std::cout << std::endl;
        }

        send_file_download_end_message(peer_data->rank, TRACKER_RANK);

        if (SAVE_RESULTS)
        {
            save_file(peer_data->downloaded_files, filename, peer_data->rank);
        }
    }

    send_client_download_end_message(peer_data->rank, TRACKER_RANK);

    return NULL;
}

void *upload_thread_func(void *arg)
{
    PeerData *peer_data = (PeerData *)arg;

    int rank = peer_data->rank;

    if (DEBUG)
    {
        std::cout << "Rank " << rank << " has " << peer_data->wanted_files.size() << " wanted files\n";
    }

    send_held_files_data(peer_data->held_files, rank, TRACKER_RANK);

    while (true)
    {
        MPI_Status status;
        vector<char> buffer;
        int buffer_size;

        int peerLoad = 0;
        int totalRequests = 0;
        int successfulRequests = 0;

        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_SOURCE, MPI_COMM_WORLD, &status);

        if ((status.MPI_TAG & UPLOAD_TAGS) != 0)
        {
            MPI_Recv(&buffer_size, 1, MPI_INT, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            peerLoad++;
        }
        else
        {
            continue;
        }

        switch (status.MPI_TAG)
        {
        case GET_FILE_SEG_TAG:
        {
            totalRequests++;

            buffer.resize(buffer_size);
            MPI_Recv(buffer.data(), buffer.size(), MPI_CHAR, status.MPI_SOURCE, FILENAME_TAG, MPI_COMM_WORLD, &status);

            string filename(buffer.begin(), buffer.end());

            // Remove the NULL terminator
            if (!filename.empty() && filename[filename.size() - 1] == '\0')
            {
                filename.pop_back();
            }

            buffer.resize(HASH_LENGTH);

            MPI_Recv(buffer.data(), HASH_LENGTH, MPI_CHAR, status.MPI_SOURCE, HASH_TAG, MPI_COMM_WORLD, &status);

            string segHash(buffer.begin(), buffer.end());

            // Remove the NULL terminator
            if (!segHash.empty() && segHash[segHash.size() - 1] == '\0')
            {
                segHash.pop_back();
            }

            string response;

            if (has_file_seg(peer_data, filename, segHash))
            {
                response = SUCCESS_MESSAGE;
                successfulRequests++;
            }
            else
            {
                response = MISSING_MESSAGE;
            }

            MPI_Send(response.c_str(), response.size(), MPI_CHAR, status.MPI_SOURCE, RET_FILE_SEG_TAG, MPI_COMM_WORLD);

            break;
        }
        case GET_PEER_SCORE_TAG:
        {
            double peerReliability = (double)successfulRequests / totalRequests;
            double score = 1.0 / (peerLoad + 1) + 0.5 * peerReliability;

            MPI_Send(&score, 1, MPI_DOUBLE, status.MPI_SOURCE, RET_PEER_SCORE_TAG, MPI_COMM_WORLD);

            break;
        }
        case END_UPLOAD_TAG:
            std::cout << "Rank " << peer_data->rank << " finished uploading\n";
            return NULL;
        }
    }

    return NULL;
}

void tracker(int numtasks, int rank)
{
    Swarm swarm;
    std::vector<int> finished_clients;

    get_swarm_data(swarm, numtasks - 1);

    for (int i = 1; i < numtasks; ++i)
    {
        send_start_message(i, rank);
    }

    while ((int)finished_clients.size() < numtasks - 1)
    {
        MPI_Status status;
        vector<char> buffer;
        int buffer_size;

        MPI_Recv(&buffer_size, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        switch (status.MPI_TAG)
        {
        case GET_FILE_PEERS_TAG:
        {
            buffer.resize(buffer_size);
            MPI_Recv(buffer.data(), buffer.size(), MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);

            string filename(buffer.begin(), buffer.end());

            // Remove the NULL terminator
            if (!filename.empty() && filename[filename.size() - 1] == '\0')
            {
                filename.pop_back();
            }

            send_file_peers(swarm, filename, status.MPI_SOURCE);

            swarm.files[filename].seedsAndPeers.insert(status.MPI_SOURCE);
            break;
        }
        case GET_FILE_SEGHASHES_TAG:
        {
            buffer.resize(buffer_size);

            MPI_Recv(buffer.data(), buffer.size(), MPI_CHAR, status.MPI_SOURCE, status.MPI_TAG, MPI_COMM_WORLD, &status);

            string filename(buffer.begin(), buffer.end());

            // Remove the NULL terminator
            if (!filename.empty() && filename[filename.size() - 1] == '\0')
            {
                filename.pop_back();
            }

            send_file_segHashes(swarm, filename, status.MPI_SOURCE);

            break;
        }
        case FILE_DOWNLOAD_END_TAG:
            // Client becomes seed, but it's already in peer list
            break;
        case CLIENT_DOWNLOAD_END_TAG:
            finished_clients.push_back(status.MPI_SOURCE);
            break;
        default:
            break;
        }
    }

    for (int i = 1; i < numtasks; ++i)
    {
        send_end_upload_message(i, rank);
    }
}

void peer(int numtasks, int rank)
{
    pthread_t download_thread;
    pthread_t upload_thread;
    void *status;
    int r;

    PeerData peer_data;

    peer_data.rank = rank;

    get_held_and_wanted_files(peer_data.held_files, peer_data.wanted_files, rank);

    r = pthread_create(&download_thread, NULL, download_thread_func, (void *)&peer_data);
    if (r)
    {
        printf("Eroare la crearea thread-ului de download\n");
        exit(-1);
    }

    r = pthread_create(&upload_thread, NULL, upload_thread_func, (void *)&peer_data);
    if (r)
    {
        printf("Eroare la crearea thread-ului de upload\n");
        exit(-1);
    }

    r = pthread_join(download_thread, &status);
    if (r)
    {
        printf("Eroare la asteptarea thread-ului de download\n");
        exit(-1);
    }

    r = pthread_join(upload_thread, &status);
    if (r)
    {
        printf("Eroare la asteptarea thread-ului de upload\n");
        exit(-1);
    }
}

int main(int argc, char *argv[])
{
    int numtasks, rank;

    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE)
    {
        fprintf(stderr, "MPI nu are suport pentru multi-threading\n");
        exit(-1);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == TRACKER_RANK)
    {
        tracker(numtasks, rank);
    }
    else
    {
        peer(numtasks, rank);
    }

    MPI_Finalize();
}
