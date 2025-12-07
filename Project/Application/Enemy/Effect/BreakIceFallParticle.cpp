#include"BreakIceFallParticle.h"
#include"RandomGenerator.h"
#include"FPSCounter.h"
#include"GameParamEditor.h"
#include"EasingManager.h"
using namespace GameEngine;

void BreakIceFallParticle::Initialize(const uint32_t& texture, const Vector3& emitPos) {

	emitPos_ = emitPos;

	for (auto& particle : particleData_) {
		particle.timer = 0.0f;
		particle.maxTime = RandomGenerator::Get(0.5f, 1.0f);
		Vector3 rot = RandomGenerator::GetVector3(0.0f, 3.2f);
		Vector3 pos = Vector3(emitPos_.x, emitPos_.y + 3.0f, emitPos_.z);
		pos.x += RandomGenerator::Get(-1.0f, 1.0f);
		pos.z += RandomGenerator::Get(-1.0f, 1.0f);
		particle.worldTransform.Initialize({ { 1.0f, 1.0f, 1.0f},rot,pos});
		particle.material = std::make_unique<IceRockMaterial>();
		particle.material->Initialize();
		particle.material->materialData_->textureHandle = texture;
		// 値を適応させる
		ApplyDebugParam(*particle.material->materialData_);

		particle.scale = RandomGenerator::GetVector3(4.0f, 8.0f);

		Vector3 v = Normalize(particle.worldTransform.transform_.translate - emitPos_);
		particle.velocity.y = v.y * RandomGenerator::Get(10.0f,15.0f);
		particle.velocity.x = v.x * RandomGenerator::Get(30.0f, 40.0f);
		particle.velocity.z = v.z * RandomGenerator::Get(30.0f, 40.0f);
	}
}

void BreakIceFallParticle::Update() {

	//const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix
	//cameraMatrix;
	//viewMatrix;

	isFinished_ = true;

	// 更新処理
	for (auto& particle : particleData_) {

		if (particle.timer >= 1.0f) { continue; }

		isFinished_ = false;

		particle.timer += FpsCounter::deltaTime / particle.maxTime;

		// 拡縮
		particle.worldTransform.transform_.scale = Lerp(particle.scale, Vector3(0.0f, 0.0f, 0.0f), EaseIn(particle.timer));

		// 移動
		particle.velocity.y -= 50.0f * FpsCounter::deltaTime;
		particle.worldTransform.transform_.translate += particle.velocity * FpsCounter::deltaTime;

		// 更新
		particle.worldTransform.UpdateTransformMatrix();
	}
}

void BreakIceFallParticle::ApplyDebugParam(IceRockMaterial::MaterialData& iceMaterial) {
	iceMaterial.color = GameParamEditor::GetInstance()->GetValue<Vector4>(kGroupName_, "IceColor");
	specularColor = GameParamEditor::GetInstance()->GetValue<Vector4>(kGroupName_, "SpecularColor");
	rimColor = GameParamEditor::GetInstance()->GetValue<Vector4>(kGroupName_, "RimColor");
	iceMaterial.shininess = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "Shininess");
	iceMaterial.rimIntensity = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "RimIntensity");
	iceMaterial.rimPower = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "RimPower");
	iceMaterial.rimColor.x = rimColor.x;
	iceMaterial.rimColor.y = rimColor.y;
	iceMaterial.rimColor.z = rimColor.z;
	iceMaterial.specularColor.x = specularColor.x;
	iceMaterial.specularColor.y = specularColor.y;
	iceMaterial.specularColor.z = specularColor.z;
}