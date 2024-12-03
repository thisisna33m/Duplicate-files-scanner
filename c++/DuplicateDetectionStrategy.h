#ifndef DUPLICATE_DETECTION_STRATEGY_H
#define DUPLICATE_DETECTION_STRATEGY_H

#include "FileScanner.h"
#include <vector>

class DuplicateDetectionStrategy {
public:
    virtual void findDuplicates(const std::vector<FileInfo>& files) = 0;
    virtual ~DuplicateDetectionStrategy() = default;
};

#endif

