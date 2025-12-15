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

	// サブ
	subWorldTransform_.Initialize({ {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f} });
	// マテリアル
	subMaterial_.Initialize({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f }, 250.0f, false);
	subMaterial_.SetTextureHandle(texture);
#ifdef _DEBUG
	RegisterBebugParam();
#endif
	ApplyDebugParam();
}

void PlayerAttackEffect::Update(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix) {

	if (!isActive_) { return; }
	viewMatrix;
#ifdef _DEBUG
	//ApplyDebugParam();
#endif

	entireTimer_ += FpsCounter::deltaTime / entireTime_;

	if (hitTimer_ <= 1.0f) {
		hitTimer_ += FpsCounter::deltaTime / hitMaxTime_;

		if (hitTimer_ <= 0.2f) {
			float localT = hitTimer_ / 0.2f;
			worldTransform_.transform_.scale = Lerp(Vector3(0.0f, 0.0f, 0.0f), Vector3(endScale_, endScale_, endScale_), EaseIn(localT));
			subWorldTransform_.transform_.scale = Lerp(Vector3(diffScale_, diffScale_, diffScale_), Vector3(endScale_, endScale_, endScale_) + diffScale_, EaseIn(localT));
		} else {
			float localT = (hitTimer_ - 0.2f) / 0.8f;
			worldTransform_.transform_.rotate.z = Lerp(0.0f, endRotZ_, localT);
		}

		if (hitTimer_ >= 0.8f) {
			float localT = (hitTimer_ - 0.8f) / 0.2f;
			worldTransform_.transform_.scale = Lerp(Vector3(endScale_, endScale_, endScale_),Vector3(0.0f, 0.0f, 0.0f), EaseOut(localT));
			subWorldTransform_.transform_.scale = Lerp(Vector3(endScale_, endScale_, endScale_) + diffScale_, Vector3(0.0f, 0.0f, 0.0f), EaseOut(localT));
		}
	} else {
		material_.SetAplha(0.0f);
		subMaterial_.SetAplha(0.0f);
		isDrawHitEffect_ = true;
	}

	// 行列の更新処理
	worldTransform_.SetWorldMatrix(MakeBillboardMatrix(worldTransform_.transform_.scale, worldTransform_.transform_.translate, cameraMatrix, worldTransform_.transform_.rotate.z));
	subWorldTransform_.SetWorldMatrix(MakeBillboardMatrix(subWorldTransform_.transform_.scale, worldTransform_.transform_.translate, cameraMatrix, worldTransform_.transform_.rotate.z));

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
		subMaterial_.SetAplha(1.0f);
		hitTimer_ = 0.0f;
		emitPos_ = pos;
		float half = endScale_ * 0.5f;
		endScale_ = half * ratio + half;
		worldTransform_.transform_.translate = emitPos_;
		smallParticle_->Emit(emitPos_);

		isSubDraw_ = true;

		// 色
		if (ratio <= 0.4f) {
			isSubDraw_ = false;
			color_ = { 1.0f,1.0f,1.0f,1.0f };
		} else if(ratio <= 0.8f) {
			color_ = { 0.91f, 1.0f, 0.0f,1.0f };
		} else {
			color_ = { 1.0f, 0.11f, 1.0f,1.0f };
		}

		subMaterial_.SetColor(color_);
	}
}

void PlayerAttackEffect::RegisterBebugParam() {
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "EntireMaxTime", entireTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "MaxTime", hitMaxTime_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "EndScale", endScale_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "EndRotateZ", endRotZ_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "Color", color_);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "DiffScale", diffScale_);
}

void PlayerAttackEffect::ApplyDebugParam() {
	entireTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "EntireMaxTime");
	hitMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "MaxTime");
	endScale_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "EndScale");
	endRotZ_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "EndRotateZ");
	color_ = GameParamEditor::GetInstance()->GetValue<Vector4>(kGroupName_, "Color");
	diffScale_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "DiffScale");
	subMaterial_.SetColor(color_);
}
