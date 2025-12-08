#include"BreakEffect.h"
#include"MyMath.h"
#include"FPSCounter.h"
#include"EasingManager.h"
#include"RandomGenerator.h"
using namespace GameEngine;

void BreakEffect::Initialize(const Vector3& pos) {


	for (size_t i = 0; i < worldTransforms_.size(); ++i) {
		//float index = static_cast<float>(i);
		Vector3 emit = pos + RandomGenerator::GetVector3(-1.0f,1.0f);
		float scaleX = RandomGenerator::Get(3.0f, 5.0f);
		float scaleYZ = RandomGenerator::Get(7.0f, 8.0f);
		worldTransforms_[i].Initialize({ {scaleX,scaleYZ,scaleYZ},{0.0f,0.0f,0.0f},emit});
	}

	for (size_t i = 0; i < particleDatas_.size();++i) {
		particleDatas_[i].velocity = Normalize(worldTransforms_[i].transform_.translate - pos) * RandomGenerator::Get(50.0f,60.0f);
		particleDatas_[i].velocity.y = 0.0f;
		particleDatas_[i].timer = 0.0f;
		particleDatas_[i].maxTime = 1.0f;
		particleDatas_[i].scale = worldTransforms_[i].transform_.scale;
	}
}

void BreakEffect::Update(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix) {
	viewMatrix;

	isFinished_ = true;

	// 更新処理
	for (size_t i = 0; i < worldTransforms_.size(); ++i) {
		//worldTransforms_[i].transform_.rotate.z += rotateSpeed_ * FpsCounter::deltaTime;

		if (particleDatas_[i].timer >= 1.0f) { continue; }

		isFinished_ = false;

		particleDatas_[i].timer += FpsCounter::deltaTime / particleDatas_[i].maxTime;

		worldTransforms_[i].transform_.scale = Lerp(particleDatas_[i].scale, Vector3(0.0f, 0.0f, 0.0f), EaseOut(particleDatas_[i].timer));

		worldTransforms_[i].transform_.translate += particleDatas_[i].velocity * FpsCounter::deltaTime;

		// 行列を更新する
		worldTransforms_[i].SetWorldMatrix(MakeDirectionalBillboardMatrix(worldTransforms_[i].transform_.scale, worldTransforms_[i].transform_.translate, cameraMatrix, viewMatrix, particleDatas_[i].velocity));
	}
}