#include "DuplicateDetectorFactory.h"

std::unique_ptr<DuplicateDetectionStrategy> DuplicateDetectorFactory::create(const std::string& strategyType) {
    if (strategyType == "-n") {
        return std::make_unique<ByNameStrategy>();
    } else if (strategyType == "-h") {
        return std::make_unique<ByContentStrategy>();
    } else {
        throw std::invalid_argument("Unknown strategy type: " + strategyType);
    }
}
