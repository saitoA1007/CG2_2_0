#pragma once

/// <summary>
/// ボスのバトル中の攻撃行動の基底クラス
/// </summary>
class IBossAttack {
public:
    virtual ~IBossAttack() = default;

    /// <summary>
    /// 攻撃の初期化
    /// </summary>
    virtual void Initialize() = 0;

    /// <summary>
    /// 攻撃の更新処理
    /// </summary>
    virtual void Update() = 0;

    /// <summary>
    /// 攻撃が完了したかどうか
    /// </summary>
    virtual bool IsFinished() const = 0;

    /// <summary>
    /// 攻撃の終了処理
    /// </summary>
    virtual void Finalize() = 0;
};

// サンプルコード
//#include"IBossAttack.h"
//#include "Application/Enemy/BossContext.h"
//
//class StampFall : public IBossAttack {
//public:
//    StampFall(BossContext& context);
//
//    void Initialize() override;
//    void Update() override;
//    bool IsFinished() const override;
//    void Finalize() override;
//
//private:
//    BossContext& bossContext_;
//
//};
