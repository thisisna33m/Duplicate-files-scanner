#include "ByContentStrategy.h"

void ByContentStrategy::findDuplicates(const std::vector<FileInfo>& files) {
    std::unordered_map<std::string, std::vector<FileInfo>> hashMap;

    // Group files by their hash values
    for (const auto& file : files) {
        hashMap[file.hash].push_back(file);
    }

    // Print duplicates by hash
    for (const auto& [hash, duplicates] : hashMap) {
        if (duplicates.size() > 1) {
            std::cout << "\nDuplicate by Content (Hash): " << hash << "\n";
            for (const auto& file : duplicates) {
                std::cout << "  Name: " << file.name 
                          << ", Path: " << file.path 
                          << ", Size: " << file.size << " bytes\n";
            }
        }
    }
}

bool ByContentStrategy::compareFiles(const std::string& filePath1, const std::string& filePath2) {
    std::ifstream file1(filePath1, std::ios::binary);
    std::ifstream file2(filePath2, std::ios::binary);

    if (!file1 || !file2) return false;

    char buffer1[8192], buffer2[8192];
    while (file1.read(buffer1, sizeof(buffer1)) && file2.read(buffer2, sizeof(buffer2))) {
        if (file1.gcount() != file2.gcount() || std::memcmp(buffer1, buffer2, file1.gcount()) != 0) {
            return false;
        }
    }

    return file1.eof() && file2.eof();
}

