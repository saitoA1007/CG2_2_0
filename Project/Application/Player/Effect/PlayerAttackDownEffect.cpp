#include "PlayerAttackDownEffect.h"
#include "MyMath.h"

void PlayerAttackDownEffect::Initialize() {
    isActive_ = true;
    const float kRotate90 = static_cast<float>(M_PI) / 2.0f;
    for (auto &wt : worldTransforms_) {
        wt.Initialize({ {1.5f,1.5f,10.0f}, {kRotate90,0.0f,0.0f}, {0.0f,1.0f,0.0f} });
    }
}

void PlayerAttackDownEffect::Update() {
    if (!isActive_) { return; }
    for (auto &wt : worldTransforms_) {
        wt.transform_.rotate.y += rotateSpeed_;
        wt.UpdateTransformMatrix();
    }
}
