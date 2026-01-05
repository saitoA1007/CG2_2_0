#pragma once
#include"IBossAttack.h"
#include "Application/Enemy/BossContext.h"

class RotateAttack : public IBossAttack {
public:
    RotateAttack(BossContext& context);

    void Initialize() override;
    void Update() override;
    void Finalize() override;

    bool IsFinished() const override { return isFinished_; }

private:
    // 移動
    enum class Phase {
        In,   // 回転するまでの動作
        Move, // 移動
        Out,  // 回転が弱まる動作
    };

private:
    BossContext& bossContext_;

    bool isFinished_ = false;

    Phase phase_ = Phase::In;

    // 値の保存グループ名
    const std::string groupName_ = "Boss_RotateAttack";

    float timer_ = 0.0f;

    float rotSpeed_ = 0.0f;

    float moveSpeed_ = 0.0f;

    Vector3 dir_;

    bool isMove_ = false;

private: // 調整項目

    float inTime_ = 1.0f;
    float moveTime_ = 1.0f;
    float outTime_ = 1.0f;
    
    // 最大の回転速度
    float maxRotSpeed_ = 20.0f;

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