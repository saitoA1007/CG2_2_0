#pragma once
#include <memory>
#include "EngineSource/2D/Sprite.h"
#include "EngineSource/Math/Vector4.h"

class Letterbox {
public:
    Letterbox() = default;
    ~Letterbox() = default;

    /// <summary>
    /// 初期化
    /// </summary>
    /// <param name="screenWidth">画面幅</param>
    /// <param name="screenHeight">画面高さ</param>
    /// <param name="boxHeight">黒帯の高さ</param>
    /// <param name="color">黒帯の色</param>
    void Initialize(float screenWidth, float screenHeight, float boxHeight = 32.0f, const Vector4& color = { 0.0f, 0.0f, 0.0f, 1.0f });

    /// <summary>
    /// 黒帯の高さ設定
    /// </summary>
    /// <param name="boxHeight">黒帯の高さ</param>
    void SetBoxHeight(float boxHeight);

    void Update();

    GameEngine::Sprite* GetTopSprite() const { return topSprite_.get(); }
    GameEngine::Sprite* GetBottomSprite() const { return bottomSprite_.get(); }

private:
    float screenWidth_ = 0.0f;
    float screenHeight_ = 0.0f;
    float boxHeight_ = 0.0f;
    Vector4 color_ { 0.0f, 0.0f, 0.0f, 1.0f };

    std::unique_ptr<GameEngine::Sprite> topSprite_;
    std::unique_ptr<GameEngine::Sprite> bottomSprite_;
};
