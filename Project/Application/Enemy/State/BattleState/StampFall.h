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

    // 値の保存グループ名
    const std::string groupName_ = "Boss_StampFall";

    // 最初の移動する位置
    float startRisePosY_ = 0.0f;
    // 移動する最初の位置
    Vector3 startMovePos_ = {};
    // 移動する最後の位置
    Vector3 endMovePos_ = {};

    float timer_ = 0.0f;

    // 着地音
    uint32_t landSH_ = 0;

private: // 調整項目

    // 最大の高さ
    float maxHeightPosY_ = 15.0f;

    // 移動時間
    float riseTime_ = 1.0f;
    float moveTime_ = 1.0f;
    float fallTime_ = 1.0f;

    // 回転速度
    float rotateSpeed_ = 5.0f;

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