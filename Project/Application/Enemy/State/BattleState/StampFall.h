#pragma once
#include"IBossAttack.h"
#include "Application/Enemy/BossContext.h"

class StampFall : public IBossAttack {
public:
    StampFall(BossContext& context);

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    bool IsFinished() const override { return isFinished_; }

private:
    // 行動
    enum class Phase {
        Rise,     // 上昇
        Move,     // 移動
        Fall,     // 落下
    };

private:
    BossContext& bossContext_;

    bool isFinished_ = false;

    // 行動
    Phase phase_ = Phase::Rise;

    float timer_ = 0.0f;

    // 移動時間
    float riseTime_ = 1.0f;
    float moveTime_ = 1.0f;
    float fallTime_ = 1.0f;

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