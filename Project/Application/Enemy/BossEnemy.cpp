#include"BossEnemy.h"
#include"CollisionConfig.h"
using namespace GameEngine;

void BossEnemy::Initialize() {

	// ワールド行列を初期化
	worldTransform_.Initialize({ {2.0f,2.0f,2.0f},{0.0f,0.0f,0.0f},{0.0f,1.0f,10.0f} });

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

	// 行列の更新
	worldTransform_.UpdateTransformMatrix();

	// 当たり判定の位置を更新
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
}

void BossEnemy::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {

}

Sphere BossEnemy::GetSphereData() {
	return Sphere{ collider_->GetWorldPosition(),collider_->GetRadius() };
}

void BossEnemy::RegisterBebugParam() {

}

void BossEnemy::ApplyDebugParam() {

}