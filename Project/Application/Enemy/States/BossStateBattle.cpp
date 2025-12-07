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
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::RotateMove)] = [this]() { RotateMoveUpdate(); };
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::CrossMove)] = [this]() { CrossMoveUpdate(); };
	behaviorsTable_[static_cast<size_t>(ButtleBehavior::InMove)] = [this]() { InMoveUpdate(); };

	// 各振る舞いのリセット処理を設定する
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::Normal)] = [this]() {ResetNormal(); };
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::RushAttack)] = [this]() {ResetRush(); };
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::WindAttack)] = [this]() {ResetWind(); };
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::IceFallAttack)] = [this]() {ResetIceFall();};
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::Wait)] = [this]() {ResetWait(); };
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::RotateMove)] = [this]() {ResetRotateMove(); };
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::CrossMove)] = [this]() {ResetCrossMove(); };
	resetBehaviorParamTable_[static_cast<size_t>(ButtleBehavior::InMove)] = [this]() {ResetInMove(); };

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
#endif
	// 値を適応させる
	ApplyDebugParam();

	// 行動のリスト
	lotteryList_ = {
			{ ButtleBehavior::RushAttack,    rushAttackWeight_ }, // 突進
			{ ButtleBehavior::WindAttack,    WindAttackWeight_ }, // 風
			{ ButtleBehavior::IceFallAttack, IceFallWeight_ }, // 氷柱
			{ ButtleBehavior::Wait,          WaitWeight_  }, // 待機
			{ ButtleBehavior::RotateMove,    RotateMoveWeight_ },  // 回転移動
			{ ButtleBehavior::CrossMove,     CrossMoveWeight_}   // 横断移動
	};
}

void BossStateBattle::Enter() {
	// 初期状態を設定
	currentBehavior_ = ButtleBehavior::InMove;
	resetBehaviorParamTable_[static_cast<size_t>(currentBehavior_)]();
	// リクエストをリセット
	behaviorRequest_ = std::nullopt;
}

void BossStateBattle::Update() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

	// 状態遷移のリクエストがあった場合、切り替える処理
	if (behaviorRequest_) {
		// 状態を変更
		currentBehavior_ = behaviorRequest_.value();
		// 振る舞いによるリセットを呼び出す
		resetBehaviorParamTable_[static_cast<size_t>(currentBehavior_)]();
		// 振る舞いのリクエストをクリア
		behaviorRequest_ = std::nullopt;
	}

	// 指定した状態による更新処理をおこなう
	behaviorsTable_[static_cast<size_t>(currentBehavior_)]();
}

void BossStateBattle::Exit() {

}

void BossStateBattle::ResetNormal() {

	// 氷柱がステージに存在してる場合は処理をおこなわない
	std::vector<BehaviorWeight> list;
	list.resize(lotteryList_.size());
	if (bossContext_.iceFallCount != 0) {
		for (size_t i = 0; i < lotteryList_.size(); ++i) {
			if (lotteryList_[i].behavior != ButtleBehavior::IceFallAttack) {
				list.push_back(lotteryList_[i]);
			}
		}
	} else {
		list = lotteryList_;
	}

	// 全体の重みを計算する
	int32_t totalWeight = 0;
	for (const auto& item : list) {
		totalWeight += item.weight;
	}

	int32_t randomValue = RandomGenerator::Get<int32_t>(0, totalWeight - 1);

	for (const auto& item : list) {
		if (randomValue < item.weight) {
			selectButtleBehavior_ = item.behavior;
			break;
		}
		// 次の範囲へ進むために値を引く
		randomValue -= item.weight;
	}

	//selectButtleBehavior_ = ButtleBehavior::WindAttack;

	//Log("randValue : " + std::to_string(randomValue));

	// 突進が選ばれた場合に距離が短過ぎる場合は移動させる
	if (selectButtleBehavior_ == ButtleBehavior::RushAttack) {
		// 円の中心からプレイヤーへのベクトルを求める
		Vector3 tmpTarget = Normalize(Vector3(bossContext_.targetPos.x, 0.0f, bossContext_.targetPos.z));
		Vector3 targetDir = tmpTarget;

		// 反転する
		targetDir = targetDir * -1.0f;
		// 反対側の角度を求める
		float endAngle = std::atan2f(targetDir.z, targetDir.x);

		// プレイヤーの一番後ろの位置
		Vector3 targetPos = { std::cosf(endAngle) * (stageRadius_), 0.0f,std::sinf(endAngle) * (stageRadius_) };

		float length = Length(targetPos - bossContext_.worldTransform->transform_.translate);

		// 距離が近い場合は離れる行動をとる用にする
		if (length <= stageRadius_ * 0.2f) {
			if (RandomGenerator::Get(0, 1) == 0) {
				selectButtleBehavior_ = ButtleBehavior::CrossMove;
			} else {
				selectButtleBehavior_ = ButtleBehavior::RotateMove;
			}
		}
	}

	// 選択した遷移
	//selectButtleBehavior_ = ButtleBehavior::RushAttack;
	//selectButtleBehavior_ = ButtleBehavior::WindAttack;
	selectButtleBehavior_ = ButtleBehavior::IceFallAttack;
}

void BossStateBattle::NormalUpdate() {

	// 遷移する
	behaviorRequest_ = selectButtleBehavior_;
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
	
	// 突進の終わりの戻る位置
	rushOutEndPos_ = { std::cosf(tmpAngle) * (stageRadius_ * rushEndRatio_),defalutPosY_,std::sinf(tmpAngle) * (stageRadius_ * rushEndRatio_) };

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

	rushTimer_ = 0.0f;

	// 回る時間を求める
	if (startAngle == endAngle) {
		rushTimer_ = 1.0f;
		rushInTime_ = 1.0f;
	} else {
		rushInTime_ = GetMoveTimeDistance(startAngle, endAngle, stageRadius_, rotSpeed_);
		maxR = rushInTime_ * rushInTime_;
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

	
	fallTimer_ = 0.0f;

	/// 自分自体の回転要素

	// ラッシュフェーズ
	rushPhase_ = RushPhase::In;

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

	bossContext_.isRushCollisionActive_ = false;

	bossContext_.isRushAttack_ = false;
}

void BossStateBattle::RushAttackUpdate() {

	// 移動する位置を表示
#ifdef _DEBUG
	CreateCatmullRom();
#endif

	switch (rushPhase_)
	{
	case BossStateBattle::RushPhase::In: {
#pragma region RushIn

		// 移動する
		rushTimer_ += FpsCounter::deltaTime / rushInTime_;
		// 移動
#pragma region Move

		// 縦移動
		float posY = 0.0f;
		float totalCycle = rushTimer_ * cycleCount_;
		float localTimer = std::fmodf(totalCycle, 1.0f);

		if (localTimer <= 0.5f) {
			float t = localTimer / 0.5f;
			posY = Lerp(0.0f, 2.0f, EaseInOut(t));
		} else {
			float t = (localTimer - 0.5f) / 0.5f;
			posY = Lerp(2.0f, 0.0f, EaseInOut(t));
		}

		rushTimer_ = std::min(rushTimer_, 1.0f);
		Vector3 pos = CatmullRomPosition(controlPoints_, EaseInOut(rushTimer_));

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
		if (rushTimer_ <= 0.2f) {
			float localT = rushTimer_ / 0.2f;

			// 回転
			dir = Slerp(startDir_, startRotEndDir_, EaseIn(localT));
			// Y軸周りの角度
			bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

			// 徐々に傾ける
			tiltPower = EaseIn(localT);

		} else if (rushTimer_ <= 0.8f) {
			bossContext_.worldTransform->transform_.rotate.z = 0.2f;
			// 進行方向に向ける
			Vector3 prePos = CatmullRomPosition(controlPoints_, EaseInOut(rushTimer_ + FpsCounter::deltaTime / rushInTime_));
			dir = Normalize(prePos - pos);
			// Y軸周りの角度
			bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);
			// 最大まで傾ける
			tiltPower = 1.0f;
			// 保存
			endRotStartDir_ = dir;
		} else {

			float localT = (rushTimer_ - 0.8f) / 0.2f;
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


		// アニメーション
		bossContext_.animationTimer += FpsCounter::deltaTime;
		bossContext_.animationTimer = std::fmodf(bossContext_.animationTimer, 1.0f);

		// 回転移動が終了
		if (rushTimer_ >= 1.0f) {
			// 突進の最初の位置を設定
			startRushPos_ = bossContext_.worldTransform->transform_.translate;
			// 突進のメインに移動
			rushPhase_ = RushPhase::Main;
			rushTimer_ = 0.0f;

			// アニメーション
			bossContext_.animationTimer = 0.0f;
			bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Rush)]["Rush_Main"]);

			// 突進する位置を求める
#pragma region SetRushPos
			// ボスの位置から突進の最後の位置へベクトル
			Vector3 myDir = Normalize(endRushPos_ - Vector3(bossContext_.worldTransform->transform_.translate.x, 0.0f, bossContext_.worldTransform->transform_.translate.z));

			 // 始点と終点の角度を求める
			float angle = std::numbers::pi_v<float> / 6.0f;
			float cos = std::cosf(angle);
			float sin = std::sinf(angle);
			Vector3 startDir = { myDir.x * cos - myDir.z * sin,0.0f,myDir.x * sin + myDir.z * cos };
			Vector3 endDir = { myDir.x * cos - myDir.z * -sin,0.0f,myDir.x * -sin + myDir.z * cos };

			// ボスの位置からターゲットの位置へのベクトル
			Vector3 targetDir = Normalize(Vector3(bossContext_.targetPos.x, 0.0f, bossContext_.targetPos.z) -
				Vector3(bossContext_.worldTransform->transform_.translate.x, 0.0f, bossContext_.worldTransform->transform_.translate.z));

			// 内積を求める
			float clampDot = Dot(myDir,endDir);
			float targetDot = Dot(myDir,targetDir);

			// 30度範囲におさまっている場合
			if (1.0f >= targetDot && targetDot >= clampDot) {
				// 突進の最後の位置を設定
				targetDir = Normalize(Vector3(bossContext_.targetPos.x, 0.0f, bossContext_.targetPos.z));
				float tmpAngle = std::atan2f(targetDir.z, targetDir.x);
				endRushPos_ = { std::cosf(tmpAngle) * (stageRadius_ + offsetEndRush_),0.0f,std::sinf(tmpAngle) * (stageRadius_ + offsetEndRush_) };

				// 向きを設定する
				Vector3 rotDir = endRushPos_ - startRushPos_;
				rotDir = Normalize(Vector3(rotDir.x, 0.0f, rotDir.z));
				bossContext_.worldTransform->transform_.rotate.y = std::atan2f(rotDir.x, rotDir.z);
			}
#pragma endregion
		}
#pragma endregion
		break;
	}

	case BossStateBattle::RushPhase::Main: {
#pragma region RushMain

		// y軸の動き
		float tmpPosY = size_.y * 0.5f;
		if (fallTimer_ <= 1.0f) {
			fallTimer_ += FpsCounter::deltaTime / ((rushMainTime_ * 3.0f) / 3.0f);
			tmpPosY = Lerp(startRushPos_.y, size_.y * 0.5f, EaseOutQuart(fallTimer_));
		}

		// 突進移動
		rushTimer_ += FpsCounter::deltaTime / rushMainTime_;
		Vector3 tmpPos = Lerp(startRushPos_, endRushPos_, EaseIn(rushTimer_));

		// 移動処理
		bossContext_.worldTransform->transform_.translate = { tmpPos.x,tmpPosY,tmpPos.z };

		// 速度を求める
		Vector3 next = Lerp(startRushPos_, endRushPos_, EaseIn(rushTimer_ + FpsCounter::deltaTime / rushMainTime_));
		bossContext_.rushVelocity = next - tmpPos;
		
		// アニメーション
		if (rushTimer_ <= 0.2f) {
			float localT = rushTimer_ / 0.2f;
			bossContext_.animationTimer = localT;
			bossContext_.isRushAttack_ = true;
		} else if (rushTimer_ <= 0.6f) {
			float localT = ((rushTimer_ - 0.2f) / 0.4f) * 0.8f;
			bossContext_.animationTimer = localT;
		} else if (rushTimer_ <= 0.9f) {
			float localT = ((rushTimer_ - 0.9f) / 0.1f) * 0.1f + 0.8f;
			bossContext_.animationTimer = localT;
		}

		// アニメーションを変更
		if (!isMidAnimation_) {
			if (rushTimer_ >= 0.2f) {
				//bossContext_.animationTimer = 0.0f;
				bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Rush)]["Rush_End"]);
				isMidAnimation_ = true;
			}
		}

		// 壁に対する当たり判定を有効にする
		if (rushTimer_ >= 0.5f) {
			bossContext_.isRushCollisionActive_ = true;
		}

		// 突進終了
		if (rushTimer_ >= 1.0f) {
			bossContext_.isRushAttack_ = false;
			bossContext_.isRushCollisionActive_ = false;
			// 突進のアウトに移行
			rushPhase_ = RushPhase::Out;
			rushTimer_ = 0.0f;
			// 最後の位置を取得
			rushOutStartPos_ = bossContext_.worldTransform->transform_.translate;

			// アニメーション
			bossContext_.animationTimer = 0.0f;
			bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);

			// 戻るための回転方向を求める
			Vector3 myDir = Normalize(Vector3(bossContext_.worldTransform->transform_.translate.x, 0.0f, bossContext_.worldTransform->transform_.translate.z));
			startDir_ = myDir;
			endDir_ = myDir * -1.0f;
		}
#pragma endregion
		break;
	}

	case BossStateBattle::RushPhase::Out: {
#pragma region RushOut

		rushTimer_ += FpsCounter::deltaTime / rushOutTime_;

		// 移動
		Vector3 tmpPos = Lerp(rushOutStartPos_, rushOutEndPos_, EaseIn(rushTimer_));
		bossContext_.worldTransform->transform_.translate = tmpPos;

		// 回転
		if (rushTimer_ <= 0.3f) {
			float localT = rushTimer_ / 0.3f;
			Vector3 dir = Slerp(startDir_, endDir_, localT);
			// Y軸周りの角度
			bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);
		}

		// アニメーション
		bossContext_.animationTimer += FpsCounter::deltaTime;
		bossContext_.animationTimer = std::fmodf(bossContext_.animationTimer, 1.0f);

		// 突進の終了
		if (rushTimer_ >= 1.0f) {
			// 振る舞いの切り替えをリクエスト
			behaviorRequest_ = ButtleBehavior::Wait;
		}
#pragma endregion
		break;
	}
	}
}

void BossStateBattle::ResetWind() {
	// 時間をリセット
	windTimer_ = 0.0f;
	bossContext_.isWindAttack_ = false;
	bossContext_.windMaxTime_ = windMainTime_;
	
	// 角度を求める
	Vector3 dir = Normalize(Vector3(-bossContext_.worldTransform->transform_.translate.x, 0.0f, -bossContext_.worldTransform->transform_.translate.z));
	float angle = std::numbers::pi_v<float> / 4.0f;
	float cos = std::cosf(angle);
	float sin = std::sinf(angle);
	startPos_ = { dir.x * cos - dir.z * sin,0.0f,dir.x * sin + dir.z * cos };
	endPos_ = { dir.x * cos - dir.z * -sin,0.0f,dir.x * -sin + dir.z * cos };

	// InPhaseの最初の方向
	startDir_ = dir;
	// outPhaseの最後の方向
	//endDir_ = dir;

	// アニメーション
	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::IceBreath)]["IceBreath_Prepare"]);
	bossContext_.animationTimer = 0.0f;

	/*isMidAnimation_ = false;
	isEndANimation_ = false;*/

	windPhase_ = WindPhase::In;
}

void BossStateBattle::WindAttackUpdate() {

	// 風攻撃
	switch (windPhase_)
	{
	case BossStateBattle::WindPhase::In: {
#pragma region WindIn

		windTimer_ += FpsCounter::deltaTime / windInTime_;
		bossContext_.animationTimer = windTimer_;

		// 回転
		Vector3 dir = Slerp(startDir_, endPos_, windTimer_);
		// Y軸周りの角度
		bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

		// ブレスを吐く方向を向く
		if (windTimer_ >= 1.0f) {
			windTimer_ = 0.0f;

			// アニメーション
			bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::IceBreath)]["IceBreath_Main"]);
			bossContext_.animationTimer = 0.0f;
			bossContext_.isWindAttack_ = true;

			// フェーズを切り替え
			windPhase_ = WindPhase::Main;
		}
#pragma endregion
		break;
	}

	case BossStateBattle::WindPhase::Main: {
#pragma region WindMain

		if (bossContext_.isWindAttack_) {
			bossContext_.isWindAttack_ = false;
		}

		windTimer_ += FpsCounter::deltaTime / windMainTime_;
		bossContext_.animationTimer = windTimer_;

		// 回転
		Vector3 dir = Slerp(endPos_, startPos_, windTimer_);
		// Y軸周りの角度
		bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

		// ブレスを吐く方向を向く
		if (windTimer_ >= 1.0f) {
			windTimer_ = 0.0f;

			// アニメーション
			//bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::IceBreath)]["IceBreath_End"]);
			bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);
			bossContext_.animationTimer = 0.0f;

			// フェーズを切り替え
			windPhase_ = WindPhase::Out;
		}
#pragma endregion
		break;
	}

	case BossStateBattle::WindPhase::Out: {
#pragma region WindOut
		windTimer_ += FpsCounter::deltaTime / windOutTime_;
		bossContext_.animationTimer = windTimer_;

		// 回転
		Vector3 dir = Slerp(startPos_, startDir_, windTimer_);
		// Y軸周りの角度
		bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

		// ブレスを吐く方向を向く
		if (windTimer_ >= 1.0f) {
			windTimer_ = 0.0f;

			// 振る舞いの切り替えをリクエスト
			behaviorRequest_ = ButtleBehavior::Normal;
		}
#pragma endregion
		break;
	}
	}
}

void BossStateBattle::ResetIceFall() {
	iceFallTimer_ = 0.0f;
	bossContext_.isActiveIceFall = false;

	//　アニメーション
	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);
	bossContext_.animationTimer = 0.0f;

	isWaitIceFall_ = true;
}

void BossStateBattle::IceFallAttackUpdate() {

	if (isWaitIceFall_) {
		iceFallTimer_ += FpsCounter::deltaTime / waitIceFallMaxTime_;
		bossContext_.animationTimer = iceFallTimer_;

		if (iceFallTimer_ >= 1.0f) {
			iceFallTimer_ = 0.0f;
			isWaitIceFall_ = false;
			// アニメーション
			bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::Scream)]["Scream"]);
			bossContext_.animationTimer = 0.0f;
			bossContext_.isActiveIceFall = true;
		}
	} else {
		if (bossContext_.isActiveIceFall) {
			bossContext_.isActiveIceFall = false;
		}

		iceFallTimer_ += FpsCounter::deltaTime / iceFallMaxTime_;
		bossContext_.animationTimer = iceFallTimer_;

		// 待機の終了
		if (iceFallTimer_ >= 1.0f) {
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
		behaviorRequest_ = ButtleBehavior::Normal;
	}
}

void BossStateBattle::ResetInMove() {
	// 円の中心から自分へのベクトルを求める
	Vector3 myDir = Normalize(Vector3(bossContext_.worldTransform->transform_.translate.x, 0.0f, bossContext_.worldTransform->transform_.translate.z));
	startDir_ = myDir;
	endDir_ = myDir * -1.0f;

	// 戻る位置の始点と終点を決める
	startBackPos_ = bossContext_.worldTransform->transform_.translate;
	endBackPos_ = myDir * (stageRadius_ - (stageRadius_ / 6.0f));
	endBackPos_.y = defalutPosY_;

	// 時間をリセット
	backTimer_ = 0.0f;

	// アニメーション
	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);
	bossContext_.animationTimer = 0.0f;
}

void BossStateBattle::InMoveUpdate() {
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
		behaviorRequest_ = ButtleBehavior::Normal;
	}
}

void BossStateBattle::ResetRotateMove() {

	// 回転移動の位置と時間を求める
#pragma region MakeRotateMovePos

	// 円の中心から自分へのベクトルを求める
	Vector3 myDir = Normalize(Vector3(bossContext_.worldTransform->transform_.translate.x, 0.0f, bossContext_.worldTransform->transform_.translate.z));
	// 最初の角度を求める
	float startAngle = std::atan2f(myDir.z, myDir.x);

	// 回転する方向を求める
	float rotOffset = 0.0f;
	if (RandomGenerator::Get(0, 1) == 0) {
		rotOffset = std::numbers::pi_v<float> *0.5f;
	} else {
		rotOffset = -std::numbers::pi_v<float> *0.5f;
	}

	// 反対側の角度を求める
	float endAngle = startAngle + rotOffset;
	endAngle = WrapAngle(endAngle);

	// 回る時間を求める
	if (startAngle == endAngle) {
		moveTimer_ = 1.0f;
		rotateMoveTime_ = 1.0f;
	} else {
		rotateMoveTime_ = GetMoveTimeDistance(startAngle, endAngle, stageRadius_, rotSpeed_);
		rotateMoveTime_ *= rotateTimeRatio_;
	}

	// 軌跡を登録
	controlPoints_.clear();
	controlPoints_.push_back(bossContext_.worldTransform->transform_.translate);
	float t = 0.0f;
	while (t <= 1.0f)
	{
		t += 0.02f;

		// 角度補間する
		float angle = LerpShortAngle(startAngle, endAngle, t);

		// 位置を求める
		Vector3 pos = { std::cosf(angle) * (stageRadius_ * RotateMoveRadiusRatio_),defalutPosY_,std::sinf(angle) * (stageRadius_ * RotateMoveRadiusRatio_) };

		controlPoints_.push_back(pos);
	}
#pragma endregion

	/// 自分自体の回転要素

	// 最初の回転各
	float rot = LerpShortAngle(startAngle, endAngle, EaseInOut(0.2f));
	Vector3 prePos = { std::cosf(rot) * stageRadius_, bossContext_.worldTransform->transform_.translate.y,std::sinf(rot) * stageRadius_ };
	startRotEndDir_ = Normalize(prePos - bossContext_.worldTransform->transform_.translate);

	// 最初の回転するための角度を求める
	startDir_ = Normalize(Vector3(-bossContext_.worldTransform->transform_.translate.x, 0.0f, -bossContext_.worldTransform->transform_.translate.z));

	// 最後の回転するための最初の角度を求める
	rot = LerpShortAngle(startAngle, endAngle, EaseInOut(1.0f));
	prePos = { std::cosf(rot) * stageRadius_, bossContext_.worldTransform->transform_.translate.y,std::sinf(rot) * stageRadius_ };
	endDir_ = Normalize(prePos * -1.0f);

	// 上下移動する回数を求める
	float angleDiff = std::fabs(Length(CatmullRomPosition(controlPoints_, 1.0f) - CatmullRomPosition(controlPoints_, 0.0f)));
	cycleCount_ = angleDiff / (stageRadius_ * 0.5f);
	
	//=========================================-

	// アニメーション
	bossContext_.animationTimer = 0.0f;
	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);

	moveTimer_ = 0.0f;
}

void BossStateBattle::RotateMoveUpdate() {

	moveTimer_ += FpsCounter::deltaTime / rotateMoveTime_;

	// 移動
#pragma region Move

		// 縦移動
	float posY = 0.0f;
	float totalCycle = moveTimer_ * cycleCount_;
	float localTimer = std::fmodf(totalCycle, 1.0f);

	if (localTimer <= 0.5f) {
		float t = localTimer / 0.5f;
		posY = Lerp(0.0f, 2.0f, EaseInOut(t));
	} else {
		float t = (localTimer - 0.5f) / 0.5f;
		posY = Lerp(2.0f, 0.0f, EaseInOut(t));
	}

	moveTimer_ = std::min(moveTimer_, 1.0f);
	Vector3 pos = CatmullRomPosition(controlPoints_, EaseInOut(moveTimer_));

	// 移動
	bossContext_.worldTransform->transform_.translate = pos;
	bossContext_.worldTransform->transform_.translate.y += posY;
#pragma endregion

	// 回転
#pragma region RotateMove
	// 計算用の進行方向ベクトル
	Vector3 dir = { 0,0,1 };

	// 回転の処理
	if (moveTimer_ <= 0.2f) {
		float localT = moveTimer_ / 0.2f;

		// 回転
		dir = Slerp(startDir_, startRotEndDir_, EaseIn(localT));
		// Y軸周りの角度
		bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);

	} else if (moveTimer_ <= 0.8f) {
		//bossContext_.worldTransform->transform_.rotate.z = 0.2f;
		// 進行方向に向ける
		Vector3 prePos = CatmullRomPosition(controlPoints_, EaseInOut(moveTimer_ + FpsCounter::deltaTime / rotateMoveTime_));
		dir = Normalize(prePos - pos);
		// Y軸周りの角度
		bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);
		// 保存
		endRotStartDir_ = dir;
	} else {

		float localT = (moveTimer_ - 0.8f) / 0.2f;
		// 回転
		dir = Slerp(endRotStartDir_, endDir_, EaseOut(localT));
		// Y軸周りの角度
		bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);
	}
#pragma endregion

	// アニメーション
	bossContext_.animationTimer += FpsCounter::deltaTime;
	bossContext_.animationTimer = std::fmodf(bossContext_.animationTimer, 1.0f);

	// 行動の終了
	if (moveTimer_ >= 1.0f) {
		// 振る舞いの切り替えをリクエスト
		behaviorRequest_ = ButtleBehavior::Normal;
	}
}

void BossStateBattle::ResetCrossMove() {
	// 開始位置を設定
	startRushPos_ = bossContext_.worldTransform->transform_.translate;
	Vector3 dir = Normalize(Vector3(bossContext_.worldTransform->transform_.translate.x, 0.0f, bossContext_.worldTransform->transform_.translate.z));

	// 始点と終点の角度を求める
	float angle = RandomGenerator::Get(-std::numbers::pi_v<float> / 4.0f, std::numbers::pi_v<float> / 4.0f);
	float cos = std::cosf(angle);
	float sin = std::sinf(angle);
	Vector3 startDir = { dir.x * cos - dir.z * sin,0.0f,dir.x * sin + dir.z * cos };
	// 反転
	startDir *= -1.0f;

	// 終盤の位置を取得
	endRushPos_ = startDir * (stageRadius_ * crossEndRatio_);

	/// 自分自体の回転要素

	// 最初の回転するための角度を求める
	startDir_ = Normalize(Vector3(-bossContext_.worldTransform->transform_.translate.x, 0.0f, -bossContext_.worldTransform->transform_.translate.z));
	// 最初の内の最後に向く方向
	startRotEndDir_ = Normalize(endRushPos_);

	endRotStartDir_ = startRotEndDir_;
	// 最後の内の最後に向く方向
	Vector3 tmp = Normalize(Vector3(-endRushPos_.x, 0.0f, -endRushPos_.z));
	endDir_ = tmp;

	//Log("startDir : x:" + std::to_string(endRotStartDir_.x) + ", y:" + std::to_string(endRotStartDir_.y) + ", z:" + std::to_string(endRotStartDir_.z));
	//Log("endDir : x:" + std::to_string(endDir_.x) + ", y:" + std::to_string(endDir_.y) + ", z:" + std::to_string(endDir_.z));

	// アニメーション
	bossContext_.animationTimer = 0.0f;
	bossContext_.animator_->SetAnimationData(&(*bossContext_.animationData_)[static_cast<size_t>(enemyAnimationType::BaseMove)]["基本移動"]);

	moveTimer_ = 0.0f;
}

void BossStateBattle::CrossMoveUpdate() {

	moveTimer_ += FpsCounter::deltaTime / crossMoveTime_;

	// 移動
#pragma region Move
	// 縦移動
	float posY = 0.0f;
	float totalCycle = moveTimer_ * upDownCount_;
	float localTimer = std::fmodf(totalCycle, 1.0f);
	if (localTimer <= 0.5f) {
		float t = localTimer / 0.5f;
		posY = Lerp(0.0f, maxMoveHeight_, EaseInOut(t));
	} else {
		float t = (localTimer - 0.5f) / 0.5f;
		posY = Lerp(maxMoveHeight_, 0.0f, EaseInOut(t));
	}

	// 移動
	moveTimer_ = std::min(moveTimer_, 1.0f);
	Vector3 pos = Lerp(startRushPos_, endRushPos_, EaseInOut(moveTimer_));
	bossContext_.worldTransform->transform_.translate = pos;
	bossContext_.worldTransform->transform_.translate.y = defalutPosY_;
	bossContext_.worldTransform->transform_.translate.y += posY;
#pragma endregion

	// 回転
#pragma region Move
	// 計算用の進行方向ベクトル
	Vector3 dir = { 0,0,1 };

	// 回転の処理
	if (moveTimer_ <= 0.2f) {
		float localT = moveTimer_ / 0.2f;
		// 回転
		dir = Slerp(startDir_, startRotEndDir_, EaseIn(localT));
		// Y軸周りの角度
		bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);
	} else if (moveTimer_ >= 0.8f) {

		float localT = (moveTimer_ - 0.8f) / 0.2f;
		// 回転
		dir = Slerp(endRotStartDir_, endDir_, EaseOut(localT));
		// Y軸周りの角度
		bossContext_.worldTransform->transform_.rotate.y = std::atan2f(dir.x, dir.z);
	}
#pragma endregion

	// アニメーション
	bossContext_.animationTimer += FpsCounter::deltaTime;
	bossContext_.animationTimer = std::fmodf(bossContext_.animationTimer, 1.0f);

	// 行動の終了
	if (moveTimer_ >= 1.0f) {
		// 振る舞いの切り替えをリクエスト
		behaviorRequest_ = ButtleBehavior::Normal;
	}
}

void BossStateBattle::RegisterBebugParam() {

	// ボスの基礎要素
	GameParamEditor::GetInstance()->AddItem("Boss", "DefaultsPosY", defalutPosY_);

	// 突進攻撃
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "OffestEndRush", offsetEndRush_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "RotateSpeed", rotSpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "RushMainTime", rushMainTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "RushOutTime", rushOutTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "RushEndRatio", rushEndRatio_);

	// 氷柱攻撃
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "IceFallTime", iceFallMaxTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "WaitIceFallTime", waitIceFallMaxTime_);

	// 風攻撃
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WindInTime", windInTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WindMainTime", windMainTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WindOutTime", windOutTime_);

	// 回転移動
	GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "RotateTimeRatio", rotateTimeRatio_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "RotateMoveRadiusRatio", RotateMoveRadiusRatio_);

	// 横断行動
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "CrossMoveTime", crossMoveTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "CrossEndRatio", crossEndRatio_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "UpDownCount", upDownCount_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "MaxMoveHeight", maxMoveHeight_);

	// 行動遷移の管理
	GameParamEditor::GetInstance()->AddItem(kGroupNames[5], "RushAttackWeight", rushAttackWeight_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[5], "WindAttackWeight", WindAttackWeight_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[5], "IceFallWeight", IceFallWeight_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[5], "WaitWeight", WaitWeight_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[5], "RotateMoveWeight", RotateMoveWeight_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[5], "CrossMoveWeight", CrossMoveWeight_);
}

void BossStateBattle::ApplyDebugParam() {
	// ボスの基礎要素
	defalutPosY_ = GameParamEditor::GetInstance()->GetValue<float>("Boss", "DefaultsPosY");

	// 突進攻撃
	offsetEndRush_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "OffestEndRush");
	rotSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "RotateSpeed");
	rushMainTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "RushMainTime");
	rushOutTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "RushOutTime");
	rushEndRatio_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "RushEndRatio");

	// 氷柱攻撃
	iceFallMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "IceFallTime");
	waitIceFallMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "WaitIceFallTime");
	bossContext_.waitIceFallMaxTime = waitIceFallMaxTime_;

	// 風攻撃
	windInTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WindInTime");
	windMainTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WindMainTime");
	windOutTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WindOutTime");

	// 回転行動
	rotateTimeRatio_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "RotateTimeRatio");
	RotateMoveRadiusRatio_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "RotateMoveRadiusRatio");

	// 横断行動
	crossMoveTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "CrossMoveTime");
	crossEndRatio_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "CrossEndRatio");
	upDownCount_ = GameParamEditor::GetInstance()->GetValue<uint32_t>(kGroupNames[4], "UpDownCount");
	maxMoveHeight_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "MaxMoveHeight");

	// 行動の管理
	rushAttackWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<uint32_t>(kGroupNames[5], "RushAttackWeight"));
	WindAttackWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<uint32_t>(kGroupNames[5], "WindAttackWeight"));
	IceFallWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<uint32_t>(kGroupNames[5], "IceFallWeight"));
	WaitWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<uint32_t>(kGroupNames[5], "WaitWeight"));
	RotateMoveWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<uint32_t>(kGroupNames[5], "RotateMoveWeight"));
	CrossMoveWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<uint32_t>(kGroupNames[5], "CrossMoveWeight"));

	lotteryList_ = {
			{ ButtleBehavior::RushAttack,    rushAttackWeight_ }, // 突進
			{ ButtleBehavior::WindAttack,    WindAttackWeight_ }, // 風
			{ ButtleBehavior::IceFallAttack, IceFallWeight_ }, // 氷柱
			{ ButtleBehavior::Wait,          WaitWeight_  }, // 待機
			{ ButtleBehavior::RotateMove,    RotateMoveWeight_ },  // 回転移動
			{ ButtleBehavior::CrossMove,     CrossMoveWeight_}   // 横断移動
	};
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

	float WrapAngle(float angle) {
		constexpr float twoPi = std::numbers::pi_v<float> *2.0f;
		angle = std::fmod(angle, twoPi);
		if (angle < 0.0f) {
			angle += twoPi;
		}
		return angle;
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