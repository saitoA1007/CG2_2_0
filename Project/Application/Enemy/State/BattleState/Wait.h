#pragma once
#include"IBossAttack.h"
#include "Application/Enemy/BossContext.h"

class Wait : public IBossAttack {
public:
    Wait(BossContext& context);

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    bool IsFinished() const override { return isFinished_; }
    
private:
    BossContext& bossContext_;

    bool isFinished_ = false;

    float timer_ = 0.0f;
    float maxTime_ = 1.0f;

private:
    /// <summary>
    /// 値を登録する
    /// </summary>
    void RegisterDebugParam();

    /// <summary>
    /// 値を適応する
    /// </summary>
    void ApplyDebugParam();
};