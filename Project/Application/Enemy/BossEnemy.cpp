#include"BossEnemy.h"
#include"CollisionConfig.h"
#include"FPSCounter.h"
#include"Application/Player/Player.h"
#include"Application/CollisionTypeID.h"
#include"LogManager.h"
using namespace GameEngine;

void BossEnemy::Initialize() {

	// ワールド行列を初期化
	worldTransform_.Initialize({ {2.0f,2.0f,2.0f},{0.0f,0.0f,0.0f},{0.0f,2.0f,10.0f} });

	// 当たり判定を設定
	collider_ = std::make_unique<SphereCollider>();
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
	collider_->SetRadius(collisionRadius_);
	collider_->SetCollisionAttribute(kCollisionAttributeEnemy);
	collider_->SetCollisionMask(~kCollisionAttributeEnemy);

	// コールバック関数を登録する
	collider_->SetOnCollisionEnterCallback([this](const CollisionResult& result) {
		this->OnCollisionEnter(result);
	});

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
	ApplyDebugParam();
#else
	// 値を適応させる
	ApplyDebugParam();
#endif
}

void BossEnemy::Update() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

	// 円運動をおこなう
	theta_ += FpsCounter::deltaTime;
	worldTransform_.transform_.translate.x = std::cosf(theta_) * 15.0f;
	worldTransform_.transform_.translate.z = std::sinf(theta_) * 15.0f;

	// ヒットした時に点滅する処理
	if (isHit_) {
		hitTimer_ += FpsCounter::deltaTime / maxHitTime_;

		if (static_cast<int>(hitTimer_ * 20.0f) % 2 == 0) {
			alpha_ = 1.0f;
		} else {
			alpha_ = 0.5f;
		}

		if (hitTimer_ >= maxHitTime_) {
			isHit_ = false;
			alpha_ = 1.0f;
			hitTimer_ = 0.0f;
		}
	}

	// 行列の更新
	worldTransform_.UpdateTransformMatrix();

	// 当たり判定の位置を更新
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
}

void BossEnemy::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {

	if (isHit_) { return; }

	if (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Player)) {
		Player* player = result.userData.As<Player>();

		if (player->GetPlayerBehavior() == Player::Behavior::Jump) {
			Log("isHitBoss");
			hp_ -= 1;
			isHit_ = true;
			if (hp_ <= 0) {
				isAlive_ = false;
			}
		}
	}
}

Sphere BossEnemy::GetSphereData() {
	return Sphere{ collider_->GetWorldPosition(),collider_->GetRadius() };
}

void BossEnemy::RegisterBebugParam() {

}

void BossEnemy::ApplyDebugParam() {

}