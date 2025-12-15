#include "PlayerAttackDownEffect.h"
#include "MyMath.h"
#include "EngineSource/3D/Model/Material.h"
#include "EasingManager.h"
#include <algorithm>

void PlayerAttackDownEffect::Initialize() {
    isActive_ = true;
    const float kRotate90 = static_cast<float>(M_PI) / 2.0f;
    for (auto &wt : worldTransforms_) {
        wt.Initialize({ {1.5f,1.5f,10.0f}, {kRotate90,0.0f,0.0f}, {0.0f,1.0f,0.0f} });
    }

    // マテリアルの作成
    material_ = std::make_unique<GameEngine::Material>();
    material_->Initialize({1.0f,1.0f,1.0f,1.0f}, {1.0f,1.0f,1.0f}, 50.0f, false);
}

void PlayerAttackDownEffect::Update() {
    if (!isActive_) { return; }
    for (auto &wt : worldTransforms_) {
        wt.transform_.rotate.y += rotateSpeed_;
        wt.UpdateTransformMatrix();
    }
}

void PlayerAttackDownEffect::SetAlpha(float a) {
    if (material_) {
        material_->SetAplha(a);
    }
}

void PlayerAttackDownEffect::SetColorFromAttackPower(float minPower, float maxPower, float currentPower) {
    if (!material_) return;

    // 開始色
    Vector4 startColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    // 中間色
    Vector4 middleColor{ 1.0f, 1.0f, 0.0f, 1.0f };
    // 終了色
    Vector4 endColor{ 1.0f, 0.25f, 0.0f, 1.0f };

    // 正規化されたtを計算 (0.0f ~ 1.0f)
    float t = 0.0f;
    if (maxPower > minPower) {
        t = (currentPower - minPower) / (maxPower - minPower);
        t = std::clamp(t, 0.0f, 1.0f);
    } else {
        // min==max の場合は currentPower >= max -> t=1, else 0
        t = currentPower >= maxPower ? 1.0f : 0.0f;
    }

    Vector4 resultColor;
    if (t <= 0.5f) {
        float localT = t / 0.5f;
        resultColor = Lerp(startColor, middleColor, localT);
    } else {
        float localT = (t - 0.5f) / 0.5f;
        resultColor = Lerp(middleColor, endColor, localT);
    }
    resultColor.w = t; // 透明度もtに応じて変化

    material_->SetColor(resultColor);
}
