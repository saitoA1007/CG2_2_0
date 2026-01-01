#include"RockBullet.h"
#include"GameParamEditor.h"
#include"FPSCounter.h"
using namespace GameEngine;

void RockBullet::Initialize(const Vector3& pos, const Vector3& dir) {
	// 速度を設定
	velocity_ = dir * speed_;

	// ワールド行列を初期化
	worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},pos });

#ifdef USE_IMGUI
	RegisterBebugParam();
#endif 
	ApplyDebugParam();
}

void RockBullet::Update() {

	timer_ += FpsCounter::deltaTime / lifeTime_;
	// 生存フラグを無効
	if (timer_ >= 1.0f) {
		isAlive_ = false;
	}

	// 移動
	worldTransform_.transform_.translate += velocity_ * FpsCounter::deltaTime;

	// 行列を更新
	worldTransform_.UpdateTransformMatrix();

	// 地面に着いたら生存フラグを無効
	if (worldTransform_.transform_.translate.y <= 0.0f) {
		isAlive_ = false;
	}
}

void RockBullet::RegisterBebugParam() {
	// 値の登録
	GameParamEditor::GetInstance()->AddItem(groupName_, "Speed", speed_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "LifeTime", lifeTime_);
}

void RockBullet::ApplyDebugParam() {
	speed_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "Speed");
	lifeTime_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "LifeTime");
}

