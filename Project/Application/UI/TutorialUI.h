#pragma once
#include <memory>
#include <vector>
#include "Sprite.h"
#include "TextureManager.h"

// チュートリアル用UI。2枚のスプライトを持つだけのシンプルなクラス
class TutorialUI {
public:
    /// <summary>
    /// 初期化処理
    /// </summary>
    void Initialize(GameEngine::TextureManager* textureManager);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

public:
    // スプライト群の取得（2枚）
    const std::vector<std::unique_ptr<GameEngine::Sprite>>& GetSprites() const { return sprites_; }
    // 各スプライトのテクスチャハンドル取得
    const std::vector<uint32_t>& GetTextureHandles() const { return textureHandles_; }

    // 位置とサイズのパラメータを外部から設定可能にする
    void SetSprite0Params(const Vector2& pos, const Vector2& size) { sprite0Pos_ = pos; sprite0Size_ = size; }
    void SetSprite1Params(const Vector2& pos, const Vector2& size) { sprite1Pos_ = pos; sprite1Size_ = size; }

private:
    // スプライト
    std::vector<std::unique_ptr<GameEngine::Sprite>> sprites_;
    // テクスチャハンドル
    std::vector<uint32_t> textureHandles_;

    // パラメータ（位置とサイズ）
    Vector2 sprite0Pos_ = { 16.0f, 360.0f - 136.0f };
    Vector2 sprite0Size_ = { 300.0f, 120.0f };
    Vector2 sprite1Pos_ = { 16.0f, 360.0f };
    Vector2 sprite1Size_ = { 300.0f, 120.0f };
    Vector2 sprite2Pos_ = { 16.0f, 360.0f + 136.0f };
    Vector2 sprite2Size_ = { 300.0f, 120.0f };
};
