#include"ParticleBehavior.h"
#include"FPSCounter.h"
#include"RandomGenerator.h"
#include"GameParamEditor.h"
#include"EasingManager.h"
#include"MyMath.h"
using namespace GameEngine;

TextureManager* ParticleBehavior::textureManager_ = nullptr;

void ParticleBehavior::StatcInitialize(TextureManager* textureManager) {
    textureManager_ = textureManager;
}

void ParticleBehavior::Initialize(const std::string& name,uint32_t maxNum) {
    maxNumInstance_ = maxNum;
    name_ = name;

    // パーティクル配列を確保
    particles_.resize(maxNumInstance_);

    // WorldTransformsを初期化
    worldTransforms_ = std::make_unique<WorldTransforms>();
    Transform defaultTransform = { {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };
    worldTransforms_->Initialize(maxNumInstance_, defaultTransform);

    // 全パーティクルを非アクティブ化
    for (auto& particle : particles_) {
        particle.currentTime = 0.0f;
        particle.lifeTime = 0.0f;
    }

#ifdef _DEBUG
    // パラメータを登録する
    RegisterBebugParam();
#endif
    // 保存したデータを取得する
    ApplyDebugParam();
}

void ParticleBehavior::Update(const Matrix4x4& cameraMatrix) {

#ifdef _DEBUG
    // デバック結果を適応する
    ApplyDebugParam();
#endif


    // パーティクルの発生を管理する
    if (particleEmitter_.isLoop) {
        Create();
    }

    // 移動処理
    Move(cameraMatrix);
}

void ParticleBehavior::Emit(const Vector3& pos) {
    emitterPos_ = pos;

    if (!particleEmitter_.isLoop) {
        spawnTimer_ = particleEmitter_.spawnCoolTime;
        // 生成する
        Create();
    }
}

ParticleData ParticleBehavior::MakeNewParticle() {

    ParticleData tmpParticleData;
    // srtを設定
    float scale = RandomGenerator::Get(particleEmitter_.scaleRange.min.x, particleEmitter_.scaleRange.max.x);
    tmpParticleData.transform.scale = { scale ,scale ,scale };
    tmpParticleData.transform.rotate = {0.0f,0.0f,0.0f};
    tmpParticleData.transform.translate = {
    RandomGenerator::Get(particleEmitter_.posRange.min.x, particleEmitter_.posRange.max.x),
    RandomGenerator::Get(particleEmitter_.posRange.min.y, particleEmitter_.posRange.max.y), 
    RandomGenerator::Get(particleEmitter_.posRange.min.z, particleEmitter_.posRange.max.z),
    };
    tmpParticleData.transform.translate += emitterPos_;
    // 速度
    tmpParticleData.velocity = {
    RandomGenerator::Get(particleEmitter_.velocityRange.min.x, particleEmitter_.velocityRange.max.x),
    RandomGenerator::Get(particleEmitter_.velocityRange.min.y, particleEmitter_.velocityRange.max.y),
    RandomGenerator::Get(particleEmitter_.velocityRange.min.z, particleEmitter_.velocityRange.max.z), 
    };
    // 色
    tmpParticleData.color = { 
    RandomGenerator::Get(particleEmitter_.colorRange.min.x, particleEmitter_.colorRange.max.x),
    RandomGenerator::Get(particleEmitter_.colorRange.min.y, particleEmitter_.colorRange.max.y),
    RandomGenerator::Get(particleEmitter_.colorRange.min.z, particleEmitter_.colorRange.max.z), 
    RandomGenerator::Get(particleEmitter_.colorRange.min.w, particleEmitter_.colorRange.max.w),
    };
    // 生存時間
    tmpParticleData.currentTime = 0.0f;
    tmpParticleData.lifeTime = particleEmitter_.lifeTime;
    // 初期値を保存
    tmpParticleData.startSize = tmpParticleData.transform.scale;
    tmpParticleData.startColor = Vector3(tmpParticleData.color.x, tmpParticleData.color.y, tmpParticleData.color.z);
    tmpParticleData.startAlpha = tmpParticleData.color.w;
    // テクスチャを設定
    if (particleEmitter_.textures_.size() == 0) {
        // 何もない場合、エラーカラーを入れる
        tmpParticleData.textureHandle = 1;
    } else {
        uint32_t index = RandomGenerator::Get(static_cast<uint32_t>(0),static_cast<uint32_t>(particleEmitter_.textures_.size()-1));
        auto it = std::next(particleEmitter_.textures_.begin(), index);
        tmpParticleData.textureHandle = it->second;
    }
    return tmpParticleData;
}

void ParticleBehavior::Create() {

    // 経過時間を加算
    spawnTimer_ += FpsCounter::deltaTime;

    if (spawnTimer_ >= particleEmitter_.spawnCoolTime) {
        uint32_t spawnCount = 0;
        for (uint32_t i = 0; i < maxNumInstance_; ++i) {
            // 時間が過ぎていれば新しく生成する
            if (particles_[i].lifeTime <= particles_[i].currentTime) {
                particles_[i] = MakeNewParticle();
                spawnCount++;
            }
            // 指定した数発生させたら終了
            if (spawnCount >= particleEmitter_.spawnMaxCount || spawnCount >= maxNumInstance_) {
                break;
            }
        }
        spawnTimer_ = 0.0f;
    }
}

void ParticleBehavior::Move(const Matrix4x4& cameraMatrix) {
    currentNumInstance_ = 0;
    for (uint32_t i = 0; i < maxNumInstance_; ++i) {
        ParticleData& particle = particles_[i];

        // 生存期間を過ぎたら描画対象にしない
        if (particle.lifeTime <= particle.currentTime) {
            continue;
        }
        // 経過時間を加算
        particle.currentTime += FpsCounter::deltaTime;
        // 速度を追加
        particle.velocity += particleEmitter_.fieldAcceleration * FpsCounter::deltaTime;
        particle.transform.translate += particle.velocity * FpsCounter::deltaTime;

        // 大きさの変化
        if (particleEmitter_.sizeOverLifeTime.isEnable) {
            particle.transform.scale = Lerp(particle.startSize, particleEmitter_.sizeOverLifeTime.endSize, particle.currentTime / particle.lifeTime);
        }

        // worldTransformsの更新
        if (particleEmitter_.isBillBoard) {
            // ビルボードを適応する
            worldTransforms_->transformDatas_[currentNumInstance_].worldMatrix = MakeBillboardMatrix(particle.transform.scale, particle.transform.translate, cameraMatrix);
        } else {
            worldTransforms_->transformDatas_[currentNumInstance_].transform = particle.transform;
        }

        // 色の変化
        if (particleEmitter_.colorOverLifeTime.isEnable) {
            Vector3 tmpColor = Lerp(particle.startColor, particleEmitter_.colorOverLifeTime.endColor, particle.currentTime / particle.lifeTime);
            particle.color.x = tmpColor.x;
            particle.color.y = tmpColor.y;
            particle.color.z = tmpColor.z;
        }

        // 透明度の変化
        if (particleEmitter_.alphaOverLifeTime.isEnable) {
            particle.color.w = Lerp(particle.startAlpha, particleEmitter_.alphaOverLifeTime.endAlpha, particle.currentTime / particle.lifeTime);
        }

        worldTransforms_->transformDatas_[currentNumInstance_].color = particle.color;
        worldTransforms_->transformDatas_[currentNumInstance_].textureHandle = particle.textureHandle;
        currentNumInstance_++;
    }

    // 行列の更新処理
    if (!particleEmitter_.isBillBoard) {
        worldTransforms_->UpdateTransformMatrix(currentNumInstance_);
    }
}

void ParticleBehavior::RegisterBebugParam() {
    int index = 0;
    GameParamEditor::GetInstance()->AddItem(name_, "Textures", particleEmitter_.textures_, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "SpawnMaxCount", particleEmitter_.spawnMaxCount, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "SpawnCoolTime", particleEmitter_.spawnCoolTime, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "IsLoop", particleEmitter_.isLoop, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "IsBillBoard", particleEmitter_.isBillBoard, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "LifeTime", particleEmitter_.lifeTime, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "FieldAcceleration", particleEmitter_.fieldAcceleration, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "VelocityRange", particleEmitter_.velocityRange, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "SpawnRange", particleEmitter_.posRange, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "ScaleRange", particleEmitter_.scaleRange, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "ColorRange", particleEmitter_.colorRange, index++);
    // 拡張機能
    GameParamEditor::GetInstance()->AddItem(name_, "IsEnableSizeOverLifeTime", particleEmitter_.sizeOverLifeTime.isEnable, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "EndSize", particleEmitter_.sizeOverLifeTime.endSize, index++);

    GameParamEditor::GetInstance()->AddItem(name_, "IsEnableColorOverLifeTime", particleEmitter_.colorOverLifeTime.isEnable, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "EndColor", particleEmitter_.colorOverLifeTime.endColor, index++);

    GameParamEditor::GetInstance()->AddItem(name_, "IsEnableAlphaOverLifeTime", particleEmitter_.alphaOverLifeTime.isEnable, index++);
    GameParamEditor::GetInstance()->AddItem(name_, "EndAlpha", particleEmitter_.alphaOverLifeTime.endAlpha, index++);
}

void ParticleBehavior::ApplyDebugParam() {
    particleEmitter_.textures_ = GameParamEditor::GetInstance()->GetValue<std::map<std::string, uint32_t>>(name_, "Textures");
    particleEmitter_.spawnMaxCount = GameParamEditor::GetInstance()->GetValue<uint32_t>(name_, "SpawnMaxCount");
    particleEmitter_.spawnCoolTime = GameParamEditor::GetInstance()->GetValue<float>(name_, "SpawnCoolTime");
    particleEmitter_.isLoop = GameParamEditor::GetInstance()->GetValue<bool>(name_, "IsLoop");
    particleEmitter_.isBillBoard = GameParamEditor::GetInstance()->GetValue<bool>(name_, "IsBillBoard");
    particleEmitter_.lifeTime = GameParamEditor::GetInstance()->GetValue<float>(name_, "LifeTime");
    particleEmitter_.fieldAcceleration = GameParamEditor::GetInstance()->GetValue<Vector3>(name_, "FieldAcceleration");
    particleEmitter_.velocityRange = GameParamEditor::GetInstance()->GetValue<Range3>(name_, "VelocityRange");
    particleEmitter_.posRange = GameParamEditor::GetInstance()->GetValue<Range3>(name_, "SpawnRange");
    particleEmitter_.scaleRange = GameParamEditor::GetInstance()->GetValue<Range3>(name_, "ScaleRange");
    particleEmitter_.colorRange = GameParamEditor::GetInstance()->GetValue<Range4>(name_, "ColorRange");
    // 拡張機能
    particleEmitter_.sizeOverLifeTime.isEnable = GameParamEditor::GetInstance()->GetValue<bool>(name_, "IsEnableSizeOverLifeTime");
    particleEmitter_.sizeOverLifeTime.endSize = GameParamEditor::GetInstance()->GetValue<Vector3>(name_, "EndSize");

    particleEmitter_.colorOverLifeTime.isEnable = GameParamEditor::GetInstance()->GetValue<bool>(name_, "IsEnableColorOverLifeTime");
    particleEmitter_.colorOverLifeTime.endColor = GameParamEditor::GetInstance()->GetValue<Vector3>(name_, "EndColor");

    particleEmitter_.alphaOverLifeTime.isEnable = GameParamEditor::GetInstance()->GetValue<bool>(name_, "IsEnableAlphaOverLifeTime");
    particleEmitter_.alphaOverLifeTime.endAlpha = GameParamEditor::GetInstance()->GetValue<float>(name_, "EndAlpha");

    // 出現範囲を抑える
    if (maxNumInstance_ <= particleEmitter_.spawnMaxCount) {
        particleEmitter_.spawnMaxCount = maxNumInstance_;
    }

    if (particleEmitter_.textures_.size() >= 1) {
        // 画像名からハンドルを取得
        for (auto& it : particleEmitter_.textures_) {
            it.second = textureManager_->GetHandleByName(it.first);
        }
    }
}