#pragma once
#include"Application/Enemy/BossContext.h"
#include"IBossState.h"
#include <optional>
#include<array>
#include<functional>

class BossStateBattle : public IBossState {
public:

	// ボスの戦い中の振る舞い
	enum class ButtleBehavior {
		Normal, // 通常状態
		RushAttack, // 突進攻撃
		ShotAttack, // 弾の発射
		FallAttack, // 氷柱を落とす攻撃

		MaxCount // 状態の数
	};

public:

	BossStateBattle(BossContext& context,const float& stageRadius);
	void Enter() override;
	void Update() override;
	void Exit() override;

private:

	// ボスの共通データを取得
	BossContext& bossContext_;

	// ボスの振る舞い
	ButtleBehavior behavior_ = ButtleBehavior::Normal;
	// 振る舞いの変更を管理
	std::optional<ButtleBehavior> behaviorRequest_ = std::nullopt;
	// ボスの状態テーブル
	std::array<std::function<void()>, static_cast<size_t>(ButtleBehavior::MaxCount)> behaviorsTable_;
	// ボスが指定した状態を行うためのリセット処理
	std::array<std::function<void()>, static_cast<size_t>(ButtleBehavior::MaxCount)> resetBehaviorParamTable_;
	
	// ステージの半径
	float stageRadius_ = 0.0f;

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