#pragma once
#include "IBossState.h"
#include "Application/Enemy/BossContext.h"

// Egg: 何もしない、止まっているだけの状態
class BossStateEgg : public IBossState {
public:
    explicit BossStateEgg(BossContext& context) : context_(context) {}

    void Enter() override {
        // アニメーションは基本移動を設定しておくが、進行は固定
        if (context_.animator_ && context_.animationData_) {
            context_.animator_->SetAnimationData(&(*context_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);
            context_.animationTimer = 0.0f;
        }
        // 攻撃系フラグをすべて無効化
        context_.isActiveIceFall = false;
        context_.isRushAttack_ = false;
        context_.isRushCollisionActive_ = false;
        context_.isWindAttack_ = false;
        context_.isWingsEffect_ = false;
    }

    void Update() override {
        // 何もしないで静止。行列更新などは外側で行われる。
        // アニメーションの進行も止めたい場合は固定のまま。
    }

    void Exit() override {
        // 特に無し
    }

private:
    BossContext& context_;
};
