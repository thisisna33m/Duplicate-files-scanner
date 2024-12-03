#include "FileScanner.h"
#include <iostream>
#include <fstream>
#include <openssl/evp.h>
#include <filesystem>

namespace fs = std::filesystem;

std::mutex queueMutex; // Mutex for thread-safe file queue access

// Calculate SHA-256 hash for a file
std::string FileScanner::calculateSHA256(const std::string& filePath) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hashLength = 0;
    char buffer[8192]; // Buffer size for file reading
    std::ifstream file(filePath, std::ios::binary);
    if (!file) return "";

    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx || EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    while (file.read(buffer, sizeof(buffer))) {
        EVP_DigestUpdate(ctx, buffer, file.gcount());
    }
    EVP_DigestFinal_ex(ctx, hash, &hashLength);
    EVP_MD_CTX_free(ctx);

    std::string hashString;
    for (unsigned int i = 0; i < hashLength; ++i) {
        char hex[3];
        snprintf(hex, sizeof(hex), "%02x", hash[i]);
        hashString += hex;
    }
    return hashString;
}

// Worker thread for processing files from the queue
void FileScanner::workerThread() {
    while (true) {
        std::string filePath;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [this] { return !fileQueue.empty(); });
            filePath = fileQueue.front();
            fileQueue.pop();
        }

        if (!fs::is_regular_file(filePath)) continue;

        // Gather file metadata
        std::string fileName = fs::path(filePath).filename().string();
        size_t fileSize = fs::file_size(filePath);
        std::string fileHash = calculateSHA256(filePath);

        // Store file information
        {
            std::lock_guard<std::mutex> lock(fileMutex);
            files.emplace_back(fileName, filePath, fileSize, fileHash);
        }
    }
}

// Recursive directory scanning with multithreading
void FileScanner::scanDirectory(const std::string& path, int threadCount, bool calculateHash) {
    for (const auto& entry : fs::recursive_directory_iterator(path, fs::directory_options::skip_permission_denied)) {
        if (fs::is_regular_file(entry)) {
            std::unique_lock<std::mutex> lock(queueMutex);
            fileQueue.push(entry.path().string());
            queueCondition.notify_one();
        }
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back([this, calculateHash]() {
            while (true) {
                std::string filePath;
                {
                    std::unique_lock<std::mutex> lock(queueMutex);
                    if (fileQueue.empty()) break;
                    filePath = fileQueue.front();
                    fileQueue.pop();
                }

                if (!fs::is_regular_file(filePath)) continue;

                std::string fileName = fs::path(filePath).filename().string();
                size_t fileSize = fs::file_size(filePath);
                std::string fileHash = calculateHash ? calculateSHA256(filePath) : "";

                std::lock_guard<std::mutex> lock(fileMutex);
                files.emplace_back(fileName, filePath, fileSize, fileHash);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

// Get all scanned file information
const std::vector<FileInfo>& FileScanner::getFiles() const {
    return files;
}

// Scan a specific drive
void FileScanner::scanDrive(const std::string& drive) {
    scanDirectory(drive, std::thread::hardware_concurrency(), false);
}

// Disk scanning for Linux and Windows
void FileScanner::scanDisk() {
#ifdef _WIN32
    char drive = 'A';
    for (drive = 'A'; drive <= 'Z'; drive++) {
        char drivePath[4] = {drive, ':', '\\', '\0'};
        if (GetDriveTypeA(drivePath) == DRIVE_FIXED) {
            scanDrive(drivePath);
        }
    }
#else
    std::ifstream mounts("/proc/mounts");
    std::string line;
    while (std::getline(mounts, line)) {
        size_t start = line.find('/');
        size_t end = line.find(' ', start);
        if (start != std::string::npos && end != std::string::npos) {
            std::string partition = line.substr(start, end - start);
            scanDirectory(partition, std::thread::hardware_concurrency(), false);
        }
    }
    mounts.close();
#endif
}

