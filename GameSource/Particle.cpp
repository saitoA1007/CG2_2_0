#include"Particle.h"
#include<cassert>
#include<numbers>

#include"EngineSource/2D/ImGuiManager.h"
#include"EngineSource/Math/MyMath.h"

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
	for (uint32_t i = 0; i < kNumMaxInstance; ++i) {
		particleDatas_.push_back(MakeNewParticle());
	}
	planeWorldTransforms_.Initialize(kNumMaxInstance);
}

void Particle::Update(const Matrix4x4& cameraMatrix) {

	// 生存期間を過ぎていたら初期化する
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		// 生存期間していれば飛ばす
		if (particleDatas_[index].lifeTime >= particleDatas_[index].currentTime) { 
			continue;
		}
		// 新しく生成する
		particleDatas_[index] = MakeNewParticle();
	}

	// 移動処理
	numInstance_ = 0;
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		if (particleDatas_[index].lifeTime <= particleDatas_[index].currentTime) { // 生存期間を過ぎていたら更新せず描画対象にしない
			continue;
		}

		// 移動
		particleDatas_[index].transform.translate += particleDatas_[index].velocity * kDeltaTime;
		particleDatas_[index].currentTime += kDeltaTime;  // 経過時間を足す
		// トラスフォームの適応
		planeWorldTransforms_.transformDatas_[numInstance_].transform = particleDatas_[index].transform;
		// 色を適応
		float alpha = 1.0f - (particleDatas_[index].currentTime / particleDatas_[index].lifeTime);
		particleDatas_[index].color.w = alpha;
		planeWorldTransforms_.transformDatas_[numInstance_].color = particleDatas_[index].color;
		numInstance_++; // 生きているParticleの数を1つカウントする
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
	ImGui::End();
#endif
}

void Particle::Draw(const Matrix4x4 VPMatrix) {

	// 平面モデルを描画
	planeModel_->Draw(numInstance_,planeWorldTransforms_, textureHandle_, VPMatrix);
}

Particle::ParticleData Particle::MakeNewParticle() {
	ParticleData particleData;
	// SRTを設定
	particleData.transform.scale = { 1.0f,1.0f,1.0f };
	particleData.transform.rotate = { 0.0f,0.0f,0.0f };
	particleData.transform.translate = randomGenerator_.GetVector3(-1.0f, 1.0f);
	// 速度
	particleData.velocity = randomGenerator_.GetVector3(-1.0f, 1.0f);
	// 色
	particleData.color = { randomGenerator_.GetFloat(0.0f,1.0f), randomGenerator_.GetFloat(0.0f,1.0f), randomGenerator_.GetFloat(0.0f,1.0f),1.0f};
	// 時間の設定
	particleData.lifeTime = randomGenerator_.GetFloat(1.0f, 3.0f);
	particleData.currentTime = 0.0f;
	return particleData;
}