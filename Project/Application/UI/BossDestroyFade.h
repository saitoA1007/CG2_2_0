#pragma once
#include"Sprite.h"

class BossDestroyFade {
public:

    enum class Phase {
        In,
        Wait,
        Out
    };

    // 初期化処理
    void Initialize();

    // 更新処理
    void Update();

    GameEngine::Sprite* GetSprite() { return sprite_.get(); }

    void SetActive() {
        isActive_ = true;
        isMiddle_ = false;
        timer_ = 0.0f;
        phase_ = Phase::In;
    }

    // 有効を表示
    bool IsActive()const {return isActive_;}

    // 中間を表示
    bool IsMiddle() const { return isMiddle_; }

private:
    // 画像
    std::unique_ptr<GameEngine::Sprite> sprite_;

    float timer_ = 0.0f;

    bool isActive_ = false;

    bool isMiddle_ = false;

    Phase phase_;
};