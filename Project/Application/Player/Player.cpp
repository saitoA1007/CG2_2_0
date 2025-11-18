#define NOMINMAX
#include"Player.h"
#include<algorithm>
#include"GameParamEditor.h"
#include"EasingManager.h"
#include"MyMath.h"
#include"FPSCounter.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
#include "LogManager.h"
using namespace GameEngine;

void Player::Initialize() {

	// ワールド行列を初期化
	worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{-2.0f,1.0f,0.0f} });

	// コライダー生成・設定
	collider_ = std::make_unique<SphereCollider>();
	collider_->SetRadius(sphereData_.radius);
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
	collider_->SetCollisionAttribute(kCollisionAttributePlayer);
	collider_->SetCollisionMask(~kCollisionAttributePlayer); // プレイヤー以外全部
	// ユーザーデータ設定（IDのみ暫定）
	UserData userData; userData.typeID = static_cast<uint32_t>(CollisionTypeID::Player); userData.object = nullptr; // object設定は必要なら後で
	collider_->SetUserData(userData);
	// コールバック登録
	collider_->SetOnCollisionEnterCallback([this](const CollisionResult& result) { this->OnCollision(result); });

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

void Player::Update(GameEngine::InputCommand* inputCommand) {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

	// フレーム開始時に入力方向をリセット
	bounceAwayDir_ = bounceAwayDir_; // NOP to avoid unused warnings if any macros

    // XZの目標速度リセット
    desiredVelXZ_ = { 0.0f, 0.0f, 0.0f };

	// 跳ね返り（硬直）中の更新を最優先
	BounceUpdate();

	// 入力・突進状態更新（硬直中は無効）
	if (!isBounceLock_) {
		ProcessMoveInput(inputCommand);
		ChargeUpdate();
	}

	// 重力（常時適応）
	velocity_.y += kFallAcceleration_ * FpsCounter::deltaTime;
	// 縦方向の上限（落下最大速度）
    velocity_.y = std::max(velocity_.y, (isAttackDown_ ? -kAttackDownSpeed_ : -MaxFallSpeed_));

	// 速度を適応
	worldTransform_.transform_.translate.x += velocity_.x * FpsCounter::deltaTime;
	worldTransform_.transform_.translate.y += velocity_.y * FpsCounter::deltaTime;
	worldTransform_.transform_.translate.z += velocity_.z * FpsCounter::deltaTime;

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
        // 球データ同期
        sphereData_.center = worldTransform_.transform_.translate;
	}
}

void Player::ProcessMoveInput(GameEngine::InputCommand *inputCommand) {
	if (isPreCharging_ || isCharging_ || isBounceLock_) {
		return;
	}

	// 入力から方向を決める
	Vector3 dir = { 0.0f, 0.0f, 0.0f };
	// MoveUp/Down/Left/Rightの押下状態を確認
	if (inputCommand->IsCommandAcitve("MoveUp")) { dir.z += 1.0f; }
	if (inputCommand->IsCommandAcitve("MoveDown")) { dir.z -= 1.0f; }
	if (inputCommand->IsCommandAcitve("MoveLeft")) { dir.x -= 1.0f; }
	if (inputCommand->IsCommandAcitve("MoveRight")) { dir.x += 1.0f; }
	// 正規化（斜め速度の過剰上昇を防ぐ）
	if (dir.x != 0.0f || dir.z != 0.0f) {
		dir = Normalize(dir);
	}
	float maxSpeed = (isJump_ ? kAirMoveSpeed_ : kMoveSpeed_);
	desiredVelXZ_.x = dir.x * maxSpeed;
	desiredVelXZ_.z = dir.z * maxSpeed;

	// 加減速（地上/空中で異なるレート）
	const float accel = (isJump_ ? kAirDeceleration_ : kGroundAcceleration_);
	auto approach = [](float current, float target, float delta) {
		if (current < target) {
			current = std::min(current + delta, target);
		} else if (current > target) {
			current = std::max(current - delta, target);
		}
		return current;
	};
	velocity_.x = approach(velocity_.x, desiredVelXZ_.x, accel);
	velocity_.z = approach(velocity_.z, desiredVelXZ_.z, accel);

	// 攻撃操作
	if (inputCommand->IsCommandAcitve("Attack")) {
		if (isJump_) {
			velocity_.y = -kAttackDownSpeed_;
            isAttackDown_ = true;
		} else {
			StartCharge(velocity_);
		}
	}
}

void Player::StartCharge(const Vector3& direction) {
	// 突進開始(予備動作)
	isPreCharging_ = true;
	isCharging_ = false;
	chargeTimer_ = 0.0f;
	chargeActiveTimer_ = 0.0f;
	// 方向を設定（正規化）
	chargeDirection_ = Normalize(direction);
	velocity_ = { 0.0f,0.0f,0.0f };
}

void Player::ChargeUpdate() {
	if (!isPreCharging_ && !isCharging_) {
		return;
	}

	// 予備動作中
	if (isPreCharging_) {
		chargeTimer_ += FpsCounter::deltaTime;
		if (chargeTimer_ >= kPreChargeTime_) {
			// 本突進へ移行
			isPreCharging_ = false;
			isCharging_ = true;
			chargeActiveTimer_ = 0.0f;
		}
		return; // 予備動作中は移動しない
	}

	// 突進中
	if (isCharging_) {
		chargeActiveTimer_ += FpsCounter::deltaTime;
		// 突進は一定速度
		desiredVelXZ_.x = chargeDirection_.x * kChargeSpeed_;
		desiredVelXZ_.z = chargeDirection_.z * kChargeSpeed_;
		velocity_.x = desiredVelXZ_.x;
		velocity_.z = desiredVelXZ_.z;
	}
}

void Player::BounceUpdate() {
	if (!isBounceLock_) { return; }

	bounceLockTimer_ += FpsCounter::deltaTime;

	// 硬直解除
	if (bounceLockTimer_ >= currentBounceLockTime_) {
		isBounceLock_ = false;
		bounceLockTimer_ = 0.0f;
		currentBounceUpSpeed_ = 0.0f;
		currentBounceAwaySpeed_ = 0.0f;
		currentBounceLockTime_ = 0.0f;
		bounceAwayDir_ = {0.0f,0.0f,0.0f};
	}
}

void Player::ChargeWallBounce(const Vector3 &bounceDirection, bool isGreatWall) {
	if (isPreCharging_ || !isCharging_) {
		return;
	}
	// 突進終了
	isPreCharging_ = false;
	isCharging_ = false;
	isJump_ = true;

	// 跳ね返りの初期設定（硬直開始）。方向は突進方向と逆
	isBounceLock_ = true;
	bounceLockTimer_ = 0.0f;
	bounceAwayDir_ = { -bounceDirection.x, 0.0f, -bounceDirection.z };
	bounceAwayDir_ = Normalize(bounceAwayDir_);

	if (isGreatWall) {
		velocity_ = bounceAwayDir_ * kGreatWallBounceAwaySpeed_;
		velocity_.y = kGreatWallBounceUpSpeed_;
		currentBounceLockTime_ = kGreatWallBounceLockTime_;
	} else {
		velocity_ = bounceAwayDir_ * kWallBounceAwaySpeed_;
		velocity_.y = kWallBounceUpSpeed_;
		currentBounceLockTime_ = kWallBounceLockTime_;
	}
}

void Player::OnCollision(const CollisionResult &result) {
	// 壁との衝突から跳ね返りを誘発（突進中のみ）
	if (!result.isHit) {
		return;
	}
	
	bool isWall = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Boss)) == false;
	if (isCharging_ && isWall) {
		// 接触法線を利用して跳ね返り
		Vector3 normal = result.contactNormal;
		// XZ成分で跳ね返り方向作成
		Vector3 bounceDir = { -normal.x, 0.0f, -normal.z };
		if (bounceDir.x != 0.0f || bounceDir.z != 0.0f) { bounceDir = Normalize(bounceDir); }
		// 強化壁判定 (例: penetrationDepthが大きい場合など仮)
		bool isGreat = false; // 今後必要なら result.userData から判定
		ChargeWallBounce(bounceDir, isGreat);
	}
}

void Player::RegisterBebugParam() {
	// 値の登録
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "JumpMaxHeight", kJumpHeight_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "JumpMaxTime", kJumpMaxTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "MoveSpeed", kMoveSpeed_);

	// プレイヤー共通
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "AirMoveSpeed", kAirMoveSpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "MaxFallSpeed", MaxFallSpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "AirAcceleration", AirAcceleration_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "GroundAcceleration", kGroundAcceleration_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "AirDeceleration", kAirDeceleration_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "FallAcceleration", kFallAcceleration_);

	// 突撃設定
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "PreChargeTime", kPreChargeTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "ChargeSpeed", kChargeSpeed_);

	// 通常壁跳ね返り設定
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceUpSpeed", kWallBounceUpSpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceAwaySpeed", kWallBounceAwaySpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceLockTime", kWallBounceLockTime_);

	// 強化壁跳ね返り設定
	GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "GreatWallBounceUpSpeed", kGreatWallBounceUpSpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "GreatWallBounceAwaySpeed", kGreatWallBounceAwaySpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "GreatWallBounceLockTime", kGreatWallBounceLockTime_);

	// Attack（空中急降下）設定
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "AttackPreDownTime", kAttackPreDownTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "AttackDownSpeed", kAttackDownSpeed_);
}

void Player::ApplyDebugParam() {
	// 値の適応
	kJumpHeight_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "JumpMaxHeight");
	kJumpMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "JumpMaxTime");
	kMoveSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "MoveSpeed");

	// プレイヤー共通
	kAirMoveSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "AirMoveSpeed");
	MaxFallSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "MaxFallSpeed");
	AirAcceleration_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "AirAcceleration");
	kGroundAcceleration_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "GroundAcceleration");
	kAirDeceleration_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "AirDeceleration");
	kFallAcceleration_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "FallAcceleration");

	// 突撃設定
	kPreChargeTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "PreChargeTime");
	kChargeSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "ChargeSpeed");

	// 通常壁跳ね返り設定
	kWallBounceUpSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceUpSpeed");
	kWallBounceAwaySpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceAwaySpeed");
	kWallBounceLockTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceLockTime");

	// 強化壁跳ね返り設定
	kGreatWallBounceUpSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "GreatWallBounceUpSpeed");
	kGreatWallBounceAwaySpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "GreatWallBounceAwaySpeed");
	kGreatWallBounceLockTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "GreatWallBounceLockTime");

	// Attack（空中急降下）設定
	kAttackPreDownTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "AttackPreDownTime");
	kAttackDownSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "AttackDownSpeed");
}