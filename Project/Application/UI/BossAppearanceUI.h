#pragma once
#include <memory>
#include "Vector2.h"
#include "Vector4.h"
#include "Sprite.h"
#include "TextureManager.h"

class BossAppearanceUI {
public:
    BossAppearanceUI() = default;
    ~BossAppearanceUI() = default;

    void Initialize(GameEngine::TextureManager* textureManager);

    void Update();

    GameEngine::Sprite *GetBossTitleSprite() const { return bossTitleSprite_.get(); }
    GameEngine::Sprite *GetBossNameSprite() const { return bossNameSprite_.get(); }
    GameEngine::Sprite *GetBossAppearanceStarSprite() const { return bossAppearanceStarSprite_.get(); }

    // Activation control
    void SetActive(bool active) { isActive_ = active; }
    bool IsActive() const { return isActive_; }

    // Texture handles for drawing
    uint32_t GetCurrentBossTitleGH() const { return currentBossTitleGH_; }
    uint32_t GetCurrentBossNameGH() const { return currentBossNameGH_; }
    uint32_t GetBossAppearanceStarGH() const { return bossAppearanceStarGH_; }

private:
    uint32_t bossTitleTextGH_ = 0;
    uint32_t bossTitle1GH_ = 0;
    uint32_t bossTitle2GH_ = 0;
    uint32_t bossNameTextGH_ = 0;
    uint32_t bossName1GH_ = 0;
    uint32_t bossName2GH_ = 0;
    uint32_t bossAppearanceStarGH_ = 0;

    uint32_t currentBossTitleGH_ = 0;
    uint32_t currentBossNameGH_ = 0;

    std::unique_ptr<GameEngine::Sprite> bossTitleSprite_;
    std::unique_ptr<GameEngine::Sprite> bossNameSprite_;
    std::unique_ptr<GameEngine::Sprite> bossAppearanceStarSprite_;

    Vector2 offsetPos_ = { 640.0f, 360.0f };

    float timer_ = 0.0f;
    float animationDuration_ = 3.0f;
    bool isActive_ = false;
    bool prevIsActive_ = false;

    void ResetAnimation();
    void UpdateAnimation();
};
