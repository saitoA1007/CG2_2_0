#include "RandomGenerator.h"

std::mt19937 RandomGenerator::randomEngine_;

void RandomGenerator::Initialize() {
    std::random_device rd;
    randomEngine_ = std::mt19937(rd());
}

// Vector3 専用
Vector3 RandomGenerator::GetVector3(float min, float max) {
    return Vector3(
        Get<float>(min, max),
        Get<float>(min, max),
        Get<float>(min, max)
    );
}