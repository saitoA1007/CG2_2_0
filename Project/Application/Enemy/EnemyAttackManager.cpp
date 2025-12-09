#include"EnemyAttackManager.h"
#include"RandomGenerator.h"
#include"FPSCounter.h"
#include"EasingManager.h"
#include"MyMath.h"
#include<numbers>
#include"CollisionConfig.h"
#include"GameParamEditor.h"
using namespace GameEngine;

GameEngine::PostEffectManager* EnemyAttackManager::postEffectManager_ = nullptr;

EnemyAttackManager::~EnemyAttackManager() {
	IceFallsList_.clear();
}

void EnemyAttackManager::Initialize(GameEngine::PostEffectManager* postEffectManager,const uint32_t& texture, const uint32_t& breakTexture) {

    // ポストエフェクトの管理クラスを受け取る
    postEffectManager_ = postEffectManager;
    
    // 風の当たり判定の位置を設定する
    windPositions_.reserve(8);
    windColliders_.reserve(8);

    for (int i = 0; i < 8; ++i) {
        windPositions_.push_back(windPoint({ 0.0f,0.0f,0.0f }, 0.0f));

        // 当たり判定を設定
        std::unique_ptr<GameEngine::SphereCollider> collider = std::make_unique<GameEngine::SphereCollider>();
        collider->SetRadius(2.0f);
        collider->SetWorldPosition({0.0f,0.0f,0.0f});
        collider->SetCollisionAttribute(kCollisionAttributeEnemy);
        collider->SetCollisionMask(~kCollisionAttributeEnemy);
        windColliders_.push_back(std::move(collider));
    }

    // 演出用のメモリを確保しておく
    iceFallEffectDatas_.reserve(3);
    // 演出のパーティクルを生成
    for (size_t i = 0; i < 3; ++i) {
        IceFallEffectData  iceFallEffectData;
        iceFallEffectData.timer = 0.0f;
        iceFallEffectData.particle = std::make_unique<ParticleBehavior>();
        iceFallEffectData.particle->Initialize("WaitIceFallParticle", 32);
        iceFallEffectDatas_.push_back(std::move(iceFallEffectData));
    }

    // テクスチャを取得
    iceFallTexture_ = texture;
    breakTexture_ = breakTexture;

#ifdef _DEBUG
    // 値を登録する
    RegisterBebugParam();
#endif
    // 値を適応させる
    ApplyDebugParam();
}

void EnemyAttackManager::Update(const Matrix4x4& cameraWorldMatrix, const Matrix4x4& viewMatrix) {
#ifdef _DEBUG
    // 値を適応させる
    ApplyDebugParam();
#endif
  
    // 演出の更新処理
    EffectUpdate(cameraWorldMatrix, viewMatrix);

	// 氷柱がデスフラグがたったら削除
    for (std::unique_ptr<IceFall>& iceFall : IceFallsList_) {
        if (!iceFall->IsAlive()) {
            AddBreakIceParticle(iceFall->GetWorldPosition());
        }
    }
	IceFallsList_.remove_if([](const std::unique_ptr<IceFall>& iceFall) {
		return iceFall->IsAlive() ? false : true;
	});

	// 氷柱の更新処理
	for (std::unique_ptr<IceFall>& iceFall : IceFallsList_) {
		iceFall->Update();
	}

    breakIceFallParticles_.remove_if([](const std::unique_ptr<BreakIceFallParticle>& breakIceFallParticle) {
        return breakIceFallParticle->IsFinished();
    });
    
    // 氷柱の破壊演出の更新処理
    for (auto& breakIceFallParticle : breakIceFallParticles_) {
        breakIceFallParticle->Update();
    }

    // 撃破演出
    enemyDestroyEffects_.remove_if([](const std::unique_ptr<EnemyDestroyEffect>& effect) {
        return effect->IsFinished();
    });
    for (auto& effect : enemyDestroyEffects_) {
        effect->Update(cameraWorldMatrix, viewMatrix);
    }

    // 咆哮演出の更新処理
    RoatUpdate();

    // 風攻撃の更新処理
    WindUpdate();
}

void EnemyAttackManager::AddIceFall(const Vector3& pos) {
	std::unique_ptr<IceFall> tmpIceFall = std::make_unique<IceFall>();
	tmpIceFall->Initialize(pos, iceFallTexture_);
	IceFallsList_.push_back(std::move(tmpIceFall));
}

void EnemyAttackManager::AddBreakIceParticle(const Vector3& pos) {
    std::unique_ptr<BreakIceFallParticle> tmpIceFall = std::make_unique<BreakIceFallParticle>();
    tmpIceFall->Initialize(iceFallTexture_, pos);
    breakIceFallParticles_.push_back(std::move(tmpIceFall)); 
}

void EnemyAttackManager::AddEnemyDestroyEffect(const Vector3& pos) {
    std::unique_ptr<EnemyDestroyEffect> tmp = std::make_unique<EnemyDestroyEffect>();
    tmp->Initialize(iceFallTexture_,breakTexture_,pos);
    enemyDestroyEffects_.push_back(std::move(tmp));
}

void EnemyAttackManager::CreateIceFallPositions(const float& waitIceFallTime) {
    maxIceFallEmitTime_ = waitIceFallTime;
    if (IceFallsList_.size() != 0) { return; }

    std::vector<Vector2> points;
    int attempts = 0;

    while (points.size() < targetCount && attempts < maxIter) {
        attempts++;

        // 大きな円の中にランダムな点を生成
        float r = stageRadius_ * std::sqrt(RandomGenerator::Get(0.0f, radiusRatio_));
        float theta = RandomGenerator::Get(0.0f, std::numbers::pi_v<float> *2.0f);

        Vector2 candidate;
        candidate.x = r * std::cos(theta);
        candidate.y = r * std::sin(theta);

        // 既存の点との距離をチェック
        bool isValid = true;
        float minDistSq = minDistance * minDistance; // 比較用に2乗しておく

        for (const auto& p : points) {
            if (GetDistance(candidate, p) < minDistSq) {
                isValid = false;
                break; // 近すぎる点があれば即座に却下
            }
        }

        // 条件を満たせば採用
        if (isValid) {
            points.push_back(candidate);
        }
    }

    // 求めた位置から氷を生成する
    for (size_t i = 0; i < points.size(); ++i) {
        //AddIceFall(Vector3(points[i].x, 10.0f, points[i].y));

        // 氷柱を落とす位置に演出を発生させる
        if (!iceFallEffectDatas_[i].isActive) {
            iceFallEffectDatas_[i].particle->SetEmitterPos(Vector3(points[i].x, 0.0f, points[i].y));
            iceFallEffectDatas_[i].isActive = true;
            iceFallEffectDatas_[i].timer = 0.0f;
        }
    }
}

void EnemyAttackManager::EffectUpdate(const Matrix4x4& cameraWorldMatrix, const Matrix4x4& viewMatrix) {


    for (auto& iceFallEffectData : iceFallEffectDatas_) {

        if (!iceFallEffectData.isActive) { continue; }

        iceFallEffectData.timer += FpsCounter::deltaTime / maxIceFallEmitTime_;

        if (iceFallEffectData.timer >= 1.0f) {
            iceFallEffectData.isActive = false;
            
            // 氷柱を落とす
            AddIceFall(iceFallEffectData.particle->GetEmitterPos());
            // 叫ぶ演出
            //SetIsRoat(true);
        }

        // 更新処理
        iceFallEffectData.particle->Update(cameraWorldMatrix, viewMatrix);
    }
}

void EnemyAttackManager::SetIsRoat(const bool& isRoat) {
    if (!isRoat_) {
        isRoat_ = isRoat;
        postEffectManager_->SetDrawMode(PostEffectManager::DrawMode::RadialBlur);
        roatTimer_ = 0.0f;
    }
}

void EnemyAttackManager::RoatUpdate() {
    if (!isRoat_) { return; }

    roatTimer_ += FpsCounter::deltaTime;

    if (roatTimer_ <= 0.5f) {
        float localT = roatTimer_ / 0.5f;
        postEffectManager_->radialBlurResource_.GetData()->blurWidth = Lerp(0.0f, -0.06f, EaseOutBounce(localT));
    } else {
        float localT = (roatTimer_ - 0.5f) / 0.5f;
        postEffectManager_->radialBlurResource_.GetData()->blurWidth = Lerp(-0.06f, 0.0f, EaseOutBounce(localT));
    }

    if (roatTimer_ >= 1.0f) {
        isRoat_ = false;
        postEffectManager_->SetDrawMode(PostEffectManager::DrawMode::Default);
    }
}

void EnemyAttackManager::StartWindAttack(const Vector3& pos, const float& maxTime) {
    if (isWind_) { return; }
    maxWindTime_ = maxTime;
    centerPos_ = pos;
    // 円の中心へのベクトルを求める
    Vector3 dir = Normalize(Vector3(-pos.x, 0.0f, -pos.z));
    float division = (stageRadius_ * 2.0f) / static_cast<float>(windPositions_.size());
    for (size_t i = 0; i < windPositions_.size(); ++i) {
        windPositions_[i].pos = pos;
        windPositions_[i].radius = 0.0f;
        windPositions_[i].startRadius = 0.0f;
        windPositions_[i].endRadius = static_cast<float>(i + 1) * division;
    }

    // 始点と終点の角度を求める
    float angle = std::numbers::pi_v<float> / 4.0f;
    float cos = std::cosf(angle);
    float sin = std::sinf(angle);
    Vector3 startDir = { dir.x * cos - dir.z * sin,0.0f,dir.x * sin + dir.z * cos };
    Vector3 endDir = { dir.x * cos - dir.z * -sin,0.0f,dir.x * -sin + dir.z * cos };
    startAngle_ = std::atan2f(startDir.x, startDir.z);
    endAngle_ = std::atan2f(endDir.x, endDir.z);

    // 最初の速度を設定する
    windVelocity_ = startDir * windSpeed_;

    // 当たり判定位置をリセット
    for (auto& collider : windColliders_) {
        collider->SetWorldPosition(pos);
    }

    // タイマーをリセット
    windTimer_ = 0.0f;
    // フラグをリセット
    isWind_ = true;
}

void EnemyAttackManager::WindUpdate() {
    if (!isWind_) { return; }

    windTimer_ += FpsCounter::deltaTime / maxWindTime_;

    // 角度を求める
    float angle = LerpShortAngle(endAngle_,startAngle_, windTimer_);
    float cos = std::cosf(angle);
    float sin = std::sinf(angle);

    // 風の攻撃
    size_t i = 0;
    for (auto& point : windPositions_) {

        if (windTimer_ <= 0.2f) {
            float localT = windTimer_ / 0.2f;
            point.radius = Lerp(point.startRadius, point.endRadius, localT);
        }

        point.pos = { sin * (point.radius), centerPos_.y * (static_cast<float>(7-i) / 7),cos * (point.radius)};
        point.pos.x += centerPos_.x;
        point.pos.z += centerPos_.z;

        // 当たり判定の更新
        point.pos.y = Lerp(centerPos_.y * 0.8f, 0.0f, (static_cast<float>(i) / 7));
        windColliders_[i]->SetWorldPosition(point.pos);
        i++;
    }

    // 速度を求める
    Vector3 tmp = Vector3(windPositions_[static_cast<size_t>(windPositions_.size() - 1)].pos.x, windPositions_[static_cast<size_t>(windPositions_.size() - 1)].pos.y + 1.0f,
        windPositions_[static_cast<size_t>(windPositions_.size() - 1)].pos.z);
    windVelocity_ = Normalize(tmp - windPositions_[0].pos) * windSpeed_;

    if (windTimer_ >= 1.0f) {
        isWind_ = false;
    }
}

void EnemyAttackManager::RegisterBebugParam() {

    // 氷柱攻撃
    GameParamEditor::GetInstance()->AddItem("Boss-IceFall", "minDistance", minDistance);
    GameParamEditor::GetInstance()->AddItem("Boss-IceFall", "RadiusRatio", radiusRatio_);
    //GameParamEditor::GetInstance()->AddItem("Boss-IceFall", "IceFallCount", targetCount);
}

void EnemyAttackManager::ApplyDebugParam() {
    // 氷柱攻撃
    minDistance = GameParamEditor::GetInstance()->GetValue<float>("Boss-IceFall", "minDistance");
    radiusRatio_ = GameParamEditor::GetInstance()->GetValue<float>("Boss-IceFall", "RadiusRatio");
    //targetCount = GameParamEditor::GetInstance()->GetValue<float>("Boss-IceFall", "IceFallCount");
}

namespace {

	float GetDistance(const Vector2& c1, const Vector2& c2) {
		return std::powf(c1.x - c2.x, 2) + std::powf(c1.y - c2.y, 2);
	}

    float EaseOutBounce(float t) {
        const float n1 = 7.5625f;
        const float d1 = 2.75f;

        if (t < 1.0f / d1) {
            return n1 * t * t;
        } else if (t < 2.0f / d1) {
            return n1 * (t -= 1.5f / d1) * t + 0.75f;
        } else if (t < 2.5f / d1) {
            return n1 * (t -= 2.25f / d1) * t + 0.9375f;
        } else {
            return n1 * (t -= 2.625f / d1) * t + 0.984375f;
        }
    }

    float LerpShortAngle(float a, float b, float t) {
        float diff = b - a;

        // -2pi-2piに補正する
        diff = std::fmodf(diff, std::numbers::pi_v<float> *2.0f);
        // -pi-piに補正する
        if (diff < -std::numbers::pi_v<float>) {
            diff += std::numbers::pi_v<float> *2.0f;
        } else if (diff > std::numbers::pi_v<float>) {
            diff -= std::numbers::pi_v<float> *2.0f;
        }

        return a + diff * t;
    }
}