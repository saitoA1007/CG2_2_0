#pragma once
#include <optional>
#include<array>
#include<numbers>
#include<functional>

#include"DebugRenderer.h"

#include"Application/Enemy/BossContext.h"
#include"IBossState.h"

class BossStateBattle : public IBossState {
public:

	// ボスの戦い中の振る舞い
	enum class ButtleBehavior {
		Normal, // 状態の切り替えを管理

		RushAttack, // 突進攻撃
		WindAttack, // 風の発射
		IceFallAttack, // 氷柱を落とす攻撃

		Wait, // その場で留まる。攻撃と攻撃の小休憩
		RotateMove, // 回転して回る動き
		CrossMove, // 横断する動き

		InMove, // 最初の時に取る行動

		MaxCount // 状態の数
	};

	// catmull用の点データ
	struct SamplePoint {
		float distance; // 始点からの累積距離
		float rawT;     // その地点での元のt (0.0 - 1.0)
	};

	// 行動に重みを付ける
	struct BehaviorWeight {
		ButtleBehavior behavior;
		int32_t weight; // この数値が高いほど選ばれやすくなる
	};

public:
	BossStateBattle(BossContext& context,const float& stageRadius, GameEngine::DebugRenderer* debugRenderer);

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

	// デバック描画用
	GameEngine::DebugRenderer* debugRenderer_ = nullptr;

	// ボスの振る舞い
	ButtleBehavior currentBehavior_ = ButtleBehavior::Normal;
	// 振る舞いの変更を管理
	std::optional<ButtleBehavior> behaviorRequest_ = std::nullopt;
	// ボスの状態テーブル
	std::array<std::function<void()>, static_cast<size_t>(ButtleBehavior::MaxCount)> behaviorsTable_;
	// ボスが指定した状態を行うためのリセット処理
	std::array<std::function<void()>, static_cast<size_t>(ButtleBehavior::MaxCount)> resetBehaviorParamTable_;
	
	ButtleBehavior selectButtleBehavior_ = ButtleBehavior::Wait;

	// ステージの半径
	float stageRadius_ = 0.0f;

	// 大きさ
	Vector3 size_ = { 1.0f,1.0f,1.0f };

	// 遷移するために使用するリスト
	std::vector<BehaviorWeight> lotteryList_;

private:

	// 突進のフェーズ
	enum class RushPhase {
		In,   // 回転移動
		Main, // 突進
		Out,  // 元の位置に戻る
	};

	// 保存するグループ名
	const std::vector<std::string> kGroupNames = {
		"Boss-Rush",
		"Boss-IceFall",
		"Boss-Wind",
		"Boss-RotateMove",
		"Boss-CrossMove",
		"Boss-Controller"
	};

	// 回転の方向を保存する
	Vector3 startDir_ = {};
	Vector3 endDir_ = {};

	// アニメーションで使用するフラグ
	bool isMidAnimation_ = false;
	bool isEndANimation_ = false;

	// 行動の重み
	int32_t rushAttackWeight_ = 30;
	int32_t WindAttackWeight_ = 30;
	int32_t IceFallWeight_ = 30;
	int32_t WaitWeight_ = 5;
	int32_t RotateMoveWeight_ = 5;
	int32_t CrossMoveWeight_ = 5;

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
	// 開始の回転角度
	Vector3 startRotEndDir_ = {};
	// 終わりの回転角度
	Vector3 endRotStartDir_ = {};

	// 上下移動する回数
	float cycleCount_ = 0;
	// 傾ける最大角度
	float maxTiltAngle_ = std::numbers::pi_v<float> / 6.0f;
	// 傾き
	float targetTilt_ = 0.0f;

	// 突進する時間
	float rushTimer_ = 0.0f;
	float fallTimer_ = 0.0f;

	float rushInTime_ = 0.0f; // 調整不可
	float rushMainTime_ = 2.0f;
	float rushOutTime_ = 2.0f;

	// 突進する位置
	Vector3 startRushPos_ = {};
	Vector3 endRushPos_ = {};
	// 突進の終わる位置の距離をどれだけ伸ばすか
	float offsetEndRush_ = 5.0f;

	// 突進の終わりの位置
	Vector3 rushOutStartPos_ = {};
	Vector3 rushOutEndPos_ = {};

	// Catmull-Rom曲線の制御点
	std::vector<Vector3> controlPoints_;
	// 距離テーブル
	std::vector<SamplePoint> lookupTable; 
	float totalLength = 0.0f;

	// 突進のフェーズ
	RushPhase rushPhase_ = RushPhase::In;

	// 氷柱落とし =============================

	uint32_t tmpIndex = 0;

	// 氷柱を出すまでの時間
	float waitTimer_ = 0.0f;
	float maxWaitTime_ = 2.0f;

	float rotateTimer_ = 0.0f;
	float maxRotateTime_ = 1.0f;

	bool isActiveIceFall_ = false;

	// 待機行動 ================================

	// 待機時間
	float moveWaitTimer_ = 0.0f; 
	float maxMoveWaitTime_ = 2.0f;

	// 間を埋める動作に使用する時間
	float moveTimer_ = 0.0f;

	
	float rotateMoveTime_ = 0.0f;
	float crossMoveTime_ = 3.0f;

	// 風の攻撃 ================================

	float windTimer_ = 0.0f;
	float maxWindTime_ = 5.0f;

	Vector3 startPos_;
	Vector3 endPos_;

	bool isActiveWind_ = false;

private: // 各振る舞いの処理

	// 通常行動のリセット処理
	void ResetNormal();
	// 通常の行動処理
	void NormalUpdate();

	// 突進攻撃のリセット処理
	void ResetRush();
	// 突進の攻撃処理
	void RushAttackUpdate();

	// 風攻撃のリセット処理
	void ResetWind();
	// 風攻撃処理
	void WindAttackUpdate();

	// 氷柱を落とす行動のリセット処理
	void ResetIceFall();
	// 氷柱を落とす攻撃処理
	void IceFallAttackUpdate();

	// 待機行動のリセット処理
	void ResetWait();
	// 待機行動処理
	void WaitUpdate();

	// 最初の入りにとる行動
	void ResetInMove();
	void InMoveUpdate();

	// 円周を回る動き
	void ResetRotateMove();
	void RotateMoveUpdate();

	// 横断する動き
	void ResetCrossMove();
	void CrossMoveUpdate();

private:

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();


	void Setup(uint32_t sampleCount = 100);

	Vector3 GetPositionUniform(float t) const;

	/// <summary>
	/// デバック描画用の線を作成する
	/// </summary>
	void CreateCatmullRom();
};

// ヘルプ関数
namespace {

	// 最短経路で角度を補間する
	float LerpShortAngle(float a, float b, float t);

	// 補間した差分を求める
	float GetAngleDiff(float a, float b);

	// 距離に応じた時間を求める
	float GetMoveTimeDistance(float startAngle, float endAngle, float radius, float speed);

	// 0-360度の範囲に抑える
	float WrapAngle(float angle);

	float EaseOutQuart(float t);

	float EaseInBack(float t);
}