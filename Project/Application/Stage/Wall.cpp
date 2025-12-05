#include"Wall.h"
#include"CollisionConfig.h"
#include"FPSCounter.h"
#include"Application/CollisionTypeID.h"
#include"Application/Player/Player.h"
#include"Application/Enemy/BossEnemy.h"
#include "LogManager.h"
using namespace GameEngine;

void Wall::Initialilze(const Transform& transform, float respawnTime, int32_t maxHp) {

	// 復活までの時間を取得
	respawnTime_ = respawnTime;

	// 最大hpを取得
	maxHp_ = maxHp;
	currentHp_ = maxHp_;

	// ワールド行列を初期化
	worldTransform_.Initialize(transform);

	// マテリアルを初期化
	material_.Initialize({ 0.8f,0.8f,0.8f,1.0f }, { 1.0f,1.0f,1.0f }, 250.0f, false);

	// 壁のデータを入れる
	/*UserData userData;
	userData.object = this;
	collider_->SetUserData(userData);*/

	// 当たり判定を作成
	collider_ = std::make_unique<OBBCollider>();
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
	worldTransform_.transform_.scale *= 0.5f;
    worldTransform_.transform_.scale.y = 1024.0f;
	collider_->SetSize(worldTransform_.transform_.scale);
	collider_->UpdateOrientationsFromRotate(worldTransform_.transform_.rotate);
	collider_->SetCollisionAttribute(kCollisionAttributeTerrain);
	collider_->SetCollisionMask(~kCollisionAttributeTerrain);
    UserData userData;
    userData.typeID = static_cast<uint32_t>(CollisionTypeID::Wall);
    userData.object = this;
    collider_->SetUserData(userData);

	// コールバック関数に登録する
	collider_->SetOnCollisionCallback([this](const CollisionResult& result) {
		this->OnCollisionEnter(result);
	});
}

void Wall::Update() {
	if (currentHp_ <= 0) {
		isAlive_ = false;
    }

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

void Wall::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {
	// 生存フラグがfalseなら早期リターン
	if (!isAlive_) { return; }

    // 当たったのがプレイヤーまたはボスでなければ早期リターン
    if (result.userData.typeID != static_cast<uint32_t>(CollisionTypeID::Player) &&
		result.userData.typeID != static_cast<uint32_t>(CollisionTypeID::Boss)) {
		return;
    }

    // プレイヤーまたはボスのポインタを取得
    Player *player = nullptr;
	BossEnemy *boss = nullptr;
	if (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Player)) {
		player = result.userData.As<Player>();
	} else if (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Boss)) {
		boss = result.userData.As<BossEnemy>();
    }

    // どちらもnullptrなら早期リターン
	if (player == nullptr && boss == nullptr) {
		return;
    }
    // プレイヤーの場合、突進中でなければ早期リターン
    if (player != nullptr && !player->IsRushing()) {
		return;
    }

	// hpを削る
    // プレイヤーの場合、突進溜めレベルに応じてダメージを変える
	if (player != nullptr) {
		int rushLevel = player->GetRushChargeLevel();
		switch (rushLevel) {
			case 1: currentHp_ -= 1; break;
			case 2: currentHp_ -= 2; break;
			case 3: currentHp_ -= 3; break;
			default: currentHp_ -= 1; break;
		}
    } else if (boss != nullptr) {
        // ボスが突進状態であれば
		if (boss->IsRushCollisionActive()) {
			// ボスの場合、固定ダメージ
			currentHp_ -= 2;
		}
    }

	if (currentHp_ <= 0) {
		currentHp_ = 0;
		// 誰が破壊するかによって状態を変える
		wallState_ = WallState::Normal;
	}
}

void Wall::ChangeWallState() {
	switch (wallState_)
	{
	case WallState::Normal:
		material_.SetColor({ 0.8f,0.8f,0.8f,1.0f });
		break;

	case WallState::Strengthen:
		material_.SetColor({ 0.7f,0.7f,0.0f,1.0f });
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