#include <mpi.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "upload.h"
#include "tracker.h"

#define TRACKER_RANK 0
#define MAX_FILES 10
#define MAX_FILENAME 15
#define HASH_SIZE 32
#define MAX_CHUNKS 100

void *download_thread_func(void *arg)
{
    // int rank = *(int*) arg;

    return NULL;
}

void *upload_thread_func(void *arg)
{
    int rank = *(int*) arg;

    std::vector<HeldFile> held_files;
    std::vector<std::string> wanted_files;
    
    get_held_and_wanted_files(held_files, wanted_files, rank);
    // print_held_files(held_files);

    if (DEBUG){
        std::cout << "Rank " << rank << " has " << wanted_files.size() << " wanted files\n";
    }

    send_held_files_data(held_files, rank, TRACKER_RANK);

    wait_start_message(rank, TRACKER_RANK);

    std::cout << "Rank " << rank << " finished uploading\n";

    return NULL;
}

void tracker(int numtasks, int rank) {
    Swarm swarm;
    std::vector<int> finished_clients;

    get_swarm_data(swarm, numtasks - 1);

    for (int i = 1; i < numtasks; ++i) {
        send_start_message(i, rank);
    }
}

void peer(int numtasks, int rank) {
    pthread_t download_thread;
    pthread_t upload_thread;
    void *status;
    int r;

    r = pthread_create(&download_thread, NULL, download_thread_func, (void *) &rank);
    if (r) {
        printf("Eroare la crearea thread-ului de download\n");
        exit(-1);
    }

    r = pthread_create(&upload_thread, NULL, upload_thread_func, (void *) &rank);
    if (r) {
        printf("Eroare la crearea thread-ului de upload\n");
        exit(-1);
    }

    r = pthread_join(download_thread, &status);
    if (r) {
        printf("Eroare la asteptarea thread-ului de download\n");
        exit(-1);
    }

    r = pthread_join(upload_thread, &status);
    if (r) {
        printf("Eroare la asteptarea thread-ului de upload\n");
        exit(-1);
    }
}
 
int main (int argc, char *argv[]) {
    int numtasks, rank;
 
    int provided;
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (provided < MPI_THREAD_MULTIPLE) {
        fprintf(stderr, "MPI nu are suport pentru multi-threading\n");
        exit(-1);
    }
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == TRACKER_RANK) {
        tracker(numtasks, rank);
    } else {
        peer(numtasks, rank);
    }

    MPI_Finalize();
}
