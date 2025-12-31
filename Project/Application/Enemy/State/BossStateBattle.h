#pragma once
#include<array>
#include<numbers>

#include"IBossState.h"
#include"Application/Enemy/BossContext.h"
#include"BattleState/IBossAttack.h"

class BossStateBattle : public IBossState {
public:

	// ボスの戦い中の振る舞い
	enum class BattleBehavior {
		StampFall, // スタンプ攻撃
		ShotBall, // 岩を投げる攻撃
		RotateAttackMove, // 回転攻撃
		RushAttack, // 突進
		RandBallAttack, // ランダムにボールを飛ばす攻撃
		Wait, // 待機
		RandMove, // ランダムに移動する

		MaxCount // 状態の数
	};

	// 行動に重みを付ける
	struct BehaviorWeight {
		BattleBehavior behavior;
		int32_t weight;
	};

public:

	BossStateBattle(BossContext& context);

	/// <summary>
	/// 入りの処理
	/// </summary>
	void Enter() override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// 終わりの処理
	/// </summary>
	void Exit() override;

private: // 調整項目

	// 行動に対応する重み
	int32_t stampFallWeight_;
	int32_t shotBallWeight_;
	int32_t rotateAttackMoveWeight_;
	int32_t rushAttackWeight_;
	int32_t randBallAttackWeight_;
	int32_t waitWeight_;

private:
	// ボスの共通データを取得
	BossContext& bossContext_;

	std::string kGroupName = "BossStateBattle";

	// ボスの状態テーブル
	std::array<std::unique_ptr<IBossAttack>, static_cast<size_t>(BattleBehavior::MaxCount)> behaviorsTable_;
	BattleBehavior currentBehavior_;

	// 遷移するために使用するリスト
	std::vector<BehaviorWeight> lotteryList_;

private:

	/// <summary>
	/// 次の攻撃行動を取得する
	/// </summary>
	/// <returns></returns>
	BattleBehavior SelectWeightedAttack();

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};
