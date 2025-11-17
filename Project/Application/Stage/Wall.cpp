#include"Wall.h"
#include"CollisionConfig.h"
#include"FPSCounter.h"
#include"Application/CollisionTypeID.h"
using namespace GameEngine;

void Wall::Initialilze(const Transform& transform, float respawnTime, uint32_t maxHp) {

	// 復活までの時間を取得
	respawnTime_ = respawnTime;

	// 最大hpを取得
	maxHp_ = maxHp;
	currentHp_ = maxHp_;

	// ワールド行列を初期化
	worldTransform_.Initialize(transform);

	// マテリアルを初期化
	material_.Initialize({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f }, 250.0f, false);

	// 壁のデータを入れる
	/*UserData userData;
	userData.object = this;
	collider_->SetUserData(userData);*/

	// 当たり判定を作成
	collider_ = std::make_unique<OBBCollider>();
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
	collider_->SetSize(worldTransform_.transform_.scale * 0.5f);
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

	respawnTimer_ += FpsCounter::deltaTime;

	// リスポーン時間を超えたら、復活する
	if (respawnTimer_ >= respawnTime_) {
		isAlive_ = true;
		respawnTimer_ = 0.0f;
		currentHp_ = maxHp_;
		// 壁の状態に応じてステータスを変更する
		ChangeWallState();
	}
}

void Wall::OnCollision([[maybe_unused]] const GameEngine::CollisionResult& result) {

	// 生存フラグがfalseなら早期リターン
	if (!isAlive_) { return; }

	// hpを削る
	if (currentHp_ > 0) {
		currentHp_ -= 1;
	} else{
		// hpが0であれば生存フラグをオフ
		isAlive_ = false;

		// 誰が破壊するかによって状態を変える
		if (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Player)) {
			wallState_ = WallState::Strengthen;
		} else if (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Boss)) {
			wallState_ = WallState::Normal;
		}
	}
}

void Wall::ChangeWallState() {
	switch (wallState_)
	{
	case WallState::Normal:
		material_.SetColor({ 1.0f,1.0f,1.0f,1.0f });
		break;

	case WallState::Strengthen:
		material_.SetColor({ 1.0f,1.0f,0.0f,1.0f });
		break;

	case WallState::None:
	default:
		isAlive_ = false;
		break;
	}
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