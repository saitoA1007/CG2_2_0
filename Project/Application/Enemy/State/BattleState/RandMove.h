#pragma once
#include"IBossAttack.h"
#include "Application/Enemy/BossContext.h"

class RandMove : public IBossAttack {
public:
    RandMove(BossContext& context);

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    bool IsFinished() const override { return isFinished_; }

private:

    // 移動
    enum class Phase {
        In,   // 移動する方向を向く
        Move, // 移動する
        Out,  // プレイヤー側を向くようにする
    };

private:
    BossContext& bossContext_;

    bool isFinished_ = false;

    float timer_ = 0.0f;
    float maxTime_ = 1.0f;

    // 移動する
    Phase phase_ = Phase::In;

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