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

    for (auto filename : peer_data->wanted_files)
    {
        std::cout << "Rank " << peer_data->rank << " downloading file " << filename << std::endl;

        set<int> seedsAndPeers = request_file_peers(filename, peer_data->rank, TRACKER_RANK);
        vector<string> segHashes = request_file_segHashes(filename, peer_data->rank, TRACKER_RANK);


        if (DEBUG)
        {
            std::cout << "Rank " << peer_data->rank << " peers for file " << filename << ": ";
            for (auto peer : seedsAndPeers)
            {
                std::cout << peer << " ";
            }

            std::cout << std::endl;
        }
    }

    send_download_end_message(peer_data->rank, TRACKER_RANK);

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

    std::cout << "Rank " << peer_data->rank << " finished uploading\n";

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
        case DOWNLOAD_END_TAG:
            finished_clients.push_back(status.MPI_SOURCE);
            break;
        default:
            break;
        }
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
