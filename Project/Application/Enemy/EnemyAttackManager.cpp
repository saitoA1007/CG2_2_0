#include"EnemyAttackManager.h"
#include"RandomGenerator.h"
#include"FPSCounter.h"
#include"EasingManager.h"
#include"MyMath.h"
#include<numbers>
#include"CollisionConfig.h"
using namespace GameEngine;

GameEngine::PostEffectManager* EnemyAttackManager::postEffectManager_ = nullptr;

EnemyAttackManager::~EnemyAttackManager() {
	IceFallsList_.clear();
}

void EnemyAttackManager::Initialize(GameEngine::PostEffectManager* postEffectManager) {

    // ポストエフェクトの管理クラスを受け取る
    postEffectManager_ = postEffectManager;
    
    // 風の当たり判定の位置を設定する
    windPositions_.reserve(3);
    windColliders_.reserve(3);

    for (int i = 0; i < 3; ++i) {
        windPositions_.push_back(windPoint({ 0.0f,0.0f,0.0f }, 0.0f));

        // 当たり判定を設定
        std::unique_ptr<GameEngine::SphereCollider> collider = std::make_unique<GameEngine::SphereCollider>();
        collider->SetRadius(1.0f);
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
}

void EnemyAttackManager::Update(const Matrix4x4& cameraWorldMatrix, const Matrix4x4& viewMatrix) {

    // 演出の更新処理
    EffectUpdate(cameraWorldMatrix, viewMatrix);

	// 氷柱がデスフラグがたったら削除
	IceFallsList_.remove_if([](const std::unique_ptr<IceFall>& iceFall) {
		return iceFall->IsAlive() ? false : true;
	});

	// 氷柱の更新処理
	for (std::unique_ptr<IceFall>& iceFall : IceFallsList_) {
		iceFall->Update();
	}

    // 咆哮演出の更新処理
    RoatUpdate();
}

void EnemyAttackManager::AddIceFall(const Vector3& pos) {
	std::unique_ptr<IceFall> tmpIceFall = std::make_unique<IceFall>();
	tmpIceFall->Initialize(pos);
	IceFallsList_.push_back(std::move(tmpIceFall));
}

void EnemyAttackManager::CreateIceFallPositions() {

    if (IceFallsList_.size() != 0) { return; }

	//// 離れる距離
	const float minDistance = 5.0f;
	const int targetCount = 3;
	const int maxIter = 100;

    std::vector<Vector2> points;
    int attempts = 0;

    while (points.size() < targetCount && attempts < maxIter) {
        attempts++;

        // 大きな円の中にランダムな点を生成
        float r = stageRadius_ * std::sqrt(RandomGenerator::Get(0.0f, 1.0f));
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
            iceFallEffectDatas_[i].particle->SetEmitterPos(Vector3(points[i].x, 10.0f, points[i].y));
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

void EnemyAttackManager::StartWindAttack(const Vector3& pos) {
    if (isWind_) { return; }
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
    float angle = LerpShortAngle(startAngle_, endAngle_, windTimer_);
    float cos = std::cosf(angle);
    float sin = std::sinf(angle);

    // 風の攻撃
    size_t i = 0;
    for (auto& point : windPositions_) {

        if (windTimer_ <= 0.2f) {
            float localT = windTimer_ / 0.2f;
            point.radius = Lerp(point.startRadius, point.endRadius, localT);
        }

        point.pos = { cos * (point.radius), 0.0f,sin * (point.radius) };
        point.pos += centerPos_;

        // 当たり判定の更新
        windColliders_[i]->SetWorldPosition(point.pos);
        i++;
    }

    if (windTimer_ >= 1.0f) {
        isWind_ = false;
    }
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