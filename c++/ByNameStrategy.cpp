#include "ByNameStrategy.h"

void ByNameStrategy::findDuplicates(const std::vector<FileInfo>& files) {
    std::unordered_map<std::string, std::vector<FileInfo>> nameMap;

    for (const auto& file : files) {
        nameMap[file.name].push_back(file);
    }

    for (const auto& [name, duplicates] : nameMap) {
        if (duplicates.size() > 1) {
            std::cout << "\nDuplicate by Name: " << name << "\n";
            for (const auto& file : duplicates) {
                std::cout << "  Path: " << file.path << ", Size: " << file.size << " bytes\n";
            }
        }
    }
}
