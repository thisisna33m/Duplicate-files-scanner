#ifndef DUPLICATE_DETECTOR_FACTORY_H
#define DUPLICATE_DETECTOR_FACTORY_H

#include "DuplicateDetectionStrategy.h"
#include "ByNameStrategy.h"
#include "ByContentStrategy.h"

class DuplicateDetectorFactory {
public:
    static std::unique_ptr<DuplicateDetectionStrategy> create(const std::string& strategyType);
};

#endif
