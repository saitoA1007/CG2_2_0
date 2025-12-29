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
    BossContext& bossContext_;

    bool isFinished_ = false;

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