#ifndef BY_CONTENT_STRATEGY_H
#define BY_CONTENT_STRATEGY_H

#include "DuplicateDetectionStrategy.h"
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <cstring>

class ByContentStrategy : public DuplicateDetectionStrategy {
public:
    void findDuplicates(const std::vector<FileInfo>& files) override;

private:
    bool compareFiles(const std::string& filePath1, const std::string& filePath2);
};

#endif

