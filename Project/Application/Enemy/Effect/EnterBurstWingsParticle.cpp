#include"EnterBurstWingsParticle.h"
#include"FPSCounter.h"
#include"RandomGenerator.h"
#include"GameParamEditor.h"
using namespace GameEngine;

void EnterBurstWingsParticle::Initialize(const uint32_t& texture) {
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

void EnterBurstWingsParticle::Update() {
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

EnterBurstWingsParticle::ParticleData EnterBurstWingsParticle::MakeNewParticle() {
	ParticleData particleData;
	// SRTを設定
	float scale = RandomGenerator::Get(scaleMin_, scaleMax_);
	particleData.transform.scale = { scale,scale,scale };
	particleData.transform.rotate = RandomGenerator::GetVector3(0.0f, 6.4f);
	particleData.transform.translate = RandomGenerator::GetVector3(spawnPosMin_, spawnPosMax_) + emitterPos_;
	particleData.transform.translate.y = emitterPos_.y + RandomGenerator::Get(-5.0f, -0.5f);
	// 速度
	particleData.velocity = Normalize(particleData.transform.translate - emitterPos_) * RandomGenerator::Get(speedMin_, speedMax_);
	particleData.velocity.y = RandomGenerator::Get(-5.0f, -3.0f);
	// 色
	particleData.color = { 1.0f,1.0f,1.0f,1.0f };
	// 時間の設定
	particleData.lifeTime = lifeTime_;
	particleData.currentTime = 0.0f;
	// 回転速度
	//particleData.rotateSpeed = RandomGenerator::Get(6.0f, 10.0f);
	// テクスチャ
	particleData.textureHandle = particleGH_;
	// 振れ幅
	particleData.swayPhase = RandomGenerator::Get(0.0f, 6.28f);
	particleData.swaySpeed = RandomGenerator::Get(2.0f, 4.0f);
	particleData.swayWidth = RandomGenerator::Get(0.1f, 0.3f);
	return particleData;
}

void EnterBurstWingsParticle::Create() {
	// 経過時間を加算
	timer_ += FpsCounter::deltaTime;

	// 0.1秒経過したら生成処理
	if (timer_ >= coolTime_) {
		int spawnCount = spawnCount_; // まとめて出す数
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

void EnterBurstWingsParticle::Move() {
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

		particle.velocity += Vector3(0.0f,0.0f,0-10.0f) * FpsCounter::deltaTime;

		Vector3 forwardDir = Normalize(particle.velocity);
		Vector3 up = { 0.0f, 1.0f, 0.0f };
		Vector3 rightDir = Cross(forwardDir, up);

		// 揺れの計算
		float timeValue = particle.currentTime * particle.swaySpeed + particle.swayPhase;
		float swayPower = std::sinf(timeValue) * 10.0f;

		Vector3 currentVelocity = particle.velocity + (rightDir * swayPower);

		// 移動
		particle.transform.translate += currentVelocity * FpsCounter::deltaTime;

		// 移動
		//particle.transform.translate += particle.velocity * FpsCounter::deltaTime;

		float swayValue = std::sinf(particle.currentTime * particle.swaySpeed + particle.swayPhase);

		Vector3 velocityDir = Normalize(particle.velocity);
		//Vector3 up = { 0.0f, 1.0f, 0.0f };
		Vector3 right = Normalize(Cross(up, velocityDir));
		Vector3 drawPos = particle.transform.translate + (right * swayValue * particle.swayWidth);
		float maxTiltAngle = 0.5f;
		particle.transform.rotate.z = swayValue * maxTiltAngle;

		// トラスフォームの適応
		worldTransforms_->transformDatas_[numInstance_].transform = particle.transform;
		worldTransforms_->transformDatas_[numInstance_].transform.translate = drawPos;

		// 色を適応
		//particle.color.w = 1.0f - (particle.currentTime / particle.lifeTime);
		worldTransforms_->transformDatas_[numInstance_].color = particle.color;
		worldTransforms_->transformDatas_[numInstance_].textureHandle = particle.textureHandle;
		//worldTransforms_->transformDatas_[numInstance_].textureHandle = 0;
		numInstance_++; // 生きているParticleの数を1つカウントする
	}
}

void EnterBurstWingsParticle::RegisterBebugParam() {
	int index = 0;
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnCount", spawnCount_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnCoolTime", coolTime_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "LifeTime", lifeTime_, index++);


	GameParamEditor::GetInstance()->AddItem(name_, "ScaleYMin", scaleMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "ScaleYMax", scaleMax_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpeedMin", speedMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpeedMax", speedMax_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnPosMin", spawnPosMin_, index++);
	GameParamEditor::GetInstance()->AddItem(name_, "SpawnPosMax", spawnPosMax_, index++);
}

void EnterBurstWingsParticle::ApplyDebugParam() {
	// 発生数
	spawnCount_ = GameParamEditor::GetInstance()->GetValue<uint32_t>(name_, "SpawnCount");
	coolTime_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnCoolTime");
	lifeTime_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "LifeTime");

	// サイズ
	scaleMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "ScaleYMin");
	scaleMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "ScaleYMax");
	// 速度
	speedMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpeedMin");
	speedMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpeedMax");
	// 生成位置
	spawnPosMin_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnPosMin");
	spawnPosMax_ = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnPosMax");
}