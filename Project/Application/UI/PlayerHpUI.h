#pragma once
#include<list>
#include<vector>
#include"Sprite.h"
#include "TextureManager.h"

class PlayerHpUI {
public:

    struct Point {
        float startScale;
        float endScale;
        float timer = 0.0f;
    };

public:

    /// <summary>
    /// 初期化処理
    /// </summary>
    /// <param name="maxHp"></param>
    void Initialize(const int32_t &maxHp, GameEngine::TextureManager *textureManager);

    /// <summary>
    /// 更新処理
    /// </summary>
    void Update();

public:

    void SetCurrentHp(const int32_t& hp) { currentHp_ = hp; }

    // HP表示用スプライト配列を取得
    const std::vector<std::unique_ptr<GameEngine::Sprite>>& GetHpSprites() const { return hpSprites_; }
    // HPアイコンのテクスチャハンドルを取得
    uint32_t GetHpIconGH() const { return hpIconGH_; }

private:

    // HP表示用のスプライト配列（個数式）
    std::vector<std::unique_ptr<GameEngine::Sprite>> hpSprites_;
    // HPのテクスチャハンドル
    uint32_t hpIconGH_ = 0;

    // 最大hp
    int32_t maxHp_ = 0;

    // 現在のhp
    int32_t currentHp_ = 0;

    Vector2 iconSize_ = { 64.0f, 64.0f };
    Vector2 startPos_ = { 32.0f, 32.0f };
    float iconSpacing_ = 4.0f;

private:
    /// <summary>
    /// 演出の更新処理（個数式では未使用）
    /// </summary>
    void EffectUpdate() {}
};
