#ifndef BY_NAME_STRATEGY_H
#define BY_NAME_STRATEGY_H

#include "DuplicateDetectionStrategy.h"
#include <unordered_map>
#include <iostream>

class ByNameStrategy : public DuplicateDetectionStrategy {
public:
    void findDuplicates(const std::vector<FileInfo>& files) override;
};

#endif
