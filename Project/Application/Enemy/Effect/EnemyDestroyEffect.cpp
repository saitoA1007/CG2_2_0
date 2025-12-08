#include"EnemyDestroyEffect.h"
#include"RandomGenerator.h"
#include"FPSCounter.h"
#include"GameParamEditor.h"
#include"EasingManager.h"
using namespace GameEngine;

void EnemyDestroyEffect::Initialize(const uint32_t& texture, const uint32_t& breakTextuer, const Vector3& emitPos) {

	emitPos_ = emitPos;

#ifdef _DEBUG
	// 値を登録
	RegisterBebugParam();
#endif
	ApplyDebugEffectParam();

	for (auto& particle : particleData_) {
		particle.timer = 0.0f;
		particle.maxTime = RandomGenerator::Get(1.0f, 1.5f);
		Vector3 rot = RandomGenerator::GetVector3(0.0f, 3.2f);
		Vector3 pos = emitPos_ + RandomGenerator::GetVector3(spawnPosMin_, spawnPosMax_);
		particle.worldTransform.Initialize({ { 1.0f, 1.0f, 1.0f},rot,pos });
		particle.material = std::make_unique<IceRockMaterial>();
		particle.material->Initialize();
		particle.material->materialData_->textureHandle = texture;
		// 値を適応させる
		ApplyDebugParam(*particle.material->materialData_);

		particle.scale = RandomGenerator::GetVector3(4.0f, 8.0f);

		Vector3 v = Normalize(particle.worldTransform.transform_.translate - emitPos_);
		particle.velocity = v * RandomGenerator::Get(10.0f, 15.0f);
	}

	// 小さい粒子の初期化
	smallParticle_ = std::make_unique<ParticleBehavior>();
	smallParticle_->Initialize("EnemyDestroyParticle", 32);
	smallParticle_->Emit(emitPos_);


	breakWorldTransform_.Initialize({ {15.0f,15.0f,15.0f},{0.0f,0.0f,0.0f},emitPos_ });
	// マテリアルを初期化
	material_.Initialize({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f }, 250.0f, false);
	material_.SetTextureHandle(breakTextuer);
}

void EnemyDestroyEffect::Update(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix) {

	isFinished_ = true;

	// 更新処理
	for (auto& particle : particleData_) {

		if (particle.timer >= 1.0f) { continue; }

		isFinished_ = false;

		particle.timer += FpsCounter::deltaTime / particle.maxTime;

		// 拡縮
		particle.worldTransform.transform_.scale = Lerp(particle.scale, Vector3(0.0f, 0.0f, 0.0f), EaseIn(particle.timer));

		// 移動
		//particle.velocity.y -= 50.0f * FpsCounter::deltaTime;
		particle.worldTransform.transform_.translate += particle.velocity * FpsCounter::deltaTime;

		// 更新
		particle.worldTransform.UpdateTransformMatrix();
	}

	timer_ += FpsCounter::deltaTime / maxTime_;
	if (timer_ >= 1.0f) {
		material_.SetAplha(0.0f);
	}

	// カメラ位置に合わせる
	breakWorldTransform_.SetWorldMatrix(MakeBillboardMatrix(breakWorldTransform_.transform_.scale, breakWorldTransform_.transform_.translate, cameraMatrix, breakWorldTransform_.transform_.rotate.z));

	smallParticle_->Update(cameraMatrix, viewMatrix);
}

void EnemyDestroyEffect::RegisterBebugParam() {
	int index = 0;
	GameParamEditor::GetInstance()->AddItem(name_, "ScaleMin", scaleMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "ScaleMax", scaleMax_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpeedMin", speedMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpeedMax", speedMax_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnPosMin", spawnPosMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnPosMax", spawnPosMax_, index++);
}

void EnemyDestroyEffect::ApplyDebugEffectParam() {
	// サイズ
	scaleMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "ScaleMin");
	scaleMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "ScaleMax");
	// 速度
	speedMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpeedMin");
	speedMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpeedMax");
	// 生成位置
	spawnPosMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnPosMin");
	spawnPosMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnPosMax");
}

void EnemyDestroyEffect::ApplyDebugParam(IceRockMaterial::MaterialData& iceMaterial) {
	// マテリアル
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