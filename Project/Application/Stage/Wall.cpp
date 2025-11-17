#include"Wall.h"
#include"CollisionConfig.h"
using namespace GameEngine;

void Wall::Initialilze(const Transform& transform, float respawnTime, uint32_t maxHp) {

	// 復活までの時間を取得
	respawnTime_ = respawnTime;

	// 最大hpを取得
	maxHp_ = maxHp;
	currentHp_ = maxHp_;

	// ワールド行列を初期化
	worldTransform_.Initialize(transform);

	// 壁のデータを入れる
	/*UserData userData;
	userData.object = this;
	collider_->SetUserData(userData);*/

	// 当たり判定を作成
	collider_ = std::make_unique<OBBCollider>();
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
	collider_->SetSize(worldTransform_.transform_.scale);
	collider_->UpdateOrientationsFromRotate(worldTransform_.transform_.rotate);
	collider_->SetCollisionAttribute(kCollisionAttributeTerrain);
	collider_->SetCollisionMask(~kCollisionAttributeTerrain);

	// コールバック関数に登録する
	collider_->SetOnCollisionCallback([this](const CollisionResult& result) {
		this->OnCollision(result);
	});
}

void Wall::Update() {

	// 生存フラグがtrueなら早期リターン
	if (isAlive_) {return;}
}

void Wall::OnCollision([[maybe_unused]] const GameEngine::CollisionResult& result) {

}

OBB Wall::GetOBBData() {
	OBB obb;
	obb.center = collider_->GetWorldPosition();
	const Vector3* orientations = collider_->GetOrientations();
	for (int i = 0; i < 3; ++i) {
		obb.orientations[i] = orientations[i];
	}
	obb.size = collider_->GetSize();
	return obb;
}