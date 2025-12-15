#include "PlayerChargeEffect.h"
#include "MyMath.h"
#include "FPSCounter.h"

void PlayerChargeEffect::Initialize() {
    isActive_ = true;
    const float kRotate90 = static_cast<float>(M_PI) / 2.0f;
    const float baseScale = 2.0f;
    std::array<Vector3, 3> positions = {
        Vector3{ 0.0f, 1.0f, 0.0f},
        Vector3{ 0.0f, 1.0f, 0.0f},
        Vector3{ 0.0f, 1.0f, 0.0f}
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

    // マテリアルの初期化（白）
    material_ = std::make_unique<GameEngine::Material>();
    material_->Initialize(Vector4{1.0f, 1.0f, 1.0f, 1.0f}, Vector3{1.0f, 1.0f, 1.0f}, 16.0f, false);
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

    // レベルに応じて色と縦スケールを変更: 1 白, 2 黄色, 3 オレンジ
    if (material_) {
        switch (activeCount_) {
        case 1:
            material_->SetColor(Vector4{1.0f, 1.0f, 1.0f, 1.0f});
            for (auto& wt : worldTransforms_) {
                wt.transform_.scale.z = 2.0f;
                wt.UpdateTransformMatrix();
            }
            break;
        case 2:
            material_->SetColor(Vector4{1.0f, 1.0f, 0.0f, 1.0f});
            for (auto& wt : worldTransforms_) {
                wt.transform_.scale.z = 3.0f;
                wt.UpdateTransformMatrix();
            }
            break;
        case 3:
            material_->SetColor(Vector4{1.0f, 0.25f, 0.0f, 1.0f});
            for (auto& wt : worldTransforms_) {
                wt.transform_.scale.z = 4.0f;
                wt.UpdateTransformMatrix();
            }
            break;
        }
    }
}
