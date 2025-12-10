#include "PlayerAttackEffect.h"
#include"FPSCounter.h"
#include"MyMath.h"
#include"EasingManager.h"
#include"GameParamEditor.h"
using namespace GameEngine;

void PlayerAttackEffect::Initialize(const uint32_t& texture) {
	isActive_ = false;

	// 行列初期化
	worldTransform_.Initialize({ {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} });

	// マテリアル
	material_.Initialize({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f }, 250.0f, false);
	material_.SetTextureHandle(texture);

	// 小さい粒子の初期化
	smallParticle_ = std::make_unique<ParticleBehavior>();
	smallParticle_->Initialize("EnemyDestroyParticle", 32);

#ifdef _DEBUG
	RegisterBebugParam();
#endif
	ApplyDebugParam();
}

void PlayerAttackEffect::Update(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix) {

	if (!isActive_) { return; }
	viewMatrix;
#ifdef _DEBUG
	ApplyDebugParam();
#endif

	entireTimer_ += FpsCounter::deltaTime / entireTime_;

	if (hitTimer_ <= 1.0f) {
		hitTimer_ += FpsCounter::deltaTime / hitMaxTime_;

		if (hitTimer_ <= 0.2f) {
			float localT = hitTimer_ / 0.2f;
			worldTransform_.transform_.scale = Lerp(Vector3(0.0f, 0.0f, 0.0f), Vector3(endScale_, endScale_, endScale_), EaseIn(localT));
		} else {
			float localT = (hitTimer_ - 0.2f) / 0.8f;
			worldTransform_.transform_.rotate.z = Lerp(0.0f, endRotZ_, localT);
		}

		if (hitTimer_ >= 0.8f) {
			float localT = (hitTimer_ - 0.8f) / 0.2f;
			worldTransform_.transform_.scale = Lerp(Vector3(endScale_, endScale_, endScale_),Vector3(0.0f, 0.0f, 0.0f), EaseOut(localT));
		}
	} else {
		material_.SetAplha(0.0f);
		isDrawHitEffect_ = true;
	}

	// 行列の更新処理
	worldTransform_.SetWorldMatrix(MakeBillboardMatrix(worldTransform_.transform_.scale, worldTransform_.transform_.translate, cameraMatrix, worldTransform_.transform_.rotate.z));

	// パーティクルの更新処理
	smallParticle_->Update(cameraMatrix, viewMatrix);

	if (entireTimer_ >= 1.0f) {
		isActive_ = false;
	}
}

void PlayerAttackEffect::Emitter(const Vector3& pos, const float& ratio) {
	if (!isActive_) {
		entireTimer_ = 0.0f;
		isActive_ = true;
		isDrawHitEffect_ = false;
		material_.SetAplha(1.0f);
		hitTimer_ = 0.0f;
		emitPos_ = pos;
		float half = endScale_ * 0.5f;
		endScale_ = half * ratio + half;
		worldTransform_.transform_.translate = emitPos_;
		smallParticle_->Emit(emitPos_);
	}
}

void PlayerAttackEffect::RegisterBebugParam() {
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "EntireMaxTime", entireTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "MaxTime", hitMaxTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "EndScale", endScale_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "EndRotateZ", endRotZ_);
}

void PlayerAttackEffect::ApplyDebugParam() {
	entireTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "EntireMaxTime");
	hitMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "MaxTime");
	endScale_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "EndScale");
	endRotZ_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "EndRotateZ");
}
