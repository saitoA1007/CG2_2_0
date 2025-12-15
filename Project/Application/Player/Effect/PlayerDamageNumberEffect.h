#pragma once
#include <memory>
#include <vector>
#include <string>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Camera.h"
#include "Sprite.h"
#include "TextureManager.h"

// プレイヤーが与えたダメージを2Dスプライトで表示するエフェクト
class PlayerDamageNumberEffect {
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize(GameEngine::TextureManager* textureManager);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

    /// <summary>
    /// エミッタを起動（3D位置とダメージ量から生成）
    /// </summary>
    /// <param name="worldPos">3Dワールド座標</param>
    /// <param name="damage">ダメージ量</param>
    /// <param name="camera">投影に使用するカメラ</param>
    void Emitter(const Vector3& worldPos, int damage, const GameEngine::Camera& camera);

public:
    /// <summary>
    /// 有効状態取得
    /// </summary>
    bool IsActive() const { return isActive_; }

    // TDGameSceneの描画用（非所有ポインタとハンドル群）
    const std::vector<GameEngine::Sprite*>& GetDigitSprites() const { return digitSprites_; }
    const std::vector<uint32_t>& GetTextureHandles() const { return digitTextureHandles_; }

    /// <summary>
    /// 桁サイズ設定（幅・高さ）
    /// </summary>
    void SetDigitSize(const Vector2& size) { digitSize_ = size; }
    Vector2 GetDigitSize() const { return digitSize_; }

    /// <summary>
    /// 桁カラー設定（RGBA）
    /// </summary>
    void SetDigitColor(const Vector4& color) { digitColor_ = color; }
    Vector4 GetDigitColor() const { return digitColor_; }

private:
    struct DigitSprite {
        std::unique_ptr<GameEngine::Sprite> numSprite_;
        uint32_t number = 0; // テクスチャハンドル
        Vector2 offset;
    };

    // スクリーン座標の原点
    Vector2 spawnScreenPos_ {0.0f, 0.0f};
    // 表示中の桁スプライト
    std::vector<DigitSprite> digits_;
    // TDGameScene描画向け
    std::vector<GameEngine::Sprite*> digitSprites_;
    std::vector<uint32_t> digitTextureHandles_;

    // アニメーションタイマー（0.0 ～ 1.0）
    float timer_ = 0.0f;
    // 有効フラグ
    bool isActive_ = false;
    // エフェクト継続時間
    float duration_ = 1.0f;

    // 数字のサイズと色
    Vector2 digitSize_ = { 64.0f, 64.0f };
    Vector4 digitColor_ = { 1.0f, 0.25f, 0.0f, 1.0f };

    GameEngine::TextureManager* textureManager_ = nullptr;

private:
    void BuildDigits(int damage);
    static Vector2 WorldToScreen(const Vector3& worldPos, const GameEngine::Camera& camera);
};
