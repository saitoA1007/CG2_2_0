#include"EnemyRushParticle.h"
#include"FPSCounter.h"
#include"RandomGenerator.h"
#include"GameParamEditor.h"
using namespace GameEngine;

void EnemyRushParticle::Initialize(const uint32_t& texture) {
	// 画像を取得
	particleGH_ = texture;

	// パーティクルが発生する位置を設定
	emitterPos_ = { 0.0f,-10.0f,0.0f };

	// ワールド行列の初期化
	worldTransforms_ = std::make_unique<WorldTransforms>();
	worldTransforms_->Initialize(kNumMaxInstance, { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// ワールド行列の初期化
	particleDatas_.reserve(kNumMaxInstance);
	for (uint32_t i = 0; i < kNumMaxInstance; ++i) {
		particleDatas_.push_back(MakeNewParticle());
	}

#ifdef _DEBUG
	// パラメータを登録する
	RegisterBebugParam();
#endif
	// 保存したデータを取得する
	ApplyDebugParam();
}

void EnemyRushParticle::Update(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix) {
#ifdef _DEBUG
	ApplyDebugParam();
#endif

	if (isLoop_) {
		// パーティクルの生成
		Create();
	}

	// 移動処理
	Move(cameraMatrix, viewMatrix);
}

EnemyRushParticle::ParticleData EnemyRushParticle::MakeNewParticle() {
	ParticleData particleData;
	// SRTを設定
	float scale = RandomGenerator::Get(scaleYMin_, scaleYMax_);
	particleData.transform.scale = { 0.5f,scale,scale };
	particleData.transform.rotate = { 0.0f,0.0f,0.0f };
	// 位置
	Vector3 vDir = Normalize(baseVelocity_);
	particleData.transform.translate = RandomGenerator::GetVector3(spawnPosMin_, spawnPosMax_) + emitterPos_ - (vDir * 2.0f);
	// 速度
	particleData.velocity = -vDir * RandomGenerator::Get(speedMin_, speedMax_);
	// 色
	particleData.color = { 1.0f,1.0f,1.0f,1.0f };
	// 時間の設定
	particleData.lifeTime = lifeTime_;
	particleData.currentTime = 0.0f;
	// テクスチャ
	particleData.textureHandle = particleGH_;

	return particleData;
}

void EnemyRushParticle::Create() {
	// 経過時間を加算
	timer_ += FpsCounter::deltaTime;

	// 0.1秒経過したら生成処理
	if (timer_ >= coolTime_) {
		uint32_t spawnCount = spawnCount_; // まとめて出す数
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

void EnemyRushParticle::Move(const Matrix4x4& cameraMatrix, const Matrix4x4& viewMatrix) {
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
		worldTransforms_->transformDatas_[numInstance_].textureHandle = particle.textureHandle;
		worldTransforms_->transformDatas_[numInstance_].worldMatrix = MakeDirectionalBillboardMatrix(particle.transform.scale, particle.transform.translate, cameraMatrix, viewMatrix, particle.velocity);
		numInstance_++; // 生きているParticleの数を1つカウントする
	}
}

void EnemyRushParticle::RegisterBebugParam() {
	int index = 0;
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnCount", spawnCount_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnCoolTime", coolTime_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "LifeTime", lifeTime_, index++);

	GameParamEditor::GetInstance()->AddItem(name_, "ScaleYMin", scaleYMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "ScaleYMax", scaleYMax_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpeedMin", speedMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpeedMax", speedMax_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnPosMin", spawnPosMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnPosMax", spawnPosMax_, index++);	
}

void EnemyRushParticle::ApplyDebugParam() {
	coolTime_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnCoolTime");
	lifeTime_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "LifeTime");
	// サイズ
	scaleYMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "ScaleYMin");
	scaleYMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "ScaleYMax");
	// 速度
	speedMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpeedMin");
	speedMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpeedMax");
	// 生成位置
	spawnPosMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnPosMin");
	spawnPosMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnPosMax");
	// 発生数
	spawnCount_ = GameParamEditor::GetInstance()->GetValue<uint32_t>(name_, "SpawnCount");
}