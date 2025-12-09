#include"Wall.h"
#include"CollisionConfig.h"
#include"FPSCounter.h"
#include"Application/CollisionTypeID.h"
#include"Application/Player/Player.h"
#include"Application/Enemy/BossEnemy.h"
#include"GameParamEditor.h"
#include"EasingManager.h"
#include "LogManager.h"
using namespace GameEngine;

void Wall::Initialilze(const Transform& transform, float respawnTime, int32_t maxHp, const uint32_t& wallTexture) {

	// 復活までの時間を取得
	respawnTime_ = respawnTime;

	// 最大hpを取得
	maxHp_ = maxHp;
	currentHp_ = maxHp_;

	// ワールド行列を初期化
	worldTransform_.Initialize(transform);

	iceMaterial_ = std::make_unique<IceRockMaterial>();
	iceMaterial_->Initialize();
	iceMaterial_->materialData_->textureHandle = wallTexture;

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

	worldTransform_.transform_ = transform;

#ifdef _DEBUG
	RegisterBebugParam();
#endif
	ApplyDebugParam();
}

void Wall::Update() {
#ifdef _DEBUG
	//ApplyDebugParam();
#endif

	if (currentHp_ <= 0) {
		isAlive_ = false;
		isBreakParticleActive_ = false;
    }

	// 生存フラグがtrueなら早期リターン
	if (isAlive_) {return;}

	respawnTimer_ += FpsCounter::deltaTime / respawnTime_;

	if (respawnTimer_ >= 0.5f) {
		float localT = (respawnTimer_ - 0.5f) / 0.5f;
		float alpha = Lerp(0.0f, 0.6f, EaseIn(localT));
		iceMaterial_->materialData_->color.w = alpha;
	}

	// リスポーン時間を超えたら、復活する
	if (respawnTimer_ >= 1.0f) {
		isAlive_ = true;
		respawnTimer_ = 0.0f;
		currentHp_ = maxHp_;
		iceMaterial_->materialData_->color.w = 0.6f;
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
		iceMaterial_->materialData_->color.w = 0.0f;
		isBreakParticleActive_ = true;
		// 誰が破壊するかによって状態を変える
		wallState_ = WallState::Normal;
	}
}

void Wall::ChangeWallState() {
	switch (wallState_)
	{
	case WallState::Normal:
		//material_.SetColor({ 0.8f,0.8f,0.8f,1.0f });
		break;

	case WallState::Strengthen:
		//material_.SetColor({ 0.7f,0.7f,0.0f,1.0f });
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

void Wall::RegisterBebugParam() {
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "IceColor", iceMaterial_->materialData_->color);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "SpecularColor", specularColor);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "RimColor", rimColor);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "Shininess", iceMaterial_->materialData_->shininess);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "RimIntensity", iceMaterial_->materialData_->rimIntensity);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "RimPower", iceMaterial_->materialData_->rimPower);
	GameParamEditor::GetInstance()->AddItem(kGroupName_, "Scale", worldTransform_.transform_.scale);
}

void Wall::ApplyDebugParam() {
	iceMaterial_->materialData_->color = GameParamEditor::GetInstance()->GetValue<Vector4>(kGroupName_, "IceColor");
	specularColor = GameParamEditor::GetInstance()->GetValue<Vector4>(kGroupName_, "SpecularColor");
	rimColor = GameParamEditor::GetInstance()->GetValue<Vector4>(kGroupName_, "RimColor");
	iceMaterial_->materialData_->shininess = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "Shininess");
	iceMaterial_->materialData_->rimIntensity = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "RimIntensity");
	iceMaterial_->materialData_->rimPower = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "RimPower");
	iceMaterial_->materialData_->rimColor.x = rimColor.x;
	iceMaterial_->materialData_->rimColor.y = rimColor.y;
	iceMaterial_->materialData_->rimColor.z = rimColor.z;
	iceMaterial_->materialData_->specularColor.x = specularColor.x;
	iceMaterial_->materialData_->specularColor.y = specularColor.y;
	iceMaterial_->materialData_->specularColor.z = specularColor.z;

	worldTransform_.transform_.scale = GameParamEditor::GetInstance()->GetValue<Vector3>(kGroupName_, "Scale");
	worldTransform_.UpdateTransformMatrix();
}