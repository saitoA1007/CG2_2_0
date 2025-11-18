#include"Player.h"
#include<algorithm>
#include"GameParamEditor.h"
#include"EasingManager.h"
#include"MyMath.h"
#include"FPSCounter.h"
#include"CollisionConfig.h"
#include"LogManager.h"
#include"Application/CollisionTypeID.h"
using namespace GameEngine;

void Player::Initialize(GameEngine::InputCommand* inputCommand) {

	// 入力処理を受け取る
	inputCommand_ = inputCommand;

	// ワールド行列を初期化
	worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{-2.0f,1.0f,0.0f} });

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
	//worldTransform_.transform_.translate.x = std::clamp(worldTransform_.transform_.translate.x,-9.0f,9.0f);
	//worldTransform_.transform_.translate.z = std::clamp(worldTransform_.transform_.translate.z, -9.0f, 9.0f);

	// 行列の更新
	worldTransform_.transform_.translate += velocity_;
	worldTransform_.UpdateTransformMatrix();

	// 当たり判定の位置を更新
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
}

void Player::ProcessMoveInput() {

	// プレイヤーの移動操作
	if (inputCommand_->IsCommandAcitve("MoveUp")) {
		move.z = 1.0f;
		isMove = true;
	}

	if (inputCommand_->IsCommandAcitve("MoveDown")) {
		move.z = -1.0f;
		isMove = true;
	}

	if (inputCommand_->IsCommandAcitve("MoveLeft")) {
		move.x = -1.0f;
		isMove = true;
	}

	if (inputCommand_->IsCommandAcitve("MoveRight")) {
		move.x = 1.0f;
		isMove = true;
	}

	// ジャンプ操作
	if (inputCommand_->IsCommandAcitve("Jump")) {
		if (behavior_ != Behavior::Jump) {
			behaviorRequest_ = Behavior::Jump;
		}
	}

	// ダッシュ操作
	if (inputCommand_->IsCommandAcitve("Dush")) {
		if (behavior_ != Behavior::Dush && behavior_ != Behavior::Jump) {
			behaviorRequest_ = Behavior::Dush;

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
	if (inputCommand_->IsCommandAcitve("Attack")) {
		if (behavior_ == Behavior::Normal) {
			behaviorRequest_ = Behavior::Attack;
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
		velocity_ = move * kMoveSpeed_ * FpsCounter::deltaTime;

		// 角度を設定する
		float tmpRotateY = std::atan2f(move.x, move.z);

		// 角度が変化していれば更新
		if (tmpRotateY != targetRotateY_) {
			targetRotateY_ = tmpRotateY;
			turnTimer_ = 0.0f;
		}
	}

	// 旋回処理
	if (turnTimer_ < 1.0f) {
		turnTimer_ += FpsCounter::deltaTime / kTurnTime_;
		// Y軸周りの角度
		worldTransform_.transform_.rotate.y = LerpShortAngle(worldTransform_.transform_.rotate.y, targetRotateY_, turnTimer_);
	}
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

}

void Player::DushUpdate() {

	// ダッシュ移動処理
	dushTimer_ += FpsCounter::deltaTime / kDushMaxTime_;

	// 移動
	velocity_ = dushDirection_ * kDushSpeed_ * FpsCounter::deltaTime;

	// 時間がたったら通常状態へ遷移
	if (dushTimer_ >= kDushMaxTime_) {
		behaviorRequest_ = Behavior::Normal;
	}
}

void Player::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {
	// ヒットログを出す
	Log("IsPlayerHit", "Player");
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
}

void Player::ApplyDebugParam() {
	// 値の適応
	kJumpHeight_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "JumpMaxHeight");
	kJumpMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "JumpMaxTime");
	kMoveSpeed_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "MoveSpeed");
	kTurnTime_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "TurnTime");
	kDushSpeed_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "DushSpeed");
	kDushMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>("Player", "DushMaxTime");
}