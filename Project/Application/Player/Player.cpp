#define NOMINMAX
#include"Player.h"
#include<algorithm>
#include<cmath>
#include"GameParamEditor.h"
#include"EasingManager.h"
#include"MyMath.h"
#include"FPSCounter.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
#include "LogManager.h"
using namespace GameEngine;

void Player::Initialize(GameEngine::Animator *animator, const std::array<std::map<std::string, AnimationData>, kPlayerAnimationCount>& animationData) {
	(void)animator;
	(void)animationData;
	// ワールド行列を初期化
	worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{-2.0f,1.0f,0.0f} });
	// コライダー生成・設定
	collider_ = std::make_unique<SphereCollider>();
	collider_->SetRadius(sphereData_.radius);
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
	collider_->SetCollisionAttribute(kCollisionAttributePlayer);
	collider_->SetCollisionMask(~kCollisionAttributePlayer);
	// ユーザーデータ設定（IDのみ暫定）
	UserData userData;
	userData.typeID = static_cast<uint32_t>(CollisionTypeID::Player);
	userData.object = this;
	collider_->SetUserData(userData);
	// コールバック登録
	collider_->SetOnCollisionCallback([this](const CollisionResult &result) { this->OnCollision(result); });
#ifdef _DEBUG
	//===========================================================
	// 
	// 現在、saveを押していないので、Playerのjsonファイルは存在していません
	// 
	//===========================================================

	// 値を登録する
	RegisterBebugParam();
#else
	// jsonファイルが作られていない状態で値の適応をおこなうとリリース版でバクります
	// 値を適応させる
	ApplyDebugParam();
#endif
}

void Player::Update(GameEngine::InputCommand* inputCommand, const Camera& camera) {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif
	// カメラ基準ベクトル更新
	UpdateCameraBasis(&camera);

	bounceAwayDir_ = bounceAwayDir_;
	desiredVelXZ_ = { 0.0f, 0.0f, 0.0f };
	BounceUpdate();
	ProcessMoveInput(inputCommand);
    ProcessAttackDownInput(inputCommand);
	HandleRushCharge(inputCommand);
	HandleRushStart(inputCommand);
	RushUpdate();

	// 重力（常時適応）
	velocity_.y += kFallAcceleration_ * FpsCounter::deltaTime;
	// 縦方向の上限（落下最大速度）
	velocity_.y = std::max(velocity_.y, (isAttackDown_ ? -kAttackDownSpeed_ : -kMaxFallSpeed_));

	// 速度を適応
	worldTransform_.transform_.translate.x += velocity_.x * FpsCounter::deltaTime;
	worldTransform_.transform_.translate.y += velocity_.y * FpsCounter::deltaTime;
	worldTransform_.transform_.translate.z += velocity_.z * FpsCounter::deltaTime;

    // 回転ターゲットを決定（溜め中はrushDirection_、それ以外はlastMoveDir_または移動速度）
    Vector3 targetDir = {0.0f, 0.0f, 0.0f};
    if (isCharging_) {
        targetDir = rushDirection_;
    } else if (lastMoveDir_.x != 0.0f || lastMoveDir_.z != 0.0f) {
        targetDir = lastMoveDir_;
    } else {
        targetDir = { velocity_.x, 0.0f, velocity_.z };
    }

    if (targetDir.x != 0.0f || targetDir.z != 0.0f) {
        targetDir.y = 0.0f;
        targetDir = Normalize(targetDir);
        float targetYaw = std::atan2f(targetDir.x, targetDir.z); // ラジアン
        // 最短角度差にする
        float diff = targetYaw - currentYaw_;
        const float PI = 3.14159265358979323846f;
        while (diff > PI) { diff -= 2.0f * PI; }
        while (diff < -PI) { diff += 2.0f * PI; }
        float maxStep = kRotationLerpSpeed_ * FpsCounter::deltaTime;
        if (std::fabs(diff) <= maxStep) {
            currentYaw_ = targetYaw;
        } else {
            currentYaw_ += (diff > 0.0f ? maxStep : -maxStep);
        }
        worldTransform_.transform_.rotate.y = currentYaw_;
    }

	// 地面との当たり（仮）
	if (worldTransform_.transform_.translate.y <= 0.0f) {
		worldTransform_.transform_.translate.y = 0.0f;
		if (velocity_.y < 0.0f) { velocity_.y = 0.0f; }
		isJump_ = false;
        isAttackDown_ = false;
	}

	// 行列の更新
	worldTransform_.UpdateTransformMatrix();

	// コライダー位置同期
	if (collider_) {
		collider_->SetWorldPosition(worldTransform_.transform_.translate);
        sphereData_.center = worldTransform_.transform_.translate;
	}

	// 追加: 保留中の当たり判定を処理
	ProcessPendingCollisions();
}

void Player::UpdateCameraBasis(const Camera* camera) {
	if (!camera) { return; }
	// ワールド行列から前方向を取得 (カメラのZ軸マイナスがForward想定)
	Matrix4x4 camWorld = camera->GetWorldMatrix();
	Vector3 forward = { -camWorld.m[2][0], -camWorld.m[2][1], -camWorld.m[2][2] }; // カメラが見る方向
	forward = Normalize(forward);
	// XZ平面へ投影
	forward.y = 0.0f;
	if (forward.x != 0.0f || forward.z != 0.0f) { forward = Normalize(forward); }
	// 右方向 = カメラのX軸 (ワールド行列の0列)
	Vector3 right = { camWorld.m[0][0], camWorld.m[0][1], camWorld.m[0][2] };
	right.y = 0.0f;
	if (right.x != 0.0f || right.z != 0.0f) { right = Normalize(right); }
	cameraForwardXZ_ = forward;
	cameraRightXZ_ = right;
}

void Player::ProcessMoveInput(GameEngine::InputCommand *inputCommand) {
	// 溜め/予備/突進/硬直中は通常移動禁止
	if (isCharging_ || isPreRushing_ || isRushing_ || isBounceLock_ || isRushLock_) { 
		// 溜め中は向きのみ変更可能
		if (isCharging_) {
			Vector3 dir = { 0.0f, 0.0f, 0.0f };
			if (inputCommand->IsCommandActive("MoveUp"))    { dir -= cameraForwardXZ_; }
			if (inputCommand->IsCommandActive("MoveDown"))  { dir += cameraForwardXZ_; }
			if (inputCommand->IsCommandActive("MoveLeft"))  { dir -= cameraRightXZ_; }
			if (inputCommand->IsCommandActive("MoveRight")) { dir += cameraRightXZ_; }
			if (dir.x != 0.0f || dir.z != 0.0f) {
				dir.y = 0.0f;
				dir = Normalize(dir);
				lastMoveDir_ = { dir.x, 0.0f, dir.z };
				// Rush方向更新
				rushDirection_ = lastMoveDir_;
			}
		}
	} else {
		Vector3 dir = { 0.0f, 0.0f, 0.0f };
		if (inputCommand->IsCommandActive("MoveUp")) { dir -= cameraForwardXZ_; }
		if (inputCommand->IsCommandActive("MoveDown")) { dir += cameraForwardXZ_; }
		if (inputCommand->IsCommandActive("MoveLeft")) { dir -= cameraRightXZ_; }
		if (inputCommand->IsCommandActive("MoveRight")) { dir += cameraRightXZ_; }
		if (dir.x != 0.0f || dir.z != 0.0f) {
			// Y成分は常に0
			dir.y = 0.0f;
			dir = Normalize(dir);
			lastMoveDir_ = { dir.x, 0.0f, dir.z };
		}
		float maxSpeed = (isJump_ ? kAirMoveSpeed_ : kMoveSpeed_);
		// 入力から算出される到達可能最大速度（XZのみ）。ここで既に上限を規定するため、直接 velocity を clamp する必要はない。
		desiredVelXZ_.x = dir.x * maxSpeed;
		desiredVelXZ_.z = dir.z * maxSpeed;
	}

	// ここで加速と減速を分離する
	auto applyAxis = [&](float &vel, float target, bool isAir) {
		float accel = isAir ? kAirAcceleration_ : kGroundAcceleration_;
		float decel = isAir ? kAirDeceleration_ : kGroundDeceleration_;
		float dt = FpsCounter::deltaTime;
		if (target != 0.0f) {
			// 目標速度方向へ加速
			float diff = target - vel;
			float step = accel * dt;
			if (std::fabs(diff) <= step) { vel = target; }
			else { vel += (diff > 0.0f ? step : -step); }
		} else {
			// 入力が無いので減速
			float speed = std::fabs(vel);
			float step = decel * dt;
			if (speed <= step) { vel = 0.0f; }
			else { vel += (vel > 0.0f ? -step : step); }
		}
	};

	applyAxis(velocity_.x, desiredVelXZ_.x, isJump_);
	applyAxis(velocity_.z, desiredVelXZ_.z, isJump_);

	// 最大速度の安全確認（念のため）
	float maxSpeedGround = kMoveSpeed_;
	float maxSpeedAir = kAirMoveSpeed_;
	float maxSpeed = isJump_ ? maxSpeedAir : maxSpeedGround;
	Vector3 horiz = { velocity_.x, 0.0f, velocity_.z };
	float horizLen = Length(horiz);
	if (horizLen > maxSpeed) {
		Vector3 n = Normalize(horiz);
		velocity_.x = n.x * maxSpeed;
		velocity_.z = n.z * maxSpeed;
	}
}

void Player::ProcessAttackDownInput(GameEngine::InputCommand *inputCommand) {
    if (isCharging_ || isPreRushing_ || isRushing_ || isBounceLock_ || isRushLock_ || !isJump_) {
		return;
	}
	// 攻撃下降開始
	if (inputCommand->IsCommandActive("AttackDown")) {
		if (isAttackDown_) {
			velocity_.y = -kMaxFallSpeed_;
		} else {
			velocity_.y = -kAttackDownSpeed_;
            velocity_.x = 0.0f;
            velocity_.z = 0.0f;
		}
        isAttackDown_ = !isAttackDown_;
    }
}

void Player::HandleRushCharge(GameEngine::InputCommand* inputCommand) {
	if (isCharging_ || isJump_ || isRushing_) {
		return;
	}
	// 溜め開始/継続
	if (inputCommand->IsCommandActive("RushCharge")) {
		isCharging_ = true;
		chargeTimer_ = 0.0f;
		chargeRatio_ = 0.0f;
		// Rush方向初期化
		Vector3 rushDirXZ = lastMoveDir_;
		if (rushDirXZ.x == 0.0f && rushDirXZ.z == 0.0f) { rushDirXZ = { velocity_.x, 0.0f, velocity_.z }; }
		rushDirection_ = (rushDirXZ.x == 0.0f && rushDirXZ.z == 0.0f) ? cameraForwardXZ_ : Normalize(rushDirXZ);
		// 溜め開始時はレベルをリセット
		rushChargeLevel_ = 0;
	}
}

void Player::HandleRushStart(GameEngine::InputCommand* inputCommand) {
	if (!isCharging_) {
		return;
	}
	// 溜め解除でRush開始要求
	if (inputCommand->IsCommandActive("RushStart")) {
		// 予備動作時間を溜め比率で決定
		chargeRatio_ = std::clamp(chargeTimer_ / kRushChargeMaxTime_, 0.0f, 1.0f);
		// 溜め比率に応じてレベル決定（1-3）
		if (chargeRatio_ < kRushChargeLevel2Ratio_) {
			rushChargeLevel_ = 1;
		} else if (chargeRatio_ < kRushChargeLevel3Ratio_) {
			rushChargeLevel_ = 2;
		} else {
			rushChargeLevel_ = 3;
		}
		preRushDuration_ = Lerp(0.0f, kPreRushMaxTime_, chargeRatio_);
		rushActiveTimer_ = Lerp(0.0f, kRushLockMaxTime_, chargeRatio_);
		isCharging_ = false;
		isPreRushing_ = true;
		rushTimer_ = 0.0f; // 可変予備動作
	}
}

void Player::RushUpdate() {
	// 溜め進行
	if (isCharging_) {
		chargeTimer_ += FpsCounter::deltaTime;
		chargeTimer_ = std::min(chargeTimer_, kRushChargeMaxTime_);
		// 視覚/内部用に比率とレベル更新
		chargeRatio_ = std::clamp(chargeTimer_ / kRushChargeMaxTime_, 0.0f, 1.0f);
		if (chargeRatio_ < kRushChargeLevel2Ratio_) {
			rushChargeLevel_ = 1;
		} else if (chargeRatio_ < kRushChargeLevel3Ratio_) {
			rushChargeLevel_ = 2;
		} else {
			rushChargeLevel_ = 3;
		}
		return;
	}

	if (!isPreRushing_ && !isRushing_) { return; }
	if (isPreRushing_) {
		rushTimer_ += FpsCounter::deltaTime;
		float waitTime = (preRushDuration_ > 0.0f ? preRushDuration_ : kPreRushMaxTime_);
		if (rushTimer_ >= waitTime) { 
			isPreRushing_ = false; isRushing_ = true;
			// 突進は初速のみ設定（溜めのレベルのみで速度を決定）
			float levelMultiplier = 1.0f;
	switch (rushChargeLevel_) {
	case 1: levelMultiplier = kRushStrengthLevel1_; break;
	case 2: levelMultiplier = kRushStrengthLevel2_; break;
	case 3: levelMultiplier = kRushStrengthLevel3_; break;
	default: levelMultiplier = kRushStrengthLevel1_; break;
	}
	float rushSpeed = kRushMaxSpeed_ * levelMultiplier;
	Vector3 initVel = rushDirection_ * rushSpeed;
	velocity_.x = initVel.x; velocity_.z = initVel.z; // 慣性に任せる
		}
		return;
	}
	if (isRushing_) {
		rushActiveTimer_ -= FpsCounter::deltaTime;
		// 突進時間が終了したら突進終了
		if (rushActiveTimer_ <= 0.0f) {
			isRushing_ = false;
		}
	}
}

void Player::BounceUpdate() {
	if (!isBounceLock_) { return; }
	isRushing_ = false;
	bounceLockTimer_ += FpsCounter::deltaTime;
	if (bounceLockTimer_ >= currentBounceLockTime_) {
		isBounceLock_ = false; bounceLockTimer_ = 0.0f; currentBounceUpSpeed_ = 0.0f; currentBounceAwaySpeed_ = 0.0f; currentBounceLockTime_ = 0.0f; bounceAwayDir_ = {0.0f,0.0f,0.0f};
	}
}

void Player::Bounce(const Vector3 &bounceDirection, float bounceStrength, bool isIceFall) {
	if (isPreRushing_ || !isRushing_) { return; }
	isPreRushing_ = false; isJump_ = true;
	isBounceLock_ = true; bounceLockTimer_ = 0.0f; bounceAwayDir_ = { -bounceDirection.x, 0.0f, -bounceDirection.z }; bounceAwayDir_ = Normalize(bounceAwayDir_);
	// IceFall の場合は溜めレベルに依存せず一定の強さで跳ね返す
	if (isIceFall) {
		velocity_ = bounceAwayDir_ * (kWallBounceAwaySpeed_ * bounceStrength);
		velocity_.y = kWallBounceUpSpeed_ * bounceStrength;
		currentBounceLockTime_ = kWallBounceLockTime_;
		return;
	}
	// 既存の bounceStrength に加えて溜めレベルに応じた倍率を適用
	float levelMultiplier = 1.0f;
	switch (rushChargeLevel_) {
	case 1: levelMultiplier = kWallBounceStrengthLevel1_; break;
	case 2: levelMultiplier = kWallBounceStrengthLevel2_; break;
	case 3: levelMultiplier = kWallBounceStrengthLevel3_; break;
	default: levelMultiplier = kWallBounceStrengthLevel1_; break;
	}
	velocity_ = bounceAwayDir_ * (kWallBounceAwaySpeed_ * bounceStrength * levelMultiplier);
	velocity_.y = kWallBounceUpSpeed_ * bounceStrength * levelMultiplier; currentBounceLockTime_ = kWallBounceLockTime_;
}

void Player::OnCollision(const CollisionResult &result) {
	// 当たり判定が発生したらその場では処理せず保存して後で処理する
	pendingCollisions_.push_back(result);
}

void Player::RegisterBebugParam() {
	// 値の登録
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "JumpMaxHeight", kJumpHeight_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "JumpMaxTime", kJumpMaxTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "MoveSpeed", kMoveSpeed_);

	// プレイヤー共通
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "AirMoveSpeed", kAirMoveSpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "MaxFallSpeed", kMaxFallSpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "AirAcceleration", kAirAcceleration_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "GroundAcceleration", kGroundAcceleration_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "AirDeceleration", kAirDeceleration_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "FallAcceleration", kFallAcceleration_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "WallHitReflectFactor", kWallHitReflectFactor_);

	// 突撃設定
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "PreRushTime", kPreRushMaxTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushSpeed", kRushMaxSpeed_);
    // レベルまでの突進溜め比率
    GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushChargeLevel1Ratio", kRushChargeLevel1Ratio_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushChargeLevel2Ratio", kRushChargeLevel2Ratio_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushChargeLevel3Ratio", kRushChargeLevel3Ratio_);

	// レベル毎の突進強さ
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushStrengthLevel1", kRushStrengthLevel1_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushStrengthLevel2", kRushStrengthLevel2_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushStrengthLevel3", kRushStrengthLevel3_);

	// 通常壁跳ね返り設定
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceUpSpeed", kWallBounceUpSpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceAwaySpeed", kWallBounceAwaySpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceLockTime", kWallBounceLockTime_);
	// レベル毎の壁跳ね返り強さ
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceStrengthLevel1", kWallBounceStrengthLevel1_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceStrengthLevel2", kWallBounceStrengthLevel2_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceStrengthLevel3", kWallBounceStrengthLevel3_);

	// Attack（空中急降下）設定
    GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "AttackPreDownTime", kAttackPreDownTime_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "AttackDownSpeed", kAttackDownSpeed_);
}

void Player::ApplyDebugParam() {
	// 値の適応
	kJumpHeight_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "JumpMaxHeight");
	kJumpMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "JumpMaxTime");
	kMoveSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "MoveSpeed");

	// プレイヤー共通
	kAirMoveSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "AirMoveSpeed");
	kMaxFallSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "MaxFallSpeed");
	kAirAcceleration_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "AirAcceleration");
	kGroundAcceleration_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "GroundAcceleration");
	kAirDeceleration_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "AirDeceleration");
	kFallAcceleration_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "FallAcceleration");

	// 突撃設定
	kPreRushMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "PreRushTime");
	kRushMaxSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushSpeed");
    // レベルまでの突進溜め比率
    kRushChargeLevel1Ratio_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushChargeLevel1Ratio");
    kRushChargeLevel2Ratio_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushChargeLevel2Ratio");
    kRushChargeLevel3Ratio_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushChargeLevel3Ratio");
	// レベル毎の突進強さ
	kRushStrengthLevel1_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushStrengthLevel1");
	kRushStrengthLevel2_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushStrengthLevel2");
	kRushStrengthLevel3_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushStrengthLevel3");

	// 通常壁跳ね返り設定
	kWallBounceUpSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceUpSpeed");
	kWallBounceAwaySpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceAwaySpeed");
	kWallBounceLockTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceLockTime");
	// レベル毎の壁跳ね返り強さ
	kWallBounceStrengthLevel1_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceStrengthLevel1");
	kWallBounceStrengthLevel2_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceStrengthLevel2");
	kWallBounceStrengthLevel3_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceStrengthLevel3");

	// Attack（空中急降下）設定
	kAttackPreDownTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "AttackPreDownTime");
	kAttackDownSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "AttackDownSpeed");
}

void Player::ProcessPendingCollisions() {
	if (pendingCollisions_.empty()) return;

	// 先に通常の壁との衝突があるかを確認し、あれば BoundaryWall を無視する
	bool hasWallCollision = false;
	for (const auto &r : pendingCollisions_) {
		if (r.isHit && r.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Wall)) {
			hasWallCollision = true;
			break;
		}
	}

	// 各当たり判定を順に処理する
	for (const auto &result : pendingCollisions_) {
		if (!result.isHit) continue;

		bool isWall = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Wall));
		bool isIceFall = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::IceFall));
		bool isBoundary = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::BoundaryWall));

		// 壁との衝突処理
		if (isWall && isRushing_) {
			Vector3 normal = result.contactNormal;
			Vector3 bounceDir = { -normal.x, 0.0f, -normal.z };
			if (bounceDir.x != 0.0f || bounceDir.z != 0.0f) { bounceDir = Normalize(bounceDir); }
			if (onWallHit_) { onWallHit_(); }
			Bounce(bounceDir, 1.0f);
			continue;
		}

		if (isWall && !isRushing_) {
			Vector3 n = result.contactNormal;
			Vector3 nXZ = { n.x, 0.0f, n.z };
			if (nXZ.x != 0.0f || nXZ.z != 0.0f) { nXZ = Normalize(nXZ); }
			float depth = std::max(result.penetrationDepth, 0.0f);
			Vector3 correction = { nXZ.x * depth, 0.0f, nXZ.z * depth };
			worldTransform_.transform_.translate.x += correction.x;
			worldTransform_.transform_.translate.z += correction.z;

			Vector3 velXZ = { velocity_.x, 0.0f, velocity_.z };
			float dot = velXZ.x * nXZ.x + velXZ.z * nXZ.z;
			if (dot < 0.0f) {
				Vector3 reflected = {
					velXZ.x - 2.0f * dot * nXZ.x,
					0.0f,
					velXZ.z - 2.0f * dot * nXZ.z
				};
				velocity_.x = reflected.x * kWallHitReflectFactor_;
				velocity_.z = reflected.z * kWallHitReflectFactor_;
				Vector3 newDir = { reflected.x, 0.0f, reflected.z };
				float len = Length(newDir);
				if (len > 0.00001f) { lastMoveDir_ = Normalize(newDir); }
			}
			if (collider_) { collider_->SetWorldPosition(worldTransform_.transform_.translate); }
			continue;
		}

		// 氷柱との衝突処理
		if (isIceFall && isRushing_) {
			Vector3 normal = result.contactNormal;
			Vector3 bounceDir = { -normal.x, 0.0f, -normal.z };
			if (bounceDir.x != 0.0f || bounceDir.z != 0.0f) { bounceDir = Normalize(bounceDir); }
			if (onWallHit_) { onWallHit_(); }
			Bounce(bounceDir, 0.5f, true);
			continue;
		}

		if (isIceFall && !isRushing_) {
			Vector3 n = result.contactNormal;
			if (n.x != 0.0f || n.y != 0.0f || n.z != 0.0f) { n = Normalize(n); }
			float depth = -result.penetrationDepth;
			Vector3 correction = n * depth;
			worldTransform_.transform_.translate.x += correction.x;
			worldTransform_.transform_.translate.y += correction.y;
			worldTransform_.transform_.translate.z += correction.z;
			float dot = velocity_.x * n.x + velocity_.y * n.y + velocity_.z * n.z;
			if (dot < 0.0f) {
				velocity_.x -= n.x * dot;
				velocity_.y -= n.y * dot;
				velocity_.z -= n.z * dot;
			}
			if (collider_) { collider_->SetWorldPosition(worldTransform_.transform_.translate); }
			continue;
		}

		// BoundaryWall の処理: 通常の壁が存在する場合は無視する
		if (isBoundary) {
			if (hasWallCollision) {
				// 通常の壁が別にヒットしているので Boundary は無視
				continue;
			}
			Vector3 n = result.contactNormal;
			if (n.x != 0.0f || n.y != 0.0f || n.z != 0.0f) { n = Normalize(n); }
			float depth = std::max(result.penetrationDepth, 0.0f);
			Vector3 correction = n * depth;
			worldTransform_.transform_.translate.x += correction.x;
			worldTransform_.transform_.translate.y += correction.y;
			worldTransform_.transform_.translate.z += correction.z;
			if (collider_) { collider_->SetWorldPosition(worldTransform_.transform_.translate); }
			continue;
		}
	}

	// 全て処理したらクリア
	pendingCollisions_.clear();
}