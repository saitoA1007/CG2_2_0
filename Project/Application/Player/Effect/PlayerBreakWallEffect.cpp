#include"PlayerBreakWallEffect.h"
#include"FPSCounter.h"
#include"RandomGenerator.h"
#include"GameParamEditor.h"
#include"EasingManager.h"
using namespace GameEngine;

void PlayerBreakWallEffect::Initialize(const uint32_t& texture, const Vector3& emitPos) {
	// 画像を取得
	particleGH_ = texture;

	// パーティクルが発生する位置を設定
	emitterPos_ = emitPos;

	// ワールド行列の初期化
	worldTransforms_ = std::make_unique<WorldTransforms>();
	worldTransforms_->Initialize(kNumMaxInstance, { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// ワールド行列の初期化
	particleDatas_.reserve(kNumMaxInstance);
	for (uint32_t i = 0; i < kNumMaxInstance; ++i) {
		particleDatas_.push_back(ParticleData());
		particleDatas_[i].currentTime = 2.0f;
		particleDatas_[i].lifeTime = 1.0f;
	}

#ifdef _DEBUG
	// パラメータを登録する
	RegisterBebugParam();
#endif
	// 保存したデータを取得する
	ApplyDebugParam();
}

void PlayerBreakWallEffect::Update() {
#ifdef _DEBUG
	ApplyDebugParam();
#endif

	if (isLoop_) {
		// パーティクルの生成
		Create();
	}

	// 移動処理
	Move();

	// 移動の更新処理
	worldTransforms_->UpdateTransformMatrix(numInstance_);
}

PlayerBreakWallEffect::ParticleData PlayerBreakWallEffect::MakeNewParticle() {
	ParticleData particleData;
	// SRTを設定
	particleData.transform.scale = RandomGenerator::GetVector3(scaleMin_, scaleMax_);
	particleData.startScale = particleData.transform.scale;
	particleData.transform.rotate = RandomGenerator::GetVector3(0.0f, 6.4f);
	// 位置
	particleData.transform.translate = RandomGenerator::GetVector3(spawnPosMin_, spawnPosMax_) + emitterPos_ + -playerDir_;
	particleData.transform.translate.y = emitterPos_.y + RandomGenerator::Get(1.0f, 3.0f);
	// 速度
	particleData.velocity = Normalize(particleData.transform.translate - emitterPos_) * RandomGenerator::Get(speedMin_, speedMax_);
	// 色
	particleData.color = color_;
	// 時間の設定
	particleData.lifeTime = RandomGenerator::Get(2.0f, 3.0f);
	particleData.currentTime = 0.0f;
	// テクスチャ
	particleData.textureHandle = particleGH_;

	return particleData;
}

void PlayerBreakWallEffect::Create() {
	// 経過時間を加算
	timer_ += FpsCounter::deltaTime;

	// 0.1秒経過したら生成処理
	if (timer_ >= coolTime_) {
		uint32_t spawnCount = kNumMaxInstance; // まとめて出す数
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

void PlayerBreakWallEffect::Move() {

	isFinished_ = true;

	// 移動処理
	numInstance_ = 0;
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		ParticleData& particle = particleDatas_[index];

		// 生存期間を過ぎたら描画対象にしない
		if (particle.lifeTime <= particle.currentTime) {
			continue;
		}

		isFinished_ = false;

		// 経過時間を加算
		particle.currentTime += FpsCounter::deltaTime;

		// サイズ
		particle.transform.scale = Lerp(particle.startScale, Vector3(0.0f, 0.0f, 0.0f), particle.currentTime / particle.lifeTime);

		// 移動
		particle.velocity.y += fieldAcceleration_ * FpsCounter::deltaTime;
		particle.transform.translate += particle.velocity * FpsCounter::deltaTime;

		if (particle.transform.translate.y <= 0.0f) {
			particle.velocity.y *= -elasticity_;
		}

		// トラスフォームの適応
		worldTransforms_->transformDatas_[numInstance_].transform = particle.transform;

		// 色を適応
		//particle.color.w = 1.0f - (particle.currentTime / particle.lifeTime);
		worldTransforms_->transformDatas_[numInstance_].color = particle.color;
		worldTransforms_->transformDatas_[numInstance_].textureHandle = particle.textureHandle;
		//worldTransforms_->transformDatas_[numInstance_].worldMatrix = MakeDirectionalBillboardMatrix(particle.transform.scale, particle.transform.translate, cameraMatrix, viewMatrix, particle.velocity);
		numInstance_++; // 生きているParticleの数を1つカウントする
	}
}

void PlayerBreakWallEffect::RegisterBebugParam() {
	int index = 0;
	//GameParamEditor::GetInstance()->AddItem(name_, "SpawnCoolTime", coolTime_, index++);
	//GameParamEditor::GetInstance()->AddItem(name_, "LifeTime", lifeTime_, index++);

	GameParamEditor::GetInstance()->AddItem(name_, "ScaleMin", scaleMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "ScaleMax", scaleMax_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpeedMin", speedMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpeedMax", speedMax_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnPosMin", spawnPosMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnPosMax", spawnPosMax_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "FieldAcceleration", fieldAcceleration_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "Elasticity", elasticity_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "Color", color_, index++);
	
}

void PlayerBreakWallEffect::ApplyDebugParam() {
	//coolTime_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnCoolTime");
	//lifeTime_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "LifeTime");
	// サイズ
	scaleMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "ScaleMin");
	scaleMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "ScaleMax");
	// 速度
	speedMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpeedMin");
	speedMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpeedMax");
	// 生成位置
	spawnPosMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnPosMin");
	spawnPosMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnPosMax");

	fieldAcceleration_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "FieldAcceleration");
	elasticity_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "Elasticity");

	color_ = GameParamEditor::GetInstance()->GetValue<Vector4>(name_, "Color");
}