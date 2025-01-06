#include "tracker.h"

void get_swarm_data(Swarm &swarm, int numclients)
{
    int finished_clients = 0;

    while (true)
    {
        MPI_Status status;

        int buffer_size;

        MPI_Recv(&buffer_size, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

        if (buffer_size == 0)
        {
            finished_clients++;
            if (finished_clients == numclients)
            {
                break;
            }

            continue;
        }

        std::vector<char> buffer(buffer_size);

        MPI_Recv(buffer.data(), buffer.size(), MPI_CHAR, status.MPI_SOURCE, 0, MPI_COMM_WORLD, &status);

        HeldFile held_file;
        deserialize_held_file(held_file, buffer);

        if (DEBUG && held_file.filename == "file3")
        {
            std::cout << "Received file3 from " << status.MPI_SOURCE << std::endl;
        }

        swarm.files[held_file.filename].seedsAndPeers.push_back(status.MPI_SOURCE);

        if (swarm.files[held_file.filename].segHashes.empty())
        {
            swarm.files[held_file.filename].segHashes = held_file.segHashes;
        }
    }

    if (DEBUG)
    {
        for (const auto &file : swarm.files)
        {
            std::cout << "Filename: " << file.first << std::endl;
            std::cout << "Seeds and Peers: ";
            for (const auto &seedPeer : file.second.seedsAndPeers)
            {
                std::cout << seedPeer << " ";
            }
            std::cout << std::endl;
            std::cout << "SegHashes: ";
            for (const auto &hash : file.second.segHashes)
            {
                std::cout << hash << " ";
            }
            std::cout << std::endl;
        }
    }
}

void send_start_message(int rank, int tracker_rank)
{
    char data[MAX_BUFFER_SIZE] = "START";

    MPI_Send(data, strlen(data) + 1, MPI_CHAR, rank, START_DOWNLOAD_TAG, MPI_COMM_WORLD);
}