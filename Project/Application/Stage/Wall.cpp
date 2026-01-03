#include"Wall.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
using namespace GameEngine;

void Wall::Initialize(const Transform& transform) {

	// ワールド行列を初期化
	worldTransform_.Initialize(transform);

	// 当たり判定を作成
	collider_ = std::make_unique<AABBCollider>();
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
	Vector3 scale = worldTransform_.transform_.scale;
	scale.y = 30.0f;
	collider_->SetSize(scale);
	collider_->SetCollisionAttribute(kCollisionAttributeTerrain);
	collider_->SetCollisionMask(~kCollisionAttributeTerrain);

	UserData userData;
	userData.typeID = static_cast<uint32_t>(CollisionTypeID::Wall);
	collider_->SetUserData(userData);

	// コールバック関数に登録する
	//collider_->SetOnCollisionCallback([this](const CollisionResult& result) {
	//	this->OnCollisionEnter(result);
	//});
}

void Wall::Update() {

}

void Wall::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {

}

AABB Wall::GetAABBData() {
	Vector3 halfSize = collider_->GetSize() * 0.5f;
	return AABB{ worldTransform_.transform_.translate - halfSize,worldTransform_.transform_.translate + halfSize };
}