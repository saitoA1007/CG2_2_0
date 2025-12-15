#define NOMINMAX
#include "PlayerLandingEffect.h"
#include "MyMath.h"
#include "Material.h"
#include "EasingManager.h"
#include <algorithm>

void PlayerLandingEffect::Initialize() {
    isActive_ = false;
    timer_ = 0.0f;
    const float kRotate90 = static_cast<float>(M_PI) / 2.0f;
    for (auto &wt : worldTransforms_) {
        wt.Initialize({ {0.1f,0.1f,0.1f}, {kRotate90,0.0f,0.0f}, {0.0f,0.0f,0.0f} });
    }
    for (auto &m : materials_) {
        m = std::make_unique<GameEngine::Material>();
        m->Initialize({ 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f }, 250.0f, false);
    }
}

void PlayerLandingEffect::Emitter(const Vector3 &pos, float power) {
    // 位置を設定してタイマーをリセット
    for (auto &wt : worldTransforms_) {
        wt.transform_.translate = pos;
        wt.transform_.translate.y += 1.0f; // 少し上にずらす
        wt.transform_.scale = {0.1f, 0.1f, 0.1f};
        wt.UpdateTransformMatrix();
    }
    timer_ = 0.0f;
    isActive_ = true;
    landingPower_ = std::clamp(power, 0.0f, 1.0f);

    // 開始色
    Vector4 startColor{ 1.0f, 1.0f, 1.0f, 1.0f };
    // 中間色
    Vector4 middleColor{ 1.0f, 1.0f, 0.0f, 1.0f };
    // 終了色
    Vector4 endColor{ 1.0f, 0.25f, 0.0f, 1.0f };

    Vector4 resultColor;
    if (landingPower_ <= 0.5f) {
        float localT = landingPower_ / 0.5f;
        resultColor = Lerp(startColor, middleColor, localT);
    } else {
        float localT = (landingPower_ - 0.5f) / 0.5f;
        resultColor = Lerp(middleColor, endColor, localT);
    }

    // マテリアル初期透明度
    for (auto &m : materials_) {
        if (m) { m->SetColor(resultColor); }
    }
}

void PlayerLandingEffect::Update() {
    if (!isActive_) { return; }

    // 時間を進める (フレーム依存・必要なら外部DeltaTimeに差し替え)
    const float dt = 1.0f / 60.0f;
    timer_ += dt;

    // インスタンスごとの開始遅延（秒）
    const float kInstanceDelay = 0.1f;

    for (size_t i = 0; i < worldTransforms_.size(); ++i) {
        float localTime = std::max(timer_ - static_cast<float>(i) * kInstanceDelay, 0.0f);
        float t = std::clamp(localTime / duration_, 0.0f, 1.0f);

        // スケールを徐々に拡大
        float startScale = 1.0f * landingPower_;
        float endScale = 8.0f * landingPower_;
        float s = EaseOutQuad(startScale, endScale, t);

        // 透明度を徐々に減少 (1.0 -> 0.0)
        float a = std::lerp(1.0f, 0.0f, t);

        auto &wt = worldTransforms_[i];
        wt.transform_.scale = {s, s, s};
        wt.UpdateTransformMatrix();

        auto &m = materials_[i];
        if (m) { m->SetAplha(a); }
    }

    // 全インスタンスが終了したら非アクティブ化
    if (timer_ >= duration_ + kInstanceDelay * (worldTransforms_.size() - 1)) {
        isActive_ = false;
    }
}
