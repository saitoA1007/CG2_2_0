#pragma once
#include"IBossAttack.h"
#include "Application/Enemy/BossContext.h"

class ShotBall : public IBossAttack {
public:
    ShotBall(BossContext& context);

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    bool IsFinished() const override { return isFinished_; }

private:

    // 行動
    enum class Phase {
        In,    // 予備動作
        Throw, // 投げる
        Out,   // 終わり
    };

private:
    BossContext& bossContext_;

    bool isFinished_ = false;

    // 行動
    Phase phase_ = Phase::In;

    float timer_ = 0.0f;

    float inTime_ = 1.0f;
    float throwTime_ = 1.0f;
    float outTime_ = 1.0f;

private:

    /// <summary>
    /// 値を登録する
    /// </summary>
    void RegisterBebugParam();

    /// <summary>
    /// 値を適応する
    /// </summary>
    void ApplyDebugParam();
};