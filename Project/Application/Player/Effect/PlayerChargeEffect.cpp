#include "PlayerChargeEffect.h"
#include "MyMath.h"

void PlayerChargeEffect::Initialize() {
    isActive_ = true;
    const float kRotate90 = static_cast<float>(M_PI) / 2.0f;
    for (auto &wt : worldTransforms_) {
        wt.Initialize({ {2.0f,2.0f,2.0f}, {kRotate90,0.0f,0.0f}, {0.0f,0.0f,0.0f} });
    }
}

void PlayerChargeEffect::Update() {
    if (!isActive_) { return; }
    for (auto &wt : worldTransforms_) {
        wt.transform_.rotate.y += rotateSpeed_;
        wt.UpdateTransformMatrix();
    }
}
