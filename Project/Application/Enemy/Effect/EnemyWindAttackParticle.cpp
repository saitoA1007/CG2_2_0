#include"EnemyWindAttackParticle.h"
#include"FPSCounter.h"
#include"RandomGenerator.h"
using namespace GameEngine;

void EnemyWindAttackParticle::Initialize(const uint32_t& texture) {
	// 画像を取得
	particleGH_ = texture;

	// パーティクルが発生する位置を設定
	emitterPos_ = {0.0f,0.0f,0.0f};

	// ワールド行列の初期化
	worldTransforms_ = std::make_unique<WorldTransforms>();
	worldTransforms_->Initialize(kNumMaxInstance, { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// ワールド行列の初期化
	particleDatas_.reserve(kNumMaxInstance);
	for (uint32_t i = 0; i < kNumMaxInstance; ++i) {
		particleDatas_.push_back(MakeNewParticle());
	}
}

void EnemyWindAttackParticle::Update() {

	if (isLoop_) {
		// パーティクルの生成
		Create();
	}

	// 移動処理
	Move();

	// 移動の更新処理
	worldTransforms_->UpdateTransformMatrix(numInstance_);
}

EnemyWindAttackParticle::ParticleData EnemyWindAttackParticle::MakeNewParticle() {
	ParticleData particleData;
	// SRTを設定
	particleData.transform.scale = { 1.0f,1.0f,1.0f };
	particleData.transform.rotate = { 0.0f,0.0f,0.0f };
	particleData.transform.translate = RandomGenerator::GetVector3(-0.2f, 0.2f) + emitterPos_;
	// 速度
	particleData.velocity = RandomGenerator::GetVector3(-0.8f, 0.8f);
	particleData.velocity.z = 0.0f;
	// 色
	particleData.color = {1.0f,1.0f,1.0f,1.0f};
	// 時間の設定
	particleData.lifeTime = RandomGenerator::Get<float>(2.0f, 3.0f);
	particleData.currentTime = 0.0f;
	// テクスチャ
	particleData.textureHandle = particleGH_;
	return particleData;
}

void EnemyWindAttackParticle::Create() {
	// 経過時間を加算
	timer_ += FpsCounter::deltaTime;

	// 0.1秒経過したら生成処理
	if (timer_ >= coolTime_) {
		int spawnCount = 2; // まとめて出す数
		for (uint32_t i = 0; i < kNumMaxInstance && spawnCount > 0; ++i) {
			if (particleDatas_[i].lifeTime < particleDatas_[i].currentTime) {
				particleDatas_[i] = MakeNewParticle();
				spawnCount--;
			}
		}

		// タイマーをリセット
		timer_ = 0.0f;
	}
}

void EnemyWindAttackParticle::Move() {
	// 移動処理
	numInstance_ = 0;
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		ParticleData& particle = particleDatas_[index];

		// 生存期間を過ぎたら描画対象にしない
		if (particle.lifeTime <= particle.currentTime) {
			continue;
		}
		// 経過時間を加算
		particle.currentTime += FpsCounter::deltaTime;

		// 移動
		particle.transform.translate += particle.velocity * FpsCounter::deltaTime;

		// トラスフォームの適応
		worldTransforms_->transformDatas_[numInstance_].transform = particle.transform;

		// 色を適応
		particle.color.w = 1.0f - (particle.currentTime / particle.lifeTime);
		worldTransforms_->transformDatas_[numInstance_].color = particle.color;

		numInstance_++; // 生きているParticleの数を1つカウントする
	}
}