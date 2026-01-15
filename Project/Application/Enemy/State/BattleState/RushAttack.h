#pragma once
#include"IBossAttack.h"
#include "Application/Enemy/BossContext.h"

class RushAttack : public IBossAttack {
public:
    RushAttack(BossContext& context);

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    bool IsFinished() const override { return isFinished_; }

private:

    // 移動
    enum class Phase {
        In,   // 移動する方向を向く
        Move, // 移動する
        Out,  // 余韻
    };

private:
    BossContext& bossContext_;

    bool isFinished_ = false;

    // 値の保存グループ名
    const std::string groupName_ = "Boss_RushAttack";

    Phase phase_ = Phase::In;

    float timer_ = 0.0f;

    // 移動する方向
    Vector3 dir_ = {};
    
    uint32_t rushSH_ = 0;

private: // 調整項目

    // 移動時間
    float InTime_ = 1.0f;
    float moveTime_ = 1.0f;
    float OutTime_ = 1.0f;

    // 回転速度
    float rotateSpeed_ = 5.0f;

    // 突進速度
    float startRushSpeed_ = 50.0f;
    float endRushSpeed_ = 0.0f;

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