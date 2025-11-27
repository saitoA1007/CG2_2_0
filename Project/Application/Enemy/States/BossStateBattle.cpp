#include"BossStateBattle.h"
#include"Application/Enemy/BossState.h"
#include"MyMath.h"
#include"FPSCounter.h"
#include"EasingManager.h"
#include"RandomGenerator.h"
#include<numbers>
using namespace GameEngine;

BossStateBattle::BossStateBattle(BossContext& context, const float& stageRadius) : bossContext_(context) {

	// ステージの半径を取得
	stageRadius_ = stageRadius;

	// 各振る舞いの更新処理を設定
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::Normal)] = [this]() { NormalUpdate(); };
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::RushAttack)] = [this]() { RushAttackUpdate(); };
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::ShotAttack)] = [this]() { ShotAttackUpdate(); };
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::IceFallAttack)] = [this]() { IceFallAttackUpdate(); };

	// 各振る舞いのリセット処理を設定する
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::Normal)] = [this]() {ResetNormal(); };
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::RushAttack)] = [this]() {ResetRush(); };
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::ShotAttack)] = [this]() {};
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::IceFallAttack)] = [this]() {ResetIceFall();};

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();	
#endif
	// 値を適応させる
	ApplyDebugParam();
}

void BossStateBattle::Enter() {
	// 初期状態を設定
	currentBehavior_ = ButtleBehavior::Normal;
	resetBehaviorParamTable_[static_cast<size_t>(currentBehavior_)]();
	// リクエストをリセット
	behaviorRequest_ = std::nullopt;
	isCurrentBehaviorActive_ = true;
}

void BossStateBattle::Update() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

	// 振る舞いを管理
	ControllBehavior();

	// 状態遷移のリクエストがあった場合、切り替える処理
	if (behaviorRequest_) {
		// 状態を変更
		currentBehavior_ = behaviorRequest_.value();
		// 振る舞いによるリセットを呼び出す
		resetBehaviorParamTable_[static_cast<size_t>(currentBehavior_)]();
		// 振る舞いのリクエストをクリア
		behaviorRequest_ = std::nullopt;
		// 現在の振る舞いを有効化する
		isCurrentBehaviorActive_ = true;
	}

	// 指定した状態による更新処理をおこなう
	behaviorsTable_[static_cast<size_t>(currentBehavior_)]();
}

void BossStateBattle::Exit() {

}

void BossStateBattle::ControllBehavior() {
	// 現在の振る舞いが有効な場合は早期リターン
	if (isCurrentBehaviorActive_) { return; }

	// 距離などによって振る舞いを判断する

	// 次に切り替える状態のリクエストをおこなう
	behaviorRequest_ = ButtleBehavior::Normal;

	behaviorRequest_ = ButtleBehavior::IceFallAttack;
}

void BossStateBattle::ResetNormal() {
	// 円の中心から自分へのベクトルを求める
	Vector3 myDir = Normalize(Vector3(bossContext_.worldTransform->transform_.translate.x, 0.0f, bossContext_.worldTransform->transform_.translate.z));
	startDir_ = myDir;
	endDir_ = myDir * -1.0f;
	myDir = myDir * stageRadius_;

	// 戻る位置の始点と終点を決める
	startBackPos_ = bossContext_.worldTransform->transform_.translate;
	endBackPos_ = myDir;
	endBackPos_.y = defalutPosY_;

	// 時間をリセット
	backTimer_ = 0.0f;
}

void BossStateBattle::NormalUpdate() {

	// 元の場所に戻る
	backTimer_ += FpsCounter::deltaTime / backMaxTime_;
	Vector3 tmpPos = Lerp(startBackPos_, endBackPos_, EaseInBack(backTimer_));

	// 移動処理
	bossContext_.worldTransform->transform_.translate = tmpPos;

	// 回転
	Vector3 dir = Slerp(startDir_, endDir_, backTimer_);
	// Y軸周りの角度
	bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

	// 元の場所に戻る処理を終了
	if (backTimer_ >= 1.0f) {
		// 振る舞いの切り替えをリクエスト
		if (tmpIndex == 0) {
			behaviorRequest_ = ButtleBehavior::IceFallAttack;
			tmpIndex++;
		} else {
			behaviorRequest_ = ButtleBehavior::RushAttack;
			tmpIndex = 0;
		}
		
	}
}

void BossStateBattle::ResetRush() {
	// 円の中心からプレイヤーへのベクトルを求める
	Vector3 tmpTarget = Normalize(Vector3(bossContext_.targetPos.x, 0.0f, bossContext_.targetPos.z));
	Vector3 targetDir = tmpTarget;
	// プレイヤーが中心位置にいる時は適当な位置を返す
	if (Length(tmpTarget) < 0.001f) {targetDir = Vector3(0, 0, 1);}
	
	// 突進の終了する位置を設定
	float tmpAngle = std::atan2f(targetDir.z, targetDir.x);
	endRushPos_ = {std::cosf(tmpAngle)* (stageRadius_ + offsetEndRush_),0.0f,std::sinf(tmpAngle)* (stageRadius_ + offsetEndRush_) };
	
	// 反転する
	targetDir = targetDir * -1.0f;
	// 反対側の角度を求める
	endAngle_ = std::atan2f(targetDir.z, targetDir.x);

	// 円の中心から自分へのベクトルを求める
	Vector3 myDir = Normalize(Vector3(bossContext_.worldTransform->transform_.translate.x, 0.0f, bossContext_.worldTransform->transform_.translate.z));
	// 最初の角度を求める
	startAngle_ = std::atan2f(myDir.z, myDir.x);

	// 回る時間を求める
	if (startAngle_ == endAngle_) {
		rotMoveTimer_ = 1.0f;
		rotMaxMoveTime_ = 0.0f;
	} else {
		rotMaxMoveTime_ = GetMoveTimeDistance(startAngle_, endAngle_, stageRadius_, rotSpeed_);
	}

	// 時間をリセット
	rotMoveTimer_ = 0.0f;
	fallTimer_ = 0.0f;
	rushTimer_ = 0.0f;

	// 回転から始まるようにする
	isRotMove_ = true;

	// 最初の回転各
	float rot = LerpShortAngle(startAngle_, endAngle_, EaseInOut(0.2f));
	Vector3 prePos = { std::cosf(rot) * stageRadius_, bossContext_.worldTransform->transform_.translate.y,std::sinf(rot) * stageRadius_ };
	startRotEndDir_ = Normalize(prePos - bossContext_.worldTransform->transform_.translate);

	// 最初の回転するための角度を求める
	Vector3 tDir = Normalize(Vector3(-bossContext_.worldTransform->transform_.translate.x, 0.0f, -bossContext_.worldTransform->transform_.translate.z));
	startDir_ = tDir;

	// 最後の回転するための最初の角度を求める
	rot = LerpShortAngle(startAngle_, endAngle_, EaseInOut(1.0f));
	prePos = { std::cosf(rot) * stageRadius_, bossContext_.worldTransform->transform_.translate.y,std::sinf(rot) * stageRadius_ };
	endDir_ = Normalize(prePos*-1.0f);

	// 上下移動する回数を求める
	float angleDiff = std::fabs(endAngle_ - startAngle_);
	cycleCount_ = angleDiff / 0.4f;
}

void BossStateBattle::RushAttackUpdate() {

	if (isRotMove_) {
		// 移動する
		if (rotMaxMoveTime_ >= 0.0001f) {
			rotMoveTimer_ += FpsCounter::deltaTime / rotMaxMoveTime_;
		} else {
			rotMoveTimer_ = 1.0f;
		}

		// 移動
#pragma region Move

		// 縦移動
		float posY = 0.0f;
		float totalCycle = rotMoveTimer_ * cycleCount_;
		float localTimer = std::fmodf(totalCycle,1.0f);

		if (localTimer <= 0.5f) {
			float t = localTimer / 0.5f;
			posY = Lerp(defalutPosY_, defalutPosY_ + 2.0f, EaseInOut(t));
		} else {
			float t = (localTimer - 0.5f) / 0.5f;
			posY = Lerp(defalutPosY_ + 2.0f, defalutPosY_, EaseInOut(t));
		}

		// 角度補間する
		float angle = LerpShortAngle(startAngle_, endAngle_, EaseInOut(rotMoveTimer_));
		// 位置を求める
		Vector3 pos = {std::cosf(angle) * stageRadius_, posY,std::sinf(angle) * stageRadius_};

		// 移動
		bossContext_.worldTransform->transform_.translate = pos;
#pragma endregion

		// 回転移動
#pragma region Rotate
		// 回転の処理
		if (rotMoveTimer_ <= 0.2f) {
			float localT = rotMoveTimer_ / 0.2f;
			// 回転
			Vector3 dir = Slerp(startDir_, startRotEndDir_, EaseIn(localT));
			// Y軸周りの角度
			bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

		} else if (rotMoveTimer_ <= 0.8f) {
			// 進行方向に向ける
			float rot = LerpShortAngle(startAngle_, endAngle_, EaseInOut(rotMoveTimer_ + FpsCounter::deltaTime / rotMaxMoveTime_));
			Vector3 prePos = { std::cosf(rot) * stageRadius_, bossContext_.worldTransform->transform_.translate.y,std::sinf(rot) * stageRadius_ };
			Vector3 dir = Normalize(prePos - pos);
			// Y軸周りの角度
			bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);
			// 保存
			endRotStartDir_ = dir;
		} else {

			float localT = (rotMoveTimer_ - 0.8f) / 0.2f;
			// 回転
			Vector3 dir = Slerp(endRotStartDir_, endDir_, EaseOut(localT));
			// Y軸周りの角度
			bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);
		}
#pragma endregion

		// 回転移動が終了
		if (rotMoveTimer_ >= 1.0f) {
			isRotMove_ = false;
			// 突進の最初の位置を設定
			startRushPos_ = bossContext_.worldTransform->transform_.translate;
		}
	} else {

		// y軸の動き
		float tmpPosY = size_.y * 0.5f;
		if (fallTimer_ <= 1.0f) {
			fallTimer_ += FpsCounter::deltaTime / ((rushMaxTime_ * 3.0f) / 3.0f);
			tmpPosY = Lerp(startRushPos_.y, size_.y * 0.5f, EaseOutQuart(fallTimer_));
		}

		// 突進移動
		rushTimer_ += FpsCounter::deltaTime / rushMaxTime_;
		Vector3 tmpPos = Lerp(startRushPos_, endRushPos_, EaseIn(rushTimer_));

		// 移動処理
		bossContext_.worldTransform->transform_.translate = { tmpPos.x,tmpPosY,tmpPos.z };

		// 突進終了
		if (rushTimer_ >= 1.0f) {
			// 振る舞いの切り替えをリクエスト
			behaviorRequest_ = ButtleBehavior::Normal;
		}
	}	
}

void BossStateBattle::ShotAttackUpdate() {

}

void BossStateBattle::ResetIceFall() {
	waitTimer_ = 0.0f;
	bossContext_.isActiveIceFall = false;
	isActiveIceFall_ = false;

	// 円の中心から自分へのベクトルを求める
	Vector3 myDir = Normalize(Vector3(bossContext_.worldTransform->transform_.translate.x, 0.0f, bossContext_.worldTransform->transform_.translate.z));
	startDir_ = myDir;
	endDir_ = myDir * -1.0f;

	// リセットする
	rotateTimer_ = 0.0f;
}

void BossStateBattle::IceFallAttackUpdate() {


	if (rotateTimer_ <= 1.0f) {

		rotateTimer_ += FpsCounter::deltaTime / maxRotateTime_;

		// 回転
		Vector3 dir = Slerp(startDir_, endDir_, backTimer_);
		// Y軸周りの角度
		bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

	} else {

		waitTimer_ += FpsCounter::deltaTime / maxWaitTime_;

		if (waitTimer_ >= 0.2f) {
			if (!isActiveIceFall_) {
				bossContext_.isActiveIceFall = true;
				isActiveIceFall_ = true;
			} else {
				// 一度発射したらfalseにする
				if (bossContext_.isActiveIceFall) {
					bossContext_.isActiveIceFall = false;
				}
			}
		}

		// 待機の終了
		if (waitTimer_ >= 1.0f) {
			// 振る舞いの切り替えをリクエスト
			behaviorRequest_ = ButtleBehavior::Normal;
		}
	}
}

void BossStateBattle::RegisterBebugParam() {

}

void BossStateBattle::ApplyDebugParam() {

}

namespace {

	float LerpShortAngle(float a, float b, float t) {
		float diff = b - a;

		// -2pi-2piに補正する
		diff = std::fmodf(diff, std::numbers::pi_v<float> *2.0f);
		// -pi-piに補正する
		if (diff < -std::numbers::pi_v<float>) {
			diff += std::numbers::pi_v<float> *2.0f;
		} else if (diff > std::numbers::pi_v<float>) {
			diff -= std::numbers::pi_v<float> *2.0f;
		}

		return a + diff * t;
	}

	float GetAngleDiff(float a, float b) {
		float diff = b - a;
		if (std::fabsf(diff) < 1.0e-4f) { return 0.0f; }
		// -2pi-2piに補正する
		diff = std::fmodf(diff, std::numbers::pi_v<float> *2.0f);
		// -pi-piに補正する
		if (diff < -std::numbers::pi_v<float>) {
			diff += std::numbers::pi_v<float> *2.0f;
		} else if (diff > std::numbers::pi_v<float>) {
			diff -= std::numbers::pi_v<float> *2.0f;
		}
		return diff;
	}

	float GetMoveTimeDistance(float startAngle, float endAngle, float radius, float speed) {
		float diff = GetAngleDiff(startAngle, endAngle);
		if (diff == 0.0f) { return 0.0f; }
		float distance = std::fabsf(diff) * radius;   // 弧の長さ
		float time = distance / speed;
		return time;
	}

	float EaseOutQuart(float t) {
		return 1 - std::powf(1 - t, 3);
	}

	float EaseInBack(float t) {
		const float c1 = 1.70158f;
		const float c3 = c1 + 1;
		return c3 * t * t * t - c1 * t * t;
	}
}