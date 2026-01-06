#define NOMINMAX
#include"Player.h"
#include<algorithm>
#include<numbers>
#include"GameParamEditor.h"
#include"EasingManager.h"
#include"MyMath.h"
#include"FPSCounter.h"
#include"LogManager.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
#include"AudioManager.h"
using namespace GameEngine;

void Player::Initialize(GameEngine::InputCommand* inputCommand) {

	// 入力処理を受け取る
	inputCommand_ = inputCommand;

	// ワールド行列を初期化
	worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{-2.0f,1.0f,-10.0f} });

	// 武器をプレイヤーに追従
	weapon_->SetOwnerPosition(&worldTransform_);
	weaponTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	weapon_->SetTransform(weaponTransform_);

	// コンボの設定
	kConstAttacks_[0].maxTime = 0.6f;
	kConstAttacks_[0].radius = 3.0f;
	kConstAttacks_[1].maxTime = 0.8f;
	kConstAttacks_[1].radius = 3.0f;
	kConstAttacks_[2].maxTime = 0.6f;
	kConstAttacks_[2].radius = 5.0f;

	// 当たり判定を設定
	collider_ = std::make_unique<SphereCollider>();
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
	collider_->SetRadius(kCollisionRadius_);
	collider_->SetCollisionAttribute(kCollisionAttributePlayer);
	collider_->SetCollisionMask(~kCollisionAttributePlayer);

	// 当たり判定システムにプレイヤーのデータを設定する
	UserData userData;
	userData.typeID = static_cast<uint32_t>(CollisionTypeID::Player);
	userData.object = this;
	collider_->SetUserData(userData);

	// コールバック関数を登録する
	collider_->SetOnCollisionEnterCallback([this](const CollisionResult& result) {
		this->OnCollisionEnter(result);
	});
	collider_->SetOnCollisionCallback([this](const CollisionResult& result) {
		this->OnCollisionStay(result);
	});
	// プレイヤーの更新状態を設定する
	behaviorsTable_ = {
		[this]() { NormalUpdate(); },
		[this]() { AttackUpdate(); },
		[this]() { JumpUpdate(); },
		[this]() { DushUpdate(); }
	};

	// プレイヤーの各状態のリセット処理を設定する
	resetBehaviorParamTable_ = {
		[this]() {}, // 通常
		[this]() {}, // 攻撃
		[this]() { jumpTimer_ = 0.0f; }, // ジャンプ
		[this]() { dushTimer_ = 0.0f; }, // ダッシュ
	};

	// 音
	dushSH_ = AudioManager::GetInstance().GetHandleByName("playerDush.mp3");
	landSH_ = AudioManager::GetInstance().GetHandleByName("playerLand.mp3");
	hitSH_ = AudioManager::GetInstance().GetHandleByName("playerHit.mp3");
	swingSH_ = AudioManager::GetInstance().GetHandleByName("playerSwing.mp3");

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
	ApplyDebugParam();
#else
	// 値を適応させる
	ApplyDebugParam();
#endif
}

void Player::Update() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

	isAttack_ = false;

	// 状態遷移のリクエストがあった場合、切り替える処理
	if (behaviorRequest_) {
		// 状態を変更
		behavior_ = behaviorRequest_.value();
		// 振る舞いによるリセットを呼び出す
		resetBehaviorParamTable_[static_cast<size_t>(behavior_)]();
		// 振る舞いのリクエストをクリア
		behaviorRequest_ = std::nullopt;
	}

	// 移動のリセット
	move = {0.0f,0.0f,0.0f};
	velocity_ = { 0.0f,0.0f,0.0f };
	isMove = false;

	// プレイヤーの入力処理
	ProcessMoveInput();

	// プレイヤーの状態による更新処理をおこなう
	behaviorsTable_[static_cast<size_t>(behavior_)]();

	// プレイヤーを移動範囲に制限
	worldTransform_.transform_.translate.x = std::clamp(worldTransform_.transform_.translate.x,-29.0f,29.0f);
	worldTransform_.transform_.translate.z = std::clamp(worldTransform_.transform_.translate.z, -29.0f, 29.0f);

	// ダメージを受けた時のノックバック処理
	if (knockbackSpeed_ > 0.0f) {
		knockbackSpeed_ -= 30.0f * FpsCounter::deltaTime / 0.5f;
		velocity_ += hitDirection_* -knockbackSpeed_;
	}
	
	// 行列の更新
	worldTransform_.transform_.translate += velocity_ * FpsCounter::deltaTime;
	worldTransform_.UpdateTransformMatrix();

	// 当たり判定の位置を更新
	collider_->SetWorldPosition(worldTransform_.transform_.translate);

	// ヒット時のクールタイム
	if (isHit_) {

		hitTimer_ += FpsCounter::deltaTime / kHitCoolTime_;

		if (static_cast<int>(hitTimer_ * 20.0f) % 2 == 0) {
			alpha_ = 1.0f;
		} else {
			alpha_ = 0.5f;
		}

		if (hitTimer_ >= 1.0f) {
			alpha_ = 1.0f;
			isHit_ = false;
			hitTimer_ = 0.0f;
		}
	}
}

void Player::ProcessMoveInput() {

	// プレイヤーの移動操作
	if (inputCommand_->IsCommandActive("MoveUp")) {
		move.z = 1.0f;
		isMove = true;
	}

	if (inputCommand_->IsCommandActive("MoveDown")) {
		move.z = -1.0f;
		isMove = true;
	}

	if (inputCommand_->IsCommandActive("MoveLeft")) {
		move.x = -1.0f;
		isMove = true;
	}

	if (inputCommand_->IsCommandActive("MoveRight")) {
		move.x = 1.0f;
		isMove = true;
	}

	// ジャンプ操作
	if (inputCommand_->IsCommandActive("Jump")) {
		if (behavior_ == Behavior::Normal) {
			behaviorRequest_ = Behavior::Jump;
		}
	}

	// ダッシュ操作
	if (inputCommand_->IsCommandActive("Dush")) {
		if (behavior_ == Behavior::Normal) {
			behaviorRequest_ = Behavior::Dush;

			// ダッシュ音
			AudioManager::GetInstance().Play(dushSH_, 0.5f, false);

			// ダッシュの向く方向を設定する
			dushDirection_ = { 0.0f,0.0f,1.0f };
			Matrix4x4 worldMatrix = worldTransform_.GetWorldMatrix();
			worldMatrix.m[3][0] = 0.0f;
			worldMatrix.m[3][1] = 0.0f;
			worldMatrix.m[3][2] = 0.0f;
			dushDirection_ = TransformNormal(dushDirection_, worldMatrix);
		}
	}

	// 攻撃操作
	if (inputCommand_->IsCommandActive("Attack")) {
		if (behavior_ == Behavior::Normal) {
			workAttack_.comboIndex = 0;
			workAttack_.timer_ = 0.0f;
			workAttack_.isComboNext = false;
			behaviorRequest_ = Behavior::Attack;

			// 武器を振る音
			AudioManager::GetInstance().Play(swingSH_, 0.5f, false);

		} else if (behavior_ == Behavior::Attack) {
			// コンボ上限に達していない時
			if (workAttack_.comboIndex < kComboNum - 1) {
				if (!workAttack_.isComboNext) {
					// 武器を振る音
					AudioManager::GetInstance().Play(swingSH_, 0.8f, false);
					workAttack_.isComboNext = true;
				}
			}
		}
	}
}

void Player::Move() {
	// 移動
	if (isMove) {
		// 正規化する
		move = Normalize(move);
		move = TransformNormal(move, rotateMatrix_);
		move.y = 0.0f;
		move = Normalize(move);
		// 移動する
		velocity_ = move * kMoveSpeed_;

		// 角度を設定する
		float tmpRotateY = std::atan2f(move.x, move.z);

		Vector3 currentRot = worldTransform_.transform_.rotate;

		// 最短距離の角度を取得
		float diffY = GetShortAngleY(tmpRotateY - currentRot.y);

		// 回転
		currentRot.y += diffY * 5.0f * FpsCounter::deltaTime;

		targetRotateY_ = currentRot.y;

	} else {

		// カメラのロックオンが有効かつ操作がされていなければ
		if (isCameraLockOn_) {
		
			// 目標へのベクトルを求める
			Vector3 toTarget = targetPos_ - worldTransform_.GetWorldPosition();
			toTarget.y = 0.0f;
			toTarget = Normalize(toTarget);

			Vector3 targetRot = { 0, 0, 0 };
			// Y軸回転を取得
			targetRot.y = atan2f(toTarget.x, toTarget.z);
			Vector3 currentRot = worldTransform_.transform_.rotate;

			// 最短距離の角度を取得
			float diffY = GetShortAngleY(targetRot.y - currentRot.y);

			// 回転
			currentRot.y += diffY * 5.0f * FpsCounter::deltaTime;

			targetRotateY_ = currentRot.y;
		}
	}

	// 旋回処理
	worldTransform_.transform_.rotate.y = targetRotateY_;
	//if (turnTimer_ < 1.0f) {
	//	turnTimer_ += FpsCounter::deltaTime / kTurnTime_;
	//	// Y軸周りの角度
	//	worldTransform_.transform_.rotate.y = LerpShortAngle(worldTransform_.transform_.rotate.y, targetRotateY_, turnTimer_);
	//}
}

void Player::NormalUpdate() {
	// 移動処理
	Move();
}

void Player::JumpUpdate() {

	// 移動処理
	Move();

	// ジャンプの移動処理
	jumpTimer_ += FpsCounter::deltaTime / kJumpMaxTime_;

	// ジャンプ処理
	if (jumpTimer_ <= 0.5f) {
		// ジャンプの上昇
		float jumpUpTimer_ = jumpTimer_ / 0.5f;
		worldTransform_.transform_.translate.y = Lerp(1.0f, kJumpHeight_, EaseOut(jumpUpTimer_));
	} else {
		// ジャンプの下降
		float jumpDownTimer_ = (jumpTimer_ - 0.5f) / 0.5f;
		worldTransform_.transform_.translate.y = Lerp(kJumpHeight_, 1.0f, EaseIn(jumpDownTimer_));
	}

	// 時間がたったら通常状態へ遷移
	if (jumpTimer_ >= 1.0f) {
		behaviorRequest_ = Behavior::Normal;
	}
}

void Player::AttackUpdate() {

	workAttack_.timer_ += FpsCounter::deltaTime / kConstAttacks_[workAttack_.comboIndex].maxTime;

	// コンボ段階によってモーション分岐
	switch (workAttack_.comboIndex)
	{
		// 縦に切る
	case 0:
		// 回転させる
		theta_ = Lerp(std::numbers::pi_v<float> *0.5f, 0.0f, EaseIn(workAttack_.timer_));
		// 武器を移動させる
		weaponTransform_.translate.x = 0.0f;
		weaponTransform_.translate.z = std::cosf(theta_) * kConstAttacks_[workAttack_.comboIndex].radius;
		weaponTransform_.translate.y = std::sinf(theta_) * kConstAttacks_[workAttack_.comboIndex].radius;
		// 武器を回転させる
		weaponTransform_.rotate.x = std::numbers::pi_v<float> *0.5f - theta_;
		weaponTransform_.rotate.y = 0.0f;
		weaponTransform_.rotate.z = 0.0f;
		break;

		// 斜め切り
	case 1:
		theta_ = Lerp(-std::numbers::pi_v<float> *0.6f, std::numbers::pi_v<float> *0.6f, EaseIn(workAttack_.timer_));
		// 武器を移動させる
		weaponTransform_.translate.x = std::sinf(theta_) * kConstAttacks_[workAttack_.comboIndex].radius;
		weaponTransform_.translate.y = std::sinf(theta_) * kConstAttacks_[workAttack_.comboIndex].radius;
		weaponTransform_.translate.z = std::cosf(theta_) * kConstAttacks_[workAttack_.comboIndex].radius;
		// 武器を回転させる
		weaponTransform_.rotate.x = 0.0f;
		weaponTransform_.rotate.y = theta_;
		weaponTransform_.rotate.z = std::numbers::pi_v<float> / 3.0f;
		break;

		// 横切り
	case 2:
		theta_ = Lerp(-std::numbers::pi_v<float> *0.6f, std::numbers::pi_v<float> *0.6f, EaseIn(workAttack_.timer_));
		// 武器を移動させる
		weaponTransform_.translate.x = std::sinf(theta_) * kConstAttacks_[workAttack_.comboIndex].radius;
		weaponTransform_.translate.y = 0.5f;
		weaponTransform_.translate.z = std::cosf(theta_) * kConstAttacks_[workAttack_.comboIndex].radius;
		// 武器を回転させる
		weaponTransform_.rotate.x = 0.0f;
		weaponTransform_.rotate.y = theta_;
		weaponTransform_.rotate.z = std::numbers::pi_v<float> *0.5f;
		break;
	}

	// 切り替え処理
	if (workAttack_.timer_ >= 1.0f) {

		// コンボ継続の場合
		if (workAttack_.isComboNext) {
			// リセット
			workAttack_.isComboNext = false;
			workAttack_.timer_ = 0.0f;
			workAttack_.comboIndex++;
		} else {
			// コンボを継続しない場合、元に戻る
			workAttack_.comboIndex = 0;
			workAttack_.timer_ = 0.0f;
			behaviorRequest_ = Behavior::Normal;
		}
	}	

	// 武器の更新処理
	weapon_->SetTransform(weaponTransform_);
	weapon_->Update();
}

void Player::DushUpdate() {

	// ダッシュ移動処理
	dushTimer_ += FpsCounter::deltaTime / kDushMaxTime_;

	// 移動
	velocity_ = dushDirection_ * kDushSpeed_;

	// 時間がたったら通常状態へ遷移
	if (dushTimer_ >= kDushMaxTime_) {
		behaviorRequest_ = Behavior::Normal;
	}
}

void Player::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {
	// ヒットログを出す
	Log("IsPlayerHit", "Player");

	//bool isBoss = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Boss));
	//
	//if (isBoss) {
	//	if (behavior_ != Behavior::Jump) {
	//		isHit_ = true;
	//	} else {
	//		isAttack_ = true;
	//	}
	//
	//	// ヒット音声
	//	AudioManager::GetInstance().Play(hitSH_, 0.5f, false);
	//
	//	knockbackSpeed_ = 30.0f;
	//	hitDirection_ = Vector3(result.contactNormal.x, 0.0f, result.contactNormal.z);
	//}
}

void Player::OnCollisionStay([[maybe_unused]] const GameEngine::CollisionResult& result) {
	bool isWall = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Wall));
	bool isBoss = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Boss));
	bool isEnemyBullet = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::EnemyBullet));

	// ボスに当たった時の処理
	if (isBoss && !isHit_) {
		if (behavior_ != Behavior::Dush) {
			isHit_ = true;
			isHitEffect_ = true;
			// ヒット音声
			AudioManager::GetInstance().Play(hitSH_, 0.5f, false);

			// hpを減らす
			if (hp_ > 0) {
				hp_ -= 1;
			}

			// ノックバック
			knockbackSpeed_ = 30.0f;
			hitDirection_ = Vector3(result.contactNormal.x, 0.0f, result.contactNormal.z);
		}
	}

	// 敵の弾に当たった時
	if (isEnemyBullet) {
		if (behavior_ != Behavior::Dush) {
			isHit_ = true;
			isHitEffect_ = true;
			// ヒット音声
			AudioManager::GetInstance().Play(hitSH_, 0.5f, false);

			// hpを減らす
			if (hp_ > 0) {
				hp_ -= 1;
			}
		}
	}

	// 壁に当たった時、押し戻す
	if (isWall) {
		Vector3 n = result.contactNormal;
		Vector3 nXZ = { n.x,0.0f,n.z };
		if (nXZ.x != 0.0f || nXZ.z != 0.0f) { nXZ = Normalize(nXZ); }
		float depth = std::max(result.penetrationDepth, 0.0f);
		Vector3 correction = { nXZ.x * depth, 0.0f, nXZ.z * depth };

		worldTransform_.transform_.translate.x += correction.x;
		worldTransform_.transform_.translate.z += correction.z;
	}
}

Sphere Player::GetSphereData() {
	return Sphere{ collider_->GetWorldPosition(),collider_->GetRadius() };
}

void Player::RegisterBebugParam() {
	// 値の登録
	GameParamEditor::GetInstance()->AddItem("Player", "JumpMaxHeight", kJumpHeight_);
	GameParamEditor::GetInstance()->AddItem("Player", "JumpMaxTime", kJumpMaxTime_);
	GameParamEditor::GetInstance()->AddItem("Player", "MoveSpeed", kMoveSpeed_);
	GameParamEditor::GetInstance()->AddItem("Player", "TurnTime", kTurnTime_);
	GameParamEditor::GetInstance()->AddItem("Player", "DushSpeed", kDushSpeed_);
	GameParamEditor::GetInstance()->AddItem("Player", "DushMaxTime", kDushMaxTime_);

	// コンボ攻撃
	for (uint32_t i = 0; i < kConstAttacks_.size(); ++i) {
		std::string s = std::to_string(i + 1);
		GameParamEditor::GetInstance()->AddItem(kGroupNames_[0], s + "ComboMaxTime", kConstAttacks_[i].maxTime);
		GameParamEditor::GetInstance()->AddItem(kGroupNames_[0], s + "ComboRadius", kConstAttacks_[i].radius);
	}
}

void Player::ApplyDebugParam() {
	// 値の適応
	kJumpHeight_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "JumpMaxHeight");
	kJumpMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "JumpMaxTime");
	kMoveSpeed_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "MoveSpeed");
	kTurnTime_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "TurnTime");
	kDushSpeed_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "DushSpeed");
	kDushMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "DushMaxTime");

	// コンボ攻撃
	for (uint32_t i = 0; i < kConstAttacks_.size(); ++i) {
		std::string s = std::to_string(i + 1);
		kConstAttacks_[i].maxTime = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames_[0], s + "ComboMaxTime");
		kConstAttacks_[i].radius = GameParamEditor::GetInstance()->GetValue<float>(kGroupNames_[0], s + "ComboRadius");
	}
}