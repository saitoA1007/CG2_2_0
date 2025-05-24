#include"Particle.h"
#include<cassert>
#include<numbers>

#include"EngineSource/2D/ImGuiManager.h"
#include"EngineSource/Math/MyMath.h"

#include"CollisionUtils.h"

using namespace GameEngine;

Particle::~Particle() {

}

void Particle::Initialize(GameEngine::Model* model, const uint32_t& textureHandle) {
	// Nullチェック
	assert(model);

	// モデルの取得
	planeModel_ = model;

	// テクスチャを取得
	textureHandle_ = textureHandle;

	// ランダムの生成器の初期化
	randomGenerator_.Initialize();

	// ワールド行列の初期化
	planeWorldTransforms_.Initialize(kNumMaxInstance);

	accelerationField_.acceleration = { 15.0f,0.0f,0.0f };
	accelerationField_.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationField_.area.max = { 1.0f,1.0f,1.0f };
}

void Particle::Update(const Matrix4x4& cameraMatrix) {

	// 発生処理
	if (particleEmitter_->IsCall()) {
		particleDatas_.splice(particleDatas_.end(), Emit(particleEmitter_->GetEmitter()));
	}

	// 移動処理
	numInstance_ = 0;
	for (std::list<ParticleData>::iterator particleIterator = particleDatas_.begin(); particleIterator != particleDatas_.end();) {
		if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
			particleIterator = particleDatas_.erase(particleIterator);  // 生存期間が過ぎたParticleはlistから消す。戻り値が次のイテレータとなる
			continue;
		}

		// Fieldの範囲内のParticleには加速度を適用する
		if (enableField_) {
			if (IsAABBPosintCollision(accelerationField_.area, particleIterator->transform.translate)) {
				particleIterator->velocity += accelerationField_.acceleration * kDeltaTime;
			}
		}

		// 移動  
		particleIterator->transform.translate += particleIterator->velocity * kDeltaTime;
		particleIterator->currentTime += kDeltaTime;  // 経過時間を足す  

		// トラスフォームの適応  
		planeWorldTransforms_.transformDatas_[numInstance_].transform = particleIterator->transform;

		// 色を適応  
		float alpha = 1.0f - (particleIterator->currentTime / particleIterator->lifeTime);
		particleIterator->color.w = alpha;
		planeWorldTransforms_.transformDatas_[numInstance_].color = particleIterator->color;

		if (numInstance_ < kNumMaxInstance) {
			++numInstance_;
		}

		++particleIterator; // 次のイテレータに進める  
	}

	// ビルボードの適応の有無で行列の更新処理を変える
	if (useBillboard_) {
		for (uint32_t i = 0; i < numInstance_; ++i) {
			// ビルボードの回転行列を作成
			Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(0.0f);
			Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, cameraMatrix);
			billboardMatrix.m[3][0] = 0.0f;
			billboardMatrix.m[3][1] = 0.0f;
			billboardMatrix.m[3][2] = 0.0f;
			// ST行列を作成
			Matrix4x4 scaleMatrix = MakeScaleMatrix(planeWorldTransforms_.transformDatas_[i].transform.scale);
			Matrix4x4 translateMatrix = MakeTranslateMatrix(planeWorldTransforms_.transformDatas_[i].transform.translate);
			// 行列の更新
			planeWorldTransforms_.transformDatas_[i].worldMatrix = scaleMatrix * billboardMatrix * translateMatrix;
		}
	} else {
		// 行列の更新処理
		planeWorldTransforms_.UpdateTransformMatrix(numInstance_);
	}

#ifdef _DEBUG

	ImGui::Begin("DebugParticle");
	ImGui::Checkbox("useBillboard", &useBillboard_);
	ImGui::Checkbox("enableField", &enableField_);

	if (ImGui::Button("Add Particle")) {
		particleDatas_.splice(particleDatas_.end(), Emit(particleEmitter_->GetEmitter()));
	}
	ImGui::End();
#endif
}

void Particle::Draw(const Matrix4x4 VPMatrix) {

	// 平面モデルを描画
	planeModel_->Draw(numInstance_,planeWorldTransforms_, textureHandle_, VPMatrix);
}

Particle::ParticleData Particle::MakeNewParticle(const Vector3& translate) {
	ParticleData particleData;
	// SRTを設定
	particleData.transform.scale = { 1.0f,1.0f,1.0f };
	particleData.transform.rotate = { 0.0f,0.0f,0.0f };
	particleData.transform.translate = translate + randomGenerator_.GetVector3(-1.0f, 1.0f);
	// 速度
	particleData.velocity = randomGenerator_.GetVector3(-1.0f, 1.0f);
	// 色
	particleData.color = { randomGenerator_.GetFloat(0.0f,1.0f), randomGenerator_.GetFloat(0.0f,1.0f), randomGenerator_.GetFloat(0.0f,1.0f),1.0f};
	// 時間の設定
	particleData.lifeTime = randomGenerator_.GetFloat(1.0f, 3.0f);
	particleData.currentTime = 0.0f;
	return particleData;
}

std::list<Particle::ParticleData> Particle::Emit(const ParticleEmitter::Emitter& emitter) {

	std::list<ParticleData> particles;

	for (uint32_t count = 0; count < emitter.count; ++count) {
		particles.push_back(MakeNewParticle(emitter.transform.translate));
	}
	return particles;
}