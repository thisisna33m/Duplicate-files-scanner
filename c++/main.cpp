#include "FileScanner.h"
#include "DuplicateDetectorFactory.h"
#include <iostream>
#include <chrono>

void printUsage() {
    std::cout << "Usage: ./scanner -p path [-n | -h]\n";
    std::cout << "Options:\n";
    std::cout << "  -p path   Specify the path to scan\n";
    std::cout << "  -n        Find duplicates by name\n";
    std::cout << "  -h        Find duplicates by content (hash and byte comparison)\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printUsage();
        return 1;
    }

    std::string optionPath, optionMode;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-p" && i + 1 < argc) {
            optionPath = argv[++i];
        } else if (std::string(argv[i]) == "-n" || std::string(argv[i]) == "-h") {
            optionMode = argv[i];
        } else {
            printUsage();
            return 1;
        }
    }

    if (optionPath.empty() || optionMode.empty()) {
        printUsage();
        return 1;
    }

    FileScanner scanner;

    bool calculateHash = (optionMode == "-h");
    int threadCount = calculateHash ? std::thread::hardware_concurrency() : 1;

    std::cout << "Scanning path: " << optionPath << "...\n";

    // Measure scanning time
    auto start = std::chrono::high_resolution_clock::now();
    scanner.scanDirectory(optionPath, threadCount, calculateHash);
    auto end = std::chrono::high_resolution_clock::now();

    const auto& files = scanner.getFiles();
    std::cout << "Scan complete. Found " << files.size() << " files in "
              << std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " seconds.\n";

    try {
        auto detector = DuplicateDetectorFactory::create(optionMode);
        detector->findDuplicates(files);
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

