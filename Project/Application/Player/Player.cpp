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
#include"Application/Stage/Wall.h"
#include "LogManager.h"
#include "AudioManager.h"
using namespace GameEngine;

float Player::GetDamageFlashAlpha() const {
    if (!isInvincible_ || kDamageInvincibleTime_ <= 0.0f) { return 1.0f; }
    // Blink with 0.1s period
    float period = 0.1f;
    float t = std::fmod(std::max(damageInvincibleTimer_, 0.0f), period);
    return (t < period * 0.5f) ? 0.2f : 1.0f;
}

void Player::Initialize(GameEngine::Animator *animator, const std::array<std::map<std::string, AnimationData>, kPlayerAnimationCount>& animationData) {
	// アニメーション設定
	SetAnimator(animator);
	SetAnimationData(animationData);
	// ワールド行列を初期化
	worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f,-8.0f} });
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

	// オーディオハンドル取得
	audioHandle_PlayerDamaged_ = AudioManager::GetInstance().GetHandleByName("PlayerDamaged.mp3");
	audioHandle_RushCharge_ = AudioManager::GetInstance().GetHandleByName("RushCharge.mp3");
	audioHandle_RushLv1_ = AudioManager::GetInstance().GetHandleByName("Rush_lv1.mp3");
	audioHandle_RushLv2_ = AudioManager::GetInstance().GetHandleByName("Rush_lv2.mp3");
	audioHandle_RushLv3_ = AudioManager::GetInstance().GetHandleByName("Rush_lv3.mp3");
	audioHandle_AirMotion_ = AudioManager::GetInstance().GetHandleByName("airMotion.mp3");
	audioHandle_Reflect_ = AudioManager::GetInstance().GetHandleByName("Reflect.mp3");

	// 初期アニメーションをセット
	if (animator_) {
        PlayAnimation(PlayerAnimationType::Walk, "PlayerWalk");
	}

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

    // ダメージ無敵タイマー更新
    if (isInvincible_) {
        damageInvincibleTimer_ -= FpsCounter::deltaTime;
        if (damageInvincibleTimer_ <= 0.0f) {
            isInvincible_ = false;
            damageInvincibleTimer_ = 0.0f;
        }
    }

    // 急降下準備中の処理
    if (isAttackDownPrepping_) {
        // 準備時間を進める
        attackDownPrepareTimer_ += FpsCounter::deltaTime;
        float half = attackDownPrepareTotal_ * 0.5f;
        if (attackDownPrepareTimer_ <= half && half > 0.0f) {
            float t = std::clamp(attackDownPrepareTimer_ / half, 0.0f, 1.0f);
            float yOffset = EaseOutCubic(0.0f, 2.0f, t);
            worldTransform_.transform_.translate.y = attackDownPrepareStartY_ + yOffset;
        }
        // 準備完了で急降下開始
        if (attackDownPrepareTimer_ >= attackDownPrepareTotal_) {
            isAttackDownPrepping_ = false;
            // 実際の急降下開始
            isAttackDown_ = true;
            attackDownStartY_ = worldTransform_.transform_.translate.y;
        }
    }

	bounceAwayDir_ = bounceAwayDir_;
	desiredVelXZ_ = { 0.0f, 0.0f, 0.0f };
	BounceUpdate();
	ProcessMoveInput(inputCommand);
    ProcessAttackDownInput(inputCommand);
	HandleRushCharge(inputCommand);
	HandleRushStart(inputCommand);
	RushUpdate();

	// 重力（常時適応）
    velocity_.y += kFallAcceleration_ * FpsCounter::deltaTime * (isAttackDown_ ? 50.0f : 1.0f);
	// 縦方向の上限（落下最大速度）
	velocity_.y = std::max(velocity_.y, (isAttackDown_ ? -kAttackDownSpeed_ : -kMaxFallSpeed_));

	// 落下攻撃中は現在の落下速度から攻撃力を算出
	if (isAttackDown_) {
		if (useSpeedBasedAttackDown_) {
			float fallSpeed = -velocity_.y; // 正の値
			float ratio = 0.0f;
			if (kAttackDownSpeed_ > 0.00001f) {
				ratio = std::clamp(fallSpeed / kAttackDownSpeed_, 0.0f, 1.0f);
			}
			attackDownPower_ = Lerp(kAttackDownMinPower_, kAttackDownMaxPower_, ratio);
		} else {
			float currentY = worldTransform_.transform_.translate.y;
			float fallDistance = attackDownStartY_ - currentY;
			if (fallDistance < 0.0f) fallDistance = 0.0f;
			float ratio = 0.0f;
			if (kAttackDownDistanceToMax_ > 0.00001f) {
				ratio = std::clamp(fallDistance / kAttackDownDistanceToMax_, 0.0f, 1.0f);
			}
			attackDownPower_ = Lerp(kAttackDownMinPower_, kAttackDownMaxPower_, ratio);
		}
	}

	// 速度を適応
	worldTransform_.transform_.translate.x += velocity_.x * FpsCounter::deltaTime;
	worldTransform_.transform_.translate.y += velocity_.y * FpsCounter::deltaTime;
	worldTransform_.transform_.translate.z += velocity_.z * FpsCounter::deltaTime;

	// Yが-32以下になったら初期位置に戻す
    if (worldTransform_.transform_.translate.y < -32.0f) {
        worldTransform_.transform_.translate = { 0.0f, 1.0f, -8.0f };
    }

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

	// 行列の更新
	worldTransform_.UpdateTransformMatrix();

	// コライダー位置同期
	if (collider_) {
		collider_->SetWorldPosition(worldTransform_.transform_.translate);
        sphereData_.center = worldTransform_.transform_.translate;
	}

	// アニメーションの状態更新
	UpdateAnimation();
}

void Player::Restart() {
	// 位置リセット
	worldTransform_.transform_.translate = { -2.0f, 1.0f, 0.0f };
	worldTransform_.UpdateTransformMatrix();
	// 速度リセット
	velocity_ = { 0.0f, 0.0f, 0.0f };
	// 状態リセット
	isJump_ = false;
	jumpTimer_ = 0.0f;
	isCharging_ = false;
	chargeTimer_ = 0.0f;
	chargeRatio_ = 0.0f;
	rushChargeLevel_ = 0;
	isPreRushing_ = false;
	isRushing_ = false;
	isRushLock_ = false;
	rushLockTimer_ = 0.0f;
	rushTimer_ = 0.0f;
	rushActiveTimer_ = 0.0f;
	isBounceLock_ = false;
	bounceLockTimer_ = 0.0f;
	currentBounceUpSpeed_ = 0.0f;
	currentBounceAwaySpeed_ = 0.0f;
	currentBounceLockTime_ = 0.0f;
	bounceAwayDir_ = { 0.0f, 0.0f, 0.0f };
	isAttackDown_ = false;
	attackDownPower_ = 0.0f;
	// HPリセット
	currentHP_ = kMaxHP_;
	isAlive_ = true;
	// 無敵解除
	isInvincible_ = false;
	damageInvincibleTimer_ = 0.0f;
	// アニメーションリセット
    StartNormalAnim(PlayerAnimationType::Walk, "歩き", true);
    // 値の適応
    ApplyDebugParam();
}

void Player::UpdateAnimation() {
    // 突進のアニメーション終了検出
    if (currentAnimationType_ == PlayerAnimationType::Rush &&
        currentAnimationName_ == "突進_End" &&
        !isJump_) {
		float animTime = animator_->GetTimer();
		float animMaxTime = animator_->GetMaxTime();
		if (animTime >= animMaxTime) {
			isRushAnimEndTriggered_ = true;
		}
    }

	// 壁衝突でのバウンス硬直開始
	if (isBounceLock_ && !prevIsBounceLock_) {
		// 壁衝突時は End をアニメーション全体時間で再生(★)
		// 使用する時間は currentBounceLockTime_
		StartCustomAnim(PlayerAnimationType::Rush, "突進_End", std::max(currentBounceLockTime_, 0.001f));
	}

    // バウンス硬直終了後
    if (!isBounceLock_ && prevIsBounceLock_) {
		StartNormalAnim(PlayerAnimationType::AirMove, "AirMove", true);
		// Notify external listener when bounce lock ends
		if (onBounceLockEnd_) {
			// Only notify if the bounce was result of a max-level rush
			if (rushChargeLevel_ == 3) {
				onBounceLockEnd_();
			}
		}
	}

    // 空中急降下開始 (isAttackDown_ が true になった瞬間)
	if (isAttackDown_ && !prevIsAttackDown_) {
		//StartNormalAnim(PlayerAnimationType::DownAttack, "DownAttack_Prepare", false);
		// 空中移動音を停止
		if (audioHandle_AirMotion_ != 0) { AudioManager::GetInstance().Stop(audioHandle_AirMotion_); }
	}

    // 空中急降下終了 (isAttackDown_ が false になった瞬間)
	if (!isAttackDown_ && prevIsAttackDown_) {
		StartNormalAnim(PlayerAnimationType::AirMove, "AirMove", true);
        // 空中移動音を再生
        if (audioHandle_AirMotion_ != 0 && !AudioManager::GetInstance().IsPlay(audioHandle_AirMotion_)) { AudioManager::GetInstance().Play(audioHandle_AirMotion_, audioVolume_AirMotion_, true); }
    }

    // 着地判定: 落下中(false)->着地(true) の遷移で Walk を再生
    // or 突進アニメ終了後の復帰
    if ((!isJump_ && prevIsJump_) ||
        (isRushAnimEndTriggered_ && !isRushing_)) {
        StartNormalAnim(PlayerAnimationType::Walk, "歩き", true);
        // 着地したら空中移動音を停止
        if (audioHandle_AirMotion_ != 0) { AudioManager::GetInstance().Stop(audioHandle_AirMotion_); }
    }

	// 突進溜め開始時
	if (isCharging_ && !prevIsCharging_) {
		StartCustomAnim(PlayerAnimationType::Rush, "突進_Prepare",
			kRushChargeMaxTime_ * kRushChargeLevel3Ratio_);
		// 再生: RushCharge
		{
			auto handle = audioHandle_RushCharge_;
			if (handle != 0) { AudioManager::GetInstance().Play(handle, audioVolume_RushCharge_, false); }
		}
	}
    // 突進溜め終了時にループを停止
    if (!isCharging_ && prevIsCharging_) {
        auto handle = audioHandle_RushCharge_;
        if (handle != 0) { AudioManager::GetInstance().Stop(handle); }
    }

	// 予備が終わって突進が始まった瞬間
	if (isRushing_ && !prevIsRushing_) {
		// 突進開始: Main を通常再生
		StartNormalAnim(PlayerAnimationType::Rush, "突進_Main", false);
		// 突進開始時のレベル別SE
		{
			int lvl = rushChargeLevel_;
			auto h = (lvl == 3) ? audioHandle_RushLv3_ : (lvl == 2 ? audioHandle_RushLv2_ : audioHandle_RushLv1_);
			float vol = (lvl == 3) ? audioVolume_RushLv3_ : (lvl == 2 ? audioVolume_RushLv2_ : audioVolume_RushLv1_);
			if (h != 0) { AudioManager::GetInstance().Play(h, vol, false); }
		}
	}

	// 突進終了時
	if (!isRushing_ && prevIsRushing_) {
		// 突進終了: End を通常再生
		StartNormalAnim(PlayerAnimationType::Rush, "突進_End", false);
	}

	// カスタム再生時間が有効なら進めて、終了で次の再生を決める
	if (animCustomActive_) {
		animator_->NormalizeUpdate(animCustomTotal_ > 0.0f ? (animCustomTimer_ / animCustomTotal_) : 1.0f);
		animCustomTimer_ += FpsCounter::deltaTime;
    } else {
		// 通常更新
		animator_->Update();
    }

    // 空中移動中の音再生管理: 空中でかつ急降下でない場合はループ音を再生
    {
        auto h = audioHandle_AirMotion_;
        if (h != 0) {
            if (isJump_ && !isAttackDown_) {
                if (!AudioManager::GetInstance().IsPlay(h)) { AudioManager::GetInstance().Play(h, audioVolume_AirMotion_, true); }
            } else {
                if (AudioManager::GetInstance().IsPlay(h)) { AudioManager::GetInstance().Stop(h); }
            }
        }
    }

    // prev 状態更新
    prevIsPreRushing_ = isPreRushing_;
    prevIsRushing_ = isRushing_;
    prevIsBounceLock_ = isBounceLock_;
    prevIsAttackDown_ = isAttackDown_;
    prevIsCharging_ = isCharging_;
    prevIsJump_ = isJump_;
    isRushAnimEndTriggered_ = false;
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
        if (inputCommand && isCharging_) {
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
		if (inputCommand) {
			if (inputCommand->IsCommandActive("MoveUp")) { dir -= cameraForwardXZ_; }
			if (inputCommand->IsCommandActive("MoveDown")) { dir += cameraForwardXZ_; }
			if (inputCommand->IsCommandActive("MoveLeft")) { dir -= cameraRightXZ_; }
			if (inputCommand->IsCommandActive("MoveRight")) { dir += cameraRightXZ_; }
		}
		if (dir.x != 0.0f || dir.z != 0.0f) {
			// Y成分は常に0
			dir.y = 0.0f;
			dir = Normalize(dir);
			lastMoveDir_ = { dir.x, 0.0f, dir.z };
		}
		float maxSpeed = (isJump_ ? kAirMoveSpeed_ : kMoveSpeed_);
		// 入力から算出される到達可能最大速度（XZのみ）。ここで既に上限を規定するため、直接 velocity を clamp する必要はない。
		// 変更: 入力による加速が既に現在の移動速度を超えないようにする。ただし、既存のvelocityが
		// 既にmaxSpeedを超えている場合はそのままにし、inputによってさらに増加させないようにする。
		if (dir.x != 0.0f || dir.z != 0.0f) {
			// 現在の水平速度を取得
			Vector3 horiz = { velocity_.x, 0.0f, velocity_.z };
			// dirは正規化済み
			Vector3 dirNorm = dir;
			float proj = horiz.x * dirNorm.x + horiz.z * dirNorm.z; // 現在速度のdir方向成分
			float horizLen = Length(horiz);
			if (horizLen >= maxSpeed) {
				// 既に水平速度がmaxSpeed以上あるなら、入力でそれ以上増やさない
				desiredVelXZ_.x = dirNorm.x * proj;
				desiredVelXZ_.z = dirNorm.z * proj;
			} else {
				// まだmax未満なら通常通り目標速度を設定
				desiredVelXZ_.x = dir.x * maxSpeed;
				desiredVelXZ_.z = dir.z * maxSpeed;
			}
		} else {
			desiredVelXZ_.x = 0.0f;
			desiredVelXZ_.z = 0.0f;
		}
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
}

void Player::ProcessAttackDownInput(GameEngine::InputCommand *inputCommand) {
    if (isCharging_ || isPreRushing_ || isRushing_ || isBounceLock_ || isRushLock_ || !isJump_ || isAttackDownPrepping_) {
        return;
    }
	// 攻撃下降開始
	if (inputCommand && inputCommand->IsCommandActive("AttackDown")) {
		velocity_.x = 0.0f;
		velocity_.z = 0.0f;
		attackDownPower_ = 0.0f;
        // 準備状態に入る（実際の落下は後で開始）
        isAttackDownPrepping_ = true;
        attackDownPrepareTimer_ = 0.0f;
        attackDownPrepareStartY_ = worldTransform_.transform_.translate.y;
        // アニメーション時間を取得して総待機時間に設定
        // DownAttack_Prepare のアニメーションデータから duration を拾う
        auto &mapRef = animationData_[static_cast<size_t>(PlayerAnimationType::DownAttack)];
        auto it = mapRef.find("DownAttack_Prepare");
        if (it != mapRef.end()) {
            attackDownPrepareTotal_ = it->second.duration;
        } else {
            attackDownPrepareTotal_ = kAttackPreDownTime_; // フォールバック
        }
        // アニメーション再生
        StartNormalAnim(PlayerAnimationType::DownAttack, "DownAttack_Prepare", false);
    }
}

void Player::HandleRushCharge(GameEngine::InputCommand* inputCommand) {
    if (isCharging_ || isJump_ || isRushing_ || isPreRushing_ ||
        isBounceLock_ || isRushLock_ || isRushCooldown_) {
		return;
	}
	// 溜め開始/継続
	if (inputCommand && inputCommand->IsCommandActive("RushCharge")) {
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
	if (inputCommand && inputCommand->IsCommandActive("RushStart")) {
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
		rushTimer_ = 0.0f;
	}
}

void Player::RushUpdate() {
	if (isRushCooldown_) {
		rushLockTimer_ -= FpsCounter::deltaTime;
		if (rushLockTimer_ <= 0.0f) {
			isRushCooldown_ = false;
			rushLockTimer_ = 0.0f;
		}
	}

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
			// 壁に当たらず通常終了した場合はクールダウンを開始
			if (!isBounceLock_) {
                isRushCooldown_ = true;
				rushLockTimer_ = kRushCooldownTime_;
			}
		}
	}
}

void Player::BounceUpdate() {
	if (!isBounceLock_) { return; }
	isRushing_ = false;
	bounceLockTimer_ += FpsCounter::deltaTime;
	if (bounceLockTimer_ >= currentBounceLockTime_) {
		isBounceLock_ = false; bounceLockTimer_ = 0.0f; currentBounceUpSpeed_ = 0.0f; currentBounceAwaySpeed_ = 0.0f; currentBounceLockTime_ = 0.0f; bounceAwayDir_ = {0.0f,0.0f,0.0f};
		if (onBounceLockEnd_) {
			if (rushChargeLevel_ == 3) {
				onBounceLockEnd_();
			}
		}
	}
}

void Player::Bounce(const Vector3 &bounceDirection, float bounceStrength, bool isIceFall) {
	if (isPreRushing_ || !isRushing_) { return; }
	isPreRushing_ = false; isJump_ = true;
	isBounceLock_ = true; bounceLockTimer_ = 0.0f; bounceAwayDir_ = { -bounceDirection.x, 0.0f, -bounceDirection.z }; bounceAwayDir_ = Normalize(bounceAwayDir_);

    // 壁跳ね返り音
    if (audioHandle_Reflect_ != 0) {
		AudioManager::GetInstance().Play(audioHandle_Reflect_, audioVolume_Reflect_, false);
	}

    if (isLegacyWallBounce_) {
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
	
	} else {
		// 変更: 跳ね返る高さを直前の水平速度に応じて変化させる
		// 現在の水平速度を取得
		Vector3 prevHoriz = { velocity_.x, 0.0f, velocity_.z };
		float prevSpeed = Length(prevHoriz);
		// 速度比 (0..1) を計算（kRushMaxSpeed_ が 0 の場合は 0 にする）
		float speedRatio = 0.0f;
		if (kRushMaxSpeed_ > 0.00001f) {
			speedRatio = std::clamp(prevSpeed / kRushMaxSpeed_, 0.0f, 1.0f);
		}
		// 高さ倍率を決定
		float heightMultiplier = Lerp(kWallBounceMinSpeedFactor_, kWallBounceMaxSpeedFactor_, speedRatio);

		// 水平方向の反発速度は従来通り（強さでスケール）
		velocity_ = bounceAwayDir_ * (kWallBounceAwaySpeed_ * bounceStrength);
		// 上方向速度は強さと速度倍率に応じて変化
		velocity_.y = kWallBounceUpSpeed_ * bounceStrength * heightMultiplier;
		currentBounceLockTime_ = kWallBounceLockTime_;
	}
}

void Player::OnCollision(const CollisionResult &result) {
	if (!result.isHit) return;

	bool isWall = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Wall))
		&& dynamic_cast<Wall *>(result.userData.object)->GetIsAlive();
	bool isIceFall = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::IceFall));
	bool isBoundary = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::BoundaryWall))
        || (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Wall) && !dynamic_cast<Wall *>(result.userData.object)->GetIsAlive());
	bool isGround = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Ground));
    bool isBoss = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Boss));
    bool isWind = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Wind));

    // ボスとの衝突処理（突進中の場合）
    if (isBoss && isRushing_) {
        Log("is hit Boss with Rushing");
        Vector3 normal = result.contactNormal;
        Vector3 bounceDir = { -normal.x, 0.0f, -normal.z };
        if (bounceDir.x != 0.0f || bounceDir.z != 0.0f) { bounceDir = Normalize(bounceDir); }
        if (onWallHit_) { onWallHit_(); }
        Bounce(bounceDir, kBossBounceReflectFactor_);
        return;
    }

    // ボスとの衝突処理（空中急降下中の場合）
    if (isBoss && isAttackDown_) {
        Log("is hit Boss with Attack Down" + std::to_string(attackDownPower_));
        //if (collider_) { collider_->SetWorldPosition(worldTransform_.transform_.translate); }
        return;
    }

    // ボスとの衝突処理（通常時）
    // or 風攻撃の場合
    if ((isBoss && !isRushing_ && !isAttackDown_ && !isInvincible_ && !isBounceLock_) ||
        (isWind && !isAttackDown_ && !isInvincible_)) {
		Log("is hit Boss normally");
		// HP減少処理（仮で1ダメージ）
		currentHP_ -= 1;
		if (currentHP_ <= 0) {
			currentHP_ = 0;
            isAlive_ = false;
		}
		// ダメージ無敵時間開始
		damageInvincibleTimer_ = kDamageInvincibleTime_;
		isInvincible_ = true;

		// 通知コールバック
		if (onDamaged_) { onDamaged_(); }

		// ダメージ時のSE再生
        if (audioHandle_PlayerDamaged_ != 0) {
			AudioManager::GetInstance().Play(audioHandle_PlayerDamaged_, audioVolume_PlayerDamaged_, false);
		}

		if (collider_) { collider_->SetWorldPosition(worldTransform_.transform_.translate); }
		return;
	}

	// 壁との衝突処理
    if (isWall && isRushing_) {
		Log("is hit Wall");
		Vector3 normal = result.contactNormal;
		Vector3 bounceDir = { -normal.x, 0.0f, -normal.z };
		if (bounceDir.x != 0.0f || bounceDir.z != 0.0f) { bounceDir = Normalize(bounceDir); }
		if (onWallHit_) { onWallHit_(); }
		Bounce(bounceDir, kWallBounceReflectFactor_);
		return;
	}

    if (isWall && !isRushing_ && !isJump_) {
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
		return;
	}

	// 氷柱との衝突処理
	if (isIceFall && isRushing_) {
		Vector3 normal = result.contactNormal;
		Vector3 bounceDir = { -normal.x, 0.0f, -normal.z };
		if (bounceDir.x != 0.0f || bounceDir.z != 0.0f) { bounceDir = Normalize(bounceDir); }
		if (onWallHit_) { onWallHit_(); }
		Bounce(bounceDir, kIceWallBounceReflectFactor_, true);
		return;
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
		return;
	}

	// BoundaryWall の処理: 通常の壁が存在する場合は無視する
    if (isBoundary || (isWall && isJump_)) {
		Vector3 n = result.contactNormal;
		if (n.x != 0.0f || n.y != 0.0f || n.z != 0.0f) { n = Normalize(n); }
		float depth = std::max(result.penetrationDepth, 0.0f);
		Vector3 correction = n * depth;
		worldTransform_.transform_.translate.x += correction.x;
		worldTransform_.transform_.translate.y += correction.y;
		worldTransform_.transform_.translate.z += correction.z;
		if (collider_) { collider_->SetWorldPosition(worldTransform_.transform_.translate); }
		return;
	}

	if (isGround) {
		// 地面にめり込んでいる分だけ押し戻す
		Vector3 n = result.contactNormal;
		if (n.x != 0.0f || n.y != 0.0f || n.z != 0.0f) { n = Normalize(n); }
		float depth = std::max(result.penetrationDepth, 0.0f);
		Vector3 correction = n * depth;
		worldTransform_.transform_.translate.x -= correction.x;
		worldTransform_.transform_.translate.y -= correction.y;
		worldTransform_.transform_.translate.z -= correction.z;
		if (velocity_.y < 0.0f) {
			velocity_.y = 0.0f;
			attackDownPower_ = 0.0f;
			if (isAttackDown_ && onLandHit_) {
				onLandHit_();
			}
		}
		if (!isRushing_ && !isBounceLock_) {
			isJump_ = false;
			isAttackDown_ = false;
		}
		if (collider_) { collider_->SetWorldPosition(worldTransform_.transform_.translate); }
		return;
	}
}

void Player::PlayAnimation(PlayerAnimationType type, const std::string &name) {
	if (!animator_) return;
	size_t idx = static_cast<size_t>(type);
	if (idx >= animationData_.size()) return;
	const auto &mapRef = animationData_[idx];
	auto it = mapRef.find(name);
	if (it == mapRef.end()) return;
	animator_->SetAnimationData(&it->second);
	animator_->SetTimer(0.0f);
	currentAnimationType_ = type;
	currentAnimationName_ = name;
    Log("Play Animation: " + name);
}

void Player::StartNormalAnim(PlayerAnimationType type, const std::string &name, bool loop) {
	PlayAnimation(type, name);
	if (animator_) {
		animator_->SetIsLoop(loop);
		animCustomActive_ = false;
	}
}

void Player::StartCustomAnim(PlayerAnimationType type, const std::string &name, float totalDuration) {
	PlayAnimation(type, name);
	if (!animator_) return;
	animator_->SetIsLoop(false);
	animCustomActive_ = true;
	animCustomTimer_ = 0.0f;
	// 優先的に外部指定時間を使う
	animCustomTotal_ = totalDuration;
	// もしanimationData_に該当する実時間が取れるなら格納しておく
	size_t idx = static_cast<size_t>(type);
	if (idx < animationData_.size()) {
		const auto &m = animationData_[idx];
		auto it = m.find(name);
		if (it != m.end()) {
			animTargetAnimMaxTime_ = it->second.duration;
		}
	}
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
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "GroundDeceleration", kGroundDeceleration_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "RotationLerpSpeed", kRotationLerpSpeed_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "MaxHP", kMaxHP_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[0], "DamageInvincibleTime", kDamageInvincibleTime_);

	// 突撃設定
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "PreRushTime", kPreRushMaxTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushSpeed", kRushMaxSpeed_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushChargeLevel1Ratio", kRushChargeLevel1Ratio_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushChargeLevel2Ratio", kRushChargeLevel2Ratio_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushChargeLevel3Ratio", kRushChargeLevel3Ratio_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushLockMaxTime", kRushLockMaxTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushChargeMaxTime", kRushChargeMaxTime_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushCooldownTime", kRushCooldownTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushStrengthLevel1", kRushStrengthLevel1_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushStrengthLevel2", kRushStrengthLevel2_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[1], "RushStrengthLevel3", kRushStrengthLevel3_);
    
	// 通常壁跳ね返り設定
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceUpSpeed", kWallBounceUpSpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceAwaySpeed", kWallBounceAwaySpeed_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceLockTime", kWallBounceLockTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceStrengthLevel1", kWallBounceStrengthLevel1_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceStrengthLevel2", kWallBounceStrengthLevel2_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceStrengthLevel3", kWallBounceStrengthLevel3_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceMinSpeedFactor", kWallBounceMinSpeedFactor_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceMaxSpeedFactor", kWallBounceMaxSpeedFactor_);
    // New: reflect factor parameters for different collision types
    GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "WallBounceReflectFactor", kWallBounceReflectFactor_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "IceWallBounceReflectFactor", kIceWallBounceReflectFactor_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "BossBounceReflectFactor", kBossBounceReflectFactor_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[2], "IsLegacyWallBounce", isLegacyWallBounce_);

	// Attack（空中急降下）設定
    GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "AttackPreDownTime", kAttackPreDownTime_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "AttackDownSpeed", kAttackDownSpeed_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "AttackDownMinPower", kAttackDownMinPower_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "AttackDownMaxPower", kAttackDownMaxPower_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "AttackDownDistanceToMax", kAttackDownDistanceToMax_);
    GameParamEditor::GetInstance()->AddItem(kGroupNames[3], "UseSpeedBasedAttackDown", useSpeedBasedAttackDown_);

	// Audio 設定 (Player-Audio)
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "PlayerDamagedVolume", audioVolume_PlayerDamaged_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "RushChargeVolume", audioVolume_RushCharge_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "RushLv1Volume", audioVolume_RushLv1_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "RushLv2Volume", audioVolume_RushLv2_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "RushLv3Volume", audioVolume_RushLv3_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "AirMotionVolume", audioVolume_AirMotion_);
	GameParamEditor::GetInstance()->AddItem(kGroupNames[4], "ReflectVolume", audioVolume_Reflect_);
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
	kWallHitReflectFactor_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "WallHitReflectFactor");
	kGroundDeceleration_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "GroundDeceleration");
	kRotationLerpSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "RotationLerpSpeed");
    kMaxHP_ = GameParamEditor::GetInstance()->GetValue<int32_t>(kGroupNames[0], "MaxHP");
    kDamageInvincibleTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[0], "DamageInvincibleTime");

	// 突撃設定
	kPreRushMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "PreRushTime");
	kRushMaxSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushSpeed");
    kRushChargeLevel1Ratio_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushChargeLevel1Ratio");
    kRushChargeLevel2Ratio_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushChargeLevel2Ratio");
    kRushChargeLevel3Ratio_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushChargeLevel3Ratio");
	kRushLockMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushLockMaxTime");
	kRushChargeMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushChargeMaxTime");
    kRushCooldownTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushCooldownTime");
	kRushStrengthLevel1_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushStrengthLevel1");
	kRushStrengthLevel2_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushStrengthLevel2");
	kRushStrengthLevel3_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[1], "RushStrengthLevel3");

	// 通常壁跳ね返り設定
	kWallBounceUpSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceUpSpeed");
	kWallBounceAwaySpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceAwaySpeed");
	kWallBounceLockTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceLockTime");
	kWallBounceStrengthLevel1_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceStrengthLevel1");
	kWallBounceStrengthLevel2_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceStrengthLevel2");
	kWallBounceStrengthLevel3_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceStrengthLevel3");
    kWallBounceMinSpeedFactor_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceMinSpeedFactor");
    kWallBounceMaxSpeedFactor_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceMaxSpeedFactor");
    // New: reflect factors
    kWallBounceReflectFactor_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "WallBounceReflectFactor");
    kIceWallBounceReflectFactor_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "IceWallBounceReflectFactor");
    kBossBounceReflectFactor_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[2], "BossBounceReflectFactor");
    isLegacyWallBounce_ = GameParamEditor::GetInstance()->GetValue<bool>(kGroupNames[2], "IsLegacyWallBounce");

	// Attack（空中急降下）設定
	kAttackPreDownTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "AttackPreDownTime");
	kAttackDownSpeed_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "AttackDownSpeed");
	kAttackDownMinPower_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "AttackDownMinPower");
	kAttackDownMaxPower_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "AttackDownMaxPower");
    kAttackDownDistanceToMax_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[3], "AttackDownDistanceToMax");
    useSpeedBasedAttackDown_ = GameParamEditor::GetInstance()->GetValue<bool>(kGroupNames[3], "UseSpeedBasedAttackDown");

	// Audio パラメータ取得
	audioVolume_PlayerDamaged_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "PlayerDamagedVolume");
	audioVolume_RushCharge_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "RushChargeVolume");
	audioVolume_RushLv1_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "RushLv1Volume");
	audioVolume_RushLv2_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "RushLv2Volume");
	audioVolume_RushLv3_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "RushLv3Volume");
	audioVolume_AirMotion_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "AirMotionVolume");
	audioVolume_Reflect_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames[4], "ReflectVolume");
}