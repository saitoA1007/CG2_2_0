#include "PlayerRushEffect.h"

void PlayerRushEffect::Initialize() {
    isActive_ = true;
    for (auto &wt : worldTransforms_) {
        wt.Initialize({ {1.0f,1.0f,1.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} });
    }
}

void PlayerRushEffect::Update() {
    if (!isActive_) { return; }
    for (auto &wt : worldTransforms_) {
        wt.transform_.rotate.y += rotateSpeed_;
        wt.UpdateTransformMatrix();
    }
}
