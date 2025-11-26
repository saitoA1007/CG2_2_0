#include"EnemyAttackManager.h"
#include"RandomGenerator.h"
#include<numbers>

using namespace GameEngine;

EnemyAttackManager::~EnemyAttackManager() {
	IceFallsList_.clear();
}

void EnemyAttackManager::Update() {

	// 氷柱がデスフラグがたったら削除
	IceFallsList_.remove_if([](const std::unique_ptr<IceFall>& iceFall) {
		return iceFall->IsAlive() ? false : true;
	});

	// 氷柱の更新処理
	for (std::unique_ptr<IceFall>& iceFall : IceFallsList_) {
		iceFall->Update();
	}
}

void EnemyAttackManager::AddIceFall(const Vector3& pos) {
	std::unique_ptr<IceFall> tmpIceFall = std::make_unique<IceFall>();
	tmpIceFall->Initialize(pos);
	IceFallsList_.push_back(std::move(tmpIceFall));
}

void EnemyAttackManager::CreateIceFallPositions() {

	//stageRadius_;

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
        AddIceFall(Vector3(points[i].x, 10.0f, points[i].y));
    }
}

namespace {

	float GetDistance(const Vector2& c1, const Vector2& c2) {
		return std::powf(c1.x - c2.x, 2) + std::powf(c1.y - c2.y, 2);
	}
}