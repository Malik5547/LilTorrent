#include "upload.h"

using namespace std;

void get_held_and_wanted_files(std::vector<HeldFile>& held_files, vector<string>& wanted_files, int rank){
    // Create filename string "in" + rank + ".txt"
    std::ostringstream filename_stream;

    if (DEBUG) {
        filename_stream << "../checker/tests/test1/";
    }

    filename_stream << "in" << rank << ".txt";

    std::string filename = filename_stream.str();

    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Error opening file " + filename);
    }

    // Read held files
    int num_files;
    file >> num_files;

    for (int i = 0; i < num_files; ++i) {
        HeldFile held_file;
        file >> held_file.filename;

        int num_chunks;
        file >> num_chunks;

        for (int j = 0; j < num_chunks; ++j) {
            std::string hash;
            file >> hash;
            held_file.segHashes.push_back(hash);
        }

        held_files.push_back(held_file);
    }

    // Read wanted files
    int num_wanted_files;
    file >> num_wanted_files;

    for (int i = 0; i < num_wanted_files; ++i) {
        std::string wanted_file;
        file >> wanted_file;
        wanted_files.push_back(wanted_file);
    }

    file.close();
}

void send_held_files_data(const std::vector<HeldFile>& held_files, int rank, int tracker_rank){
    for(const auto& held_file : held_files){
        std::vector<char> buffer;
        
        serialize_held_file(held_file, buffer);

        int buffer_size = buffer.size();

        cout << "Buffer size: " << buffer.size() << endl;

        MPI_Send(&buffer_size, 1, MPI_INT, tracker_rank, 0, MPI_COMM_WORLD);

        MPI_Send(buffer.data(), buffer.size(), MPI_CHAR, tracker_rank, 0, MPI_COMM_WORLD);
    }

    int buffer_size = 0;
    MPI_Send(&buffer_size, 1, MPI_INT, tracker_rank, 0, MPI_COMM_WORLD);
}


void serialize_held_file(const HeldFile& held_file, std::vector<char>& buffer){
    std::ostringstream oss;
    oss << held_file.filename << " " << held_file.segHashes.size() << " ";
    for(const auto& hash : held_file.segHashes){
        oss << hash << " ";
    }

    std::string str = oss.str();
    buffer.resize(str.size());
    std::copy(str.begin(), str.end(), buffer.begin());
}

void deserialize_held_file(HeldFile& held_file, const std::vector<char>& buffer){
    std::istringstream iss(std::string(buffer.begin(), buffer.end()));
    iss >> held_file.filename;

    int num_chunks;
    iss >> num_chunks;

    for(int i = 0; i < num_chunks; ++i){
        std::string hash;
        iss >> hash;
        held_file.segHashes.push_back(hash);
    }
}

void wait_start_message(int rank, int tracker_rank){
    MPI_Status status;
    char data[MAX_BUFFER_SIZE];

    MPI_Recv(data, MAX_BUFFER_SIZE, MPI_CHAR, tracker_rank, START_DOWNLOAD_TAG, MPI_COMM_WORLD, &status);
}

void print_held_files(const std::vector<HeldFile>& held_files) {
    for (const auto& held_file : held_files) {
        std::cout << "Filename: " << held_file.filename << "\n";
        std::cout << "Chunks: " << std::endl;
        for (const auto& hash : held_file.segHashes) {
            std::cout << hash << std::endl;
        }
        std::cout << "\n";
    }
}