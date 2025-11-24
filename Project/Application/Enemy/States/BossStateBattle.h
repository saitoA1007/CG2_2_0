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
		IceFallAttack, // 氷柱を落とす攻撃

		MaxCount // 状態の数
	};

public:
	BossStateBattle(BossContext& context,const float& stageRadius);

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

private:

	// ボスの共通データを取得
	BossContext& bossContext_;

	// ボスの振る舞い
	ButtleBehavior currentBehavior_ = ButtleBehavior::Normal;
	// 振る舞いの変更を管理
	std::optional<ButtleBehavior> behaviorRequest_ = std::nullopt;
	// ボスの状態テーブル
	std::array<std::function<void()>, static_cast<size_t>(ButtleBehavior::MaxCount)> behaviorsTable_;
	// ボスが指定した状態を行うためのリセット処理
	std::array<std::function<void()>, static_cast<size_t>(ButtleBehavior::MaxCount)> resetBehaviorParamTable_;
	
	// 現在の状態が有効化管理
	bool isCurrentBehaviorActive_ = false;

	// ステージの半径
	float stageRadius_ = 0.0f;

	// 大きさ
	Vector3 size_ = { 1.0f,1.0f,1.0f };

private:

	// Behavior : Normal
	// 行動をおこなうための繋ぎの行動に使用する変数============

	// 元の場所に戻る位置
	Vector3 startBackPos_ = {};
	Vector3 endBackPos_ = {};

	// 元の場所に戻る時間
	float backTimer_ = 0.0f;
	float backMaxTime_ = 1.0f;

	// 普段いる高さ
	float defalutPosY_ = 5.0f;

	// 突進に使う変数======================

	// 回転の時に移動する速度
	float rotSpeed_ = 20.0f;

	// 回転する時間
	float rotMoveTimer_ = 0.0f;
	float rotMaxMoveTime_ = 0.0f;
	// 移動する角度
	float startAngle_ = 0.0f;
	float endAngle_ = 0.0f;

	// 突進する時間
	float rushTimer_ = 0.0f;
	float fallTimer_ = 0.0f;
	float rushMaxTime_ = 2.0f;

	// 突進する位置
	Vector3 startRushPos_ = {};
	Vector3 endRushPos_ = {};
	// 突進の終わる位置の距離をどれだけ伸ばすか
	float offsetEndRush_ = 5.0f;

	bool isRotMove_ = true;

	// 氷柱落とし =============================

	// 時間
	float iceFallTimer_ = 0.0f;
	float maxIceFallTime_ = 3.0f;

	// 移動する位置
	Vector3 startIceFall_ = {};
	Vector3 endIceFall_ = {};

	// 飛ぶ最大の高さ
	float iceFallHeight = 8.0f;
	float iceFallHeightTimer_ = 0.0f;

	// 落とす時間
	float fallTime_ = 0.0f;
	// 氷柱を落とすフラグ
	bool isActiveIceFall_ = false;

private: // 各振る舞いの処理

	/// <summary>
	/// 振る舞いを管理
	/// </summary>
	void ControllBehavior();

	/// <summary>
	/// 通常行動のリセット処理
	/// </summary>
	void ResetNormal();

	/// <summary>
	/// 通常の行動処理
	/// </summary>
	void NormalUpdate();

	/// <summary>
	/// 突進攻撃のリセット処理
	/// </summary>
	void ResetRush();

	/// <summary>
	/// 突進の攻撃処理
	/// </summary>
	void RushAttackUpdate();

	/// <summary>
	/// 弾を発射する攻撃処理
	/// </summary>
	void ShotAttackUpdate();

	/// <summary>
	/// 氷柱を落とす行動のリセット処理
	/// </summary>
	void ResetIceFall();

	/// <summary>
	/// 氷柱を落とす攻撃処理
	/// </summary>
	void IceFallAttackUpdate();

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

// ヘルプ関数
namespace {

	// 最短経路で角度を補間する
	float LerpShortAngle(float a, float b, float t);

	// 補間した差分を求める
	float GetAngleDiff(float a, float b);

	// 距離に応じた時間を求める
	float GetMoveTimeDistance(float startAngle, float endAngle, float radius, float speed);

	float EaseOutQuart(float t);
}