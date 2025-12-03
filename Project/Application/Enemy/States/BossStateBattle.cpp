#define NOMINMAX
#include"BossStateBattle.h"
#include"Application/Enemy/BossState.h"
#include"MyMath.h"
#include"FPSCounter.h"
#include"EasingManager.h"
#include"RandomGenerator.h"
#include"GameParamEditor.h"
#include"LogManager.h"
using namespace GameEngine;

BossStateBattle::BossStateBattle(BossContext& context, const float& stageRadius, GameEngine::DebugRenderer* debugRenderer) : bossContext_(context) {

	debugRenderer_ = debugRenderer;

	// ステージの半径を取得
	stageRadius_ = stageRadius;

	// 各振る舞いの更新処理を設定
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::Normal)] = [this]() { NormalUpdate(); };
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::RushAttack)] = [this]() { RushAttackUpdate(); };
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::WindAttack)] = [this]() { WindAttackUpdate(); };
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::IceFallAttack)] = [this]() { IceFallAttackUpdate(); };
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::Wait)] = [this]() { WaitUpdate(); };

	// 各振る舞いのリセット処理を設定する
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::Normal)] = [this]() {ResetNormal(); };
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::RushAttack)] = [this]() {ResetRush(); };
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::WindAttack)] = [this]() {ResetWind(); };
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::IceFallAttack)] = [this]() {ResetIceFall();};
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::Wait)] = [this]() {ResetWait(); };

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

	// アニメーション
	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);
	bossContext_.animationTimer = 0.0f;
}

void BossStateBattle::NormalUpdate() {

	// 元の場所に戻る
	backTimer_ += FpsCounter::deltaTime / backMaxTime_;
	Vector3 tmpPos = Lerp(startBackPos_, endBackPos_, EaseInBack(backTimer_));

	bossContext_.animationTimer = backTimer_;

	// 移動処理
	bossContext_.worldTransform->transform_.translate = tmpPos;

	// 回転
	Vector3 dir = Slerp(startDir_, endDir_, backTimer_);
	// Y軸周りの角度
	bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

	// 元の場所に戻る処理を終了
	if (backTimer_ >= 1.0f) {
		// 振る舞いの切り替えをリクエスト
		//behaviorRequest_ = ButtleBehavior::IceFallAttack;
		behaviorRequest_ = ButtleBehavior::WindAttack;
		/*if (tmpIndex == 0) {
			
			tmpIndex++;
		} else {
			behaviorRequest_ = ButtleBehavior::RushAttack;
			tmpIndex = 0;
		}*/	
	}
}

void BossStateBattle::ResetRush() {
	
	// 移動するための角度を求める
#pragma region MakeRotate
	// 円の中心からプレイヤーへのベクトルを求める
	Vector3 tmpTarget = Normalize(Vector3(bossContext_.targetPos.x, 0.0f, bossContext_.targetPos.z));
	Vector3 targetDir = tmpTarget;
	// プレイヤーが中心位置にいる時は適当な位置を返す
	if (Length(tmpTarget) < 0.001f) {targetDir = Vector3(0, 0, 1);}
	
	// 突進の終了する位置を設定
	float tmpAngle = std::atan2f(targetDir.z, targetDir.x);
	endRushPos_ = {std::cosf(tmpAngle)* (stageRadius_ + offsetEndRush_),0.0f,std::sinf(tmpAngle)* (stageRadius_ + offsetEndRush_) };
	
	// 移動する角度
	float startAngle = 0.0f;
	float endAngle = 0.0f;
	// 反転する
	targetDir = targetDir * -1.0f;
	// 反対側の角度を求める
	endAngle = std::atan2f(targetDir.z, targetDir.x);

	// 円の中心から自分へのベクトルを求める
	Vector3 myDir = Normalize(Vector3(bossContext_.worldTransform->transform_.translate.x, 0.0f, bossContext_.worldTransform->transform_.translate.z));
	// 最初の角度を求める
	startAngle = std::atan2f(myDir.z, myDir.x);
#pragma endregion

	// 回転移動の位置と時間を求める
#pragma region MakeRotateMovePos
	float maxR = 0.0f;

	// 回る時間を求める
	if (startAngle == endAngle) {
		rotMoveTimer_ = 1.0f;
		rotMaxMoveTime_ = 0.0f;
	} else {
		rotMaxMoveTime_ = GetMoveTimeDistance(startAngle, endAngle, stageRadius_, rotSpeed_);
		maxR = rotMaxMoveTime_ * rotMaxMoveTime_;
	}

	float r = 0.0f;
	controlPoints_.clear();
	controlPoints_.push_back(bossContext_.worldTransform->transform_.translate);
	float t = 0.0f;

	while (t <= 1.0f)
	{
		if (t <= 0.8f) {
			t += 0.02f;
		} else {
			t += 0.01f;
		}

		// 角度補間する
		float angle = LerpShortAngle(startAngle, endAngle, t);

		if (t >= 0.4f && t < 0.8f) {

			float localT = (t - 0.4f) / 0.4f;
			r = Lerp(0.0f, maxR, EaseIn(localT));
		} else if (t >= 0.8f) {
			float localT = (t - 0.8f) / 0.2f;
			r = Lerp(maxR, 0.0f, EaseIn(localT));
		}

		// 位置を求める
		Vector3 pos = { std::cosf(angle) * (stageRadius_ + r), bossContext_.worldTransform->transform_.translate.y + r * 0.2f,std::sinf(angle) * (stageRadius_ + r) };

		controlPoints_.push_back(pos);
	}
#pragma endregion

	// 時間をリセット
	rotMoveTimer_ = 0.0f;
	fallTimer_ = 0.0f;
	rushTimer_ = 0.0f;

	/// 自分自体の回転要素

	// 回転から始まるようにする
	isRotMove_ = true;

	// 最初の回転各
	float rot = LerpShortAngle(startAngle, endAngle, EaseInOut(0.2f));
	Vector3 prePos = { std::cosf(rot) * stageRadius_, bossContext_.worldTransform->transform_.translate.y,std::sinf(rot) * stageRadius_ };
	startRotEndDir_ = Normalize(prePos - bossContext_.worldTransform->transform_.translate);

	// 最初の回転するための角度を求める
	startDir_ = Normalize(Vector3(-bossContext_.worldTransform->transform_.translate.x, 0.0f, -bossContext_.worldTransform->transform_.translate.z));

	// 最後の回転するための最初の角度を求める
	rot = LerpShortAngle(startAngle, endAngle, EaseInOut(1.0f));
	prePos = { std::cosf(rot) * stageRadius_, bossContext_.worldTransform->transform_.translate.y,std::sinf(rot) * stageRadius_ };
	endDir_ = Normalize(prePos*-1.0f);

	// 上下移動する回数を求める
	float angleDiff = std::fabs(Length(CatmullRomPosition(controlPoints_, 1.0f) - CatmullRomPosition(controlPoints_, 0.0f)));
	cycleCount_ = angleDiff / (stageRadius_ * 0.5f);

	// 突進前の挙動を取得
	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Rush)]["Rush_Prepare"]);
	bossContext_.animationTimer = 0.0f;

	isMidAnimation_ = false;
	isEndANimation_ = false;
}

void BossStateBattle::RushAttackUpdate() {

	// 移動する位置を表示
#ifdef _DEBUG
	CreateCatmullRom();
#endif

	if (isRotMove_) {
		// 移動する
		rotMoveTimer_ += FpsCounter::deltaTime / rotMaxMoveTime_;

		// 移動
#pragma region Move

		// 縦移動
		float posY = 0.0f;
		float totalCycle = rotMoveTimer_ * cycleCount_;
		float localTimer = std::fmodf(totalCycle,1.0f);

		if (localTimer <= 0.5f) {
			float t = localTimer / 0.5f;
			posY = Lerp(0.0f, 2.0f, EaseInOut(t));
		} else {
			float t = (localTimer - 0.5f) / 0.5f;
			posY = Lerp(2.0f, 0.0f, EaseInOut(t));
		}

		rotMoveTimer_ = std::min(rotMoveTimer_, 1.0f);
		Vector3 pos = CatmullRomPosition(controlPoints_, EaseInOut(rotMoveTimer_));

		// 移動
		bossContext_.worldTransform->transform_.translate = pos;
		bossContext_.worldTransform->transform_.translate.y += posY;
#pragma endregion

		// 回転移動
#pragma region Rotate
		// 計算用の進行方向ベクトル
		Vector3 dir = { 0,0,1 }; 
		float tiltPower = 0.0f;

		// 回転の処理
		if (rotMoveTimer_ <= 0.2f) {
			float localT = rotMoveTimer_ / 0.2f;

			// 回転
			dir = Slerp(startDir_, startRotEndDir_, EaseIn(localT));
			// Y軸周りの角度
			bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

			// 徐々に傾ける
			tiltPower = EaseIn(localT);

		} else if (rotMoveTimer_ <= 0.8f) {
			bossContext_.worldTransform->transform_.rotate.z = 0.2f;
			// 進行方向に向ける
			Vector3 prePos = CatmullRomPosition(controlPoints_, EaseInOut(rotMoveTimer_ + FpsCounter::deltaTime / rotMaxMoveTime_));
			dir = Normalize(prePos - pos);
			// Y軸周りの角度
			bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);
			// 最大まで傾ける
			tiltPower = 1.0f;
			// 保存
			endRotStartDir_ = dir;
		} else {

			float localT = (rotMoveTimer_ - 0.8f) / 0.2f;
			// 回転
			dir = Slerp(endRotStartDir_, endDir_, EaseOut(localT));
			// Y軸周りの角度
			bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

			// 徐々に傾きを戻す
			tiltPower = 1.0f - EaseOut(localT);
		}

		// 傾ける角度を求める
		Vector3 rightVector = Normalize(Cross(Vector3(0.0f, 1.0f, 0.0f), dir));
		Vector3 toCenter = Normalize(bossContext_.worldTransform->transform_.translate * -1.0f);
		// 内積を使って中心が左右どちらにあるか判定
		float side = Dot(rightVector, toCenter);
		// 傾きを適用
		targetTilt_ = (side > 0.0f ? -maxTiltAngle_ : maxTiltAngle_);

		// 傾きを適用
		bossContext_.worldTransform->transform_.rotate.z = targetTilt_ * tiltPower;
#pragma endregion

		// 回転移動が終了
		if (rotMoveTimer_ >= 1.0f) {
			//bossContext_.worldTransform->transform_.rotate.z = 0.0f;
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

	// アニメーション処理
#pragma region RushAnimation

	if (isMidAnimation_) {

		if (isEndANimation_) {

			// 突進行動
			if (rushTimer_ <= 0.8f) {
				if (bossContext_.animationTimer <= 0.8f) {
					bossContext_.animationTimer += FpsCounter::deltaTime / rushMaxTime_;
					bossContext_.animationTimer = std::min(bossContext_.animationTimer, 1.0f);
				}
			} else {
				if (bossContext_.animationTimer <= 1.0f) {
					bossContext_.animationTimer += FpsCounter::deltaTime / rushMaxTime_;
					bossContext_.animationTimer = std::min(bossContext_.animationTimer, 1.0f);
				}
			}
		} else {
			// 中間
			bossContext_.animationTimer += FpsCounter::deltaTime / 0.2f;
			
			// 突進行動に移行
			if (bossContext_.animationTimer >= 1.0f) {
				isEndANimation_ = true;
				bossContext_.animationTimer = 0.0f;
				AnimationData animation = (*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Rush)]["Rush_End"];
				bossContext_.animationMaxTime = animation.duration;
				bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Rush)]["Rush_End"]);
			}
		}
	} else {

		// 回転するまでの動き
		bossContext_.animationTimer += FpsCounter::deltaTime / 0.5f;

		// 中間動作に移行
		if (bossContext_.animationTimer >= 1.0f) {
			isMidAnimation_ = true;
			bossContext_.animationTimer = 0.0f;
			bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Rush)]["Rush_Main"]);
		}
	}
#pragma endregion
}

void BossStateBattle::ResetWind() {
	// 時間をリセット
	windTimer_ = 0.0f;
	bossContext_.isWindAttack_ = false;
	isActiveWind_ = false;

	// 円の中心から自分へのベクトルを求める
	Vector3 myDir = Normalize(Vector3(bossContext_.worldTransform->transform_.translate.x, 0.0f, bossContext_.worldTransform->transform_.translate.z));
	startDir_ = myDir;
	endDir_ = myDir * -1.0f;

	// リセットする
	rotateTimer_ = 0.0f;
}

void BossStateBattle::WindAttackUpdate() {

	if (rotateTimer_ <= 1.0f) {

		rotateTimer_ += FpsCounter::deltaTime / maxRotateTime_;

		// 回転
		Vector3 dir = Slerp(startDir_, endDir_, rotateTimer_);
		// Y軸周りの角度
		bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

	} else {
		if (!isActiveWind_) {
			bossContext_.isWindAttack_ = true;
			isActiveWind_ = true;
		} else {
			// 一度発射したらfalseにする
			if (bossContext_.isWindAttack_) {
				bossContext_.isWindAttack_ = false;
			}
		}

		windTimer_ += FpsCounter::deltaTime / maxWaitTime_;

		// 待機の終了
		if (windTimer_ >= 1.0f) {
			// 振る舞いの切り替えをリクエスト
			behaviorRequest_ = ButtleBehavior::Normal;
		}
	}
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

	// アニメーション
	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);
	bossContext_.animationTimer = 0.0f;
}

void BossStateBattle::IceFallAttackUpdate() {

	if (rotateTimer_ <= 1.0f) {

		rotateTimer_ += FpsCounter::deltaTime / maxRotateTime_;
		bossContext_.animationTimer = rotateTimer_;

		// 回転
		Vector3 dir = Slerp(startDir_, endDir_, rotateTimer_);
		// Y軸周りの角度
		bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

	} else {
		if (!isActiveIceFall_) {
			bossContext_.isActiveIceFall = true;
			isActiveIceFall_ = true;

			//　アニメーション
			bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Scream)]["Scream"]);
			bossContext_.animationTimer = 0.0f;
		} else {
			// 一度発射したらfalseにする
			if (bossContext_.isActiveIceFall) {
				bossContext_.isActiveIceFall = false;
			}
		}

		waitTimer_ += FpsCounter::deltaTime / maxWaitTime_;
		bossContext_.animationTimer = waitTimer_;

		// 待機の終了
		if (waitTimer_ >= 1.0f) {
			// 振る舞いの切り替えをリクエスト
			behaviorRequest_ = ButtleBehavior::Wait;
			bossContext_.animationTimer = 1.0f;
		}
	}
}

void BossStateBattle::ResetWait() {
	moveWaitTimer_ = 0.0f;

	// アニメーション

	// 基本移動の最大は1.3秒
	//bossContext_.animationMaxTime = 1.3f;
	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);
	bossContext_.animationTimer = 0.0f;
}

void BossStateBattle::WaitUpdate() {

	//Log("WaitPhase");

	moveWaitTimer_ += FpsCounter::deltaTime / maxMoveWaitTime_;
	bossContext_.animationTimer = moveWaitTimer_;

	// 縦移動
	float posY = 0.0f;
	float totalCycle = moveWaitTimer_ * 3.0f;
	float localTimer = std::fmodf(totalCycle, 1.0f);

	if (localTimer <= 0.5f) {
		float t = localTimer / 0.5f;
		posY = Lerp(defalutPosY_, defalutPosY_ + 2.0f, EaseInOut(t));
	} else {
		float t = (localTimer - 0.5f) / 0.5f;
		posY = Lerp(defalutPosY_ + 2.0f, defalutPosY_, EaseInOut(t));
	}

	// 移動
	bossContext_.worldTransform->transform_.translate.y = posY;
	
	// 待機行動の終了
	if (moveWaitTimer_ >= 1.0f) {
		behaviorRequest_ = ButtleBehavior::RushAttack;
	}
}

void BossStateBattle::RegisterBebugParam() {
	// 突進攻撃
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "RushTime", rushMaxTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "OffestEndRush", offsetEndRush_);

	// 氷柱攻撃
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "IceFallTime", maxWaitTime_);
}

void BossStateBattle::ApplyDebugParam() {
	// 突進攻撃
	rushMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "RushTime");
	offsetEndRush_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "OffestEndRush");

	// 氷柱攻撃
	maxWaitTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "IceFallTime");
}

void BossStateBattle::Setup(uint32_t sampleCount) {
	lookupTable.clear();
	totalLength = 0.0f;
	if (controlPoints_.size() < 4) return;

	// 始点を登録
	lookupTable.push_back({ 0.0f, 0.0f });

	Vector3 prevPos = CatmullRomPosition(controlPoints_, 0.0f);

	// 曲線全体の距離を求める
	for (uint32_t i = 1; i <= sampleCount; ++i) {
		float t = static_cast<float>(i) / static_cast<float>(sampleCount);
		Vector3 currentPos = CatmullRomPosition(controlPoints_, t);

		// ベクトルの長さ
		float segmentDist = Length(currentPos - prevPos); 
		totalLength += segmentDist;

		lookupTable.push_back({ totalLength, t });
		prevPos = currentPos;
	}
}

Vector3 BossStateBattle::GetPositionUniform(float t) const {
	if (controlPoints_.empty()) return Vector3();

	t = std::clamp(t, 0.0f, 1.0f);

	// 欲しい距離を計算
	float targetDistance = t * totalLength;

	// 二分探索で、目標の距離を超える最初の点を探す
	auto it = std::upper_bound(lookupTable.begin(), lookupTable.end(), targetDistance,
		[](float val, const SamplePoint& p) { return val < p.distance; });

	// 補間してrawTを求める
	float rawT = 0.0f;

	if (it == lookupTable.begin()) {
		rawT = 0.0f;
	} else if (it == lookupTable.end()) {
		rawT = 1.0f;
	} else {
		const SamplePoint& pNext = *it;
		const SamplePoint& pPre = *std::prev(it);

		// 2点間の距離の割合から、rawTを線形補間する
		float distRatio = (targetDistance - pPre.distance) / (pNext.distance - pPre.distance);
		rawT = pPre.rawT + (pNext.rawT - pPre.rawT) * distRatio;
	}

	return CatmullRomPosition(controlPoints_, rawT);
}

void BossStateBattle::CreateCatmullRom() {
	// 線分で描画する用の頂点リスト
	std::vector<Vector3> pointDrawing;
	// 線分の数+1個分の頂点座標を計算
	for (size_t i = 0; i < 100 + 1; ++i) {
		float t = (1.0f / 100) * i;
		float t1 = (1.0f / 100) * (i + 1);
		Vector3 pos = CatmullRomPosition(controlPoints_, t);
		Vector3 nextpos = CatmullRomPosition(controlPoints_, t1);
		
		debugRenderer_->AddLine(pos, nextpos,{1.0f,0.0f,0.0f,1.0f});
	}
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