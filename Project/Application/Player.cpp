#include"Player.h"
#include<algorithm>

#include"EasingManager.h"
#include"MyMath.h"
#include"FPSCounter.h"

using namespace GameEngine;

void Player::Initialize(GameEngine::Model* model) {

	// モデルを取得
	playerModel_ = model;
	// ワールド行列を初期化
	worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f,0.0f} });
}

void Player::Update(GameEngine::InputCommand* inputCommand) {

	// プレイヤーの入力処理
	ProcessMoveInput(inputCommand);

	// プレイヤーのジャンプ処理
	JumpUpdate();

	// プレイヤーを移動範囲に制限
	worldTransform_.transform_.translate.x = std::clamp(worldTransform_.transform_.translate.x,-9.0f,9.0f);
	worldTransform_.transform_.translate.z = std::clamp(worldTransform_.transform_.translate.z, -9.0f, 9.0f);

	// 行列の更新
	worldTransform_.UpdateTransformMatrix();
}

void Player::Draw(const Matrix4x4& VPMatrix) {
	// プレイヤーの描画
	playerModel_->Draw(worldTransform_, VPMatrix);
}

void Player::ProcessMoveInput(GameEngine::InputCommand* inputCommand) {

	// プレイヤーの移動操作
	if (inputCommand->IsCommandAcitve("MoveUp")) {
		worldTransform_.transform_.translate.z += kMoveSpeed_;
	}

	if (inputCommand->IsCommandAcitve("MoveDown")) {
		worldTransform_.transform_.translate.z -= kMoveSpeed_;
	}

	if (inputCommand->IsCommandAcitve("MoveLeft")) {
		worldTransform_.transform_.translate.x -= kMoveSpeed_;
	}

	if (inputCommand->IsCommandAcitve("MoveRight")) {
		worldTransform_.transform_.translate.x += kMoveSpeed_;
	}

	// ジャンプ操作
	if (inputCommand->IsCommandAcitve("Jump")) {
		if (isJump_) { return; }
		isJump_ = true;
		jumpTimer_ = 0.0f;
	}
}

void Player::JumpUpdate() {
	// フラグが立っていなければ早期リターン
	if (!isJump_) { return; }

	jumpTimer_ += 1.0f / (FpsCounter::maxFrameCount * kJumpTime_);

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

	// 時間がたったらフラグをfalse
	if (jumpTimer_ >= 1.0f) {
		isJump_ = false;
	}
}