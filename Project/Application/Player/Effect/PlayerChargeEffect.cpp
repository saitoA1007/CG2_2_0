#include "PlayerChargeEffect.h"
#include "MyMath.h"
#include "FPSCounter.h"

void PlayerChargeEffect::Initialize() {
    isActive_ = true;
    const float kRotate90 = static_cast<float>(M_PI) / 2.0f;
    const float baseScale = 2.0f;
    std::array<Vector3, 3> positions = {
        Vector3{ 0.0f, 0.5f, 0.0f},
        Vector3{ 0.0f, 0.5f, 0.0f},
        Vector3{ 0.0f, 0.5f, 0.0f}
    };

    for (size_t i = 0; i < worldTransforms_.size(); ++i) {
        Transform t;
        float scaleFactor = (1.0f + static_cast<float>(i) * 0.5f) * baseScale;
        t.scale = { scaleFactor, scaleFactor, scaleFactor };
        t.rotate = { kRotate90, 0.0f, 0.0f };
        t.translate = positions[i];
        worldTransforms_[i].Initialize(t);
    }
    activeCount_ = 1;
}

void PlayerChargeEffect::Update() {
    if (!isActive_) { return; }

    for (int i = 0; i < activeCount_; ++i) {
        worldTransforms_[i].transform_.rotate.y += rotateSpeed_ * GameEngine::FpsCounter::deltaTime;
        worldTransforms_[i].UpdateTransformMatrix();
    }
}

void PlayerChargeEffect::SetLevel(int level) {
    if (level < 1) level = 1;
    if (level > 3) level = 3;
    activeCount_ = level;
}
