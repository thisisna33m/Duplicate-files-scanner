#ifndef FILE_SCANNER_H
#define FILE_SCANNER_H

#include <string>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>

class FileInfo {
public:
    std::string name;     // File name
    std::string path;     // File path
    size_t size;          // File size
    std::string hash;     // SHA-256 hash of the file

    FileInfo(const std::string& name, const std::string& path, size_t size, const std::string& hash)
        : name(name), path(path), size(size), hash(hash) {}
};

class FileScanner {
private:
    std::vector<FileInfo> files; // Stores information of scanned files
    std::mutex fileMutex;        // Mutex to protect access to `files`

    // Multithreading support
    std::condition_variable queueCondition;
    std::queue<std::string> fileQueue;

    // Private methods
    std::string calculateSHA256(const std::string& filePath); // Calculate SHA-256 hash
    void workerThread();                                     // Worker thread function

public:
    // Public methods
    void scanDirectory(const std::string& path, int threadCount, bool calculateHash); // Scan directory recursively
    void scanDrive(const std::string& drive);                         // Scan specific drive
    void scanDisk();                                                  // Scan full disk

    const std::vector<FileInfo>& getFiles() const;                    // Get scanned file information
};

#endif

