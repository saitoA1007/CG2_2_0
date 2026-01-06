#include"RockBullet.h"
#include"GameParamEditor.h"
#include"FPSCounter.h"
#include"RandomGenerator.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
#include"AudioManager.h"
using namespace GameEngine;

RockBullet::RockBullet(EffectManager* effectManager) {
	effectManager_ = effectManager;
}

void RockBullet::Initialize(const Vector3& pos, const Vector3& dir) {
	
	// ワールド行列を初期化
	Vector3 rotate = RandomGenerator::GetVector3(0.0f,6.4f);
	worldTransform_.Initialize({ {5.0f,5.0f,5.0f},rotate,pos });

	// マテリアルの初期化
	iceMaterial_ = std::make_unique<IceRockMaterial>();
	iceMaterial_->Initialize();
	iceMaterial_->materialData_->textureHandle = 0;

	// 当たり判定を設定
	collider_ = std::make_unique<SphereCollider>();
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
	collider_->SetRadius(2.0f);
	collider_->SetCollisionAttribute(kCollisionAttributeEnemy);
	collider_->SetCollisionMask(~kCollisionAttributeEnemy);
	// データを設定
	UserData userData;
	userData.typeID = static_cast<uint32_t>(CollisionTypeID::EnemyBullet);
	//userData.object = this;
	collider_->SetUserData(userData);

	// コールバック関数を登録する
	collider_->SetOnCollisionEnterCallback([this](const CollisionResult& result) {
		this->OnCollisionEnter(result);
	});

	breakSH_ = AudioManager::GetInstance().GetHandleByName("iceBreak.mp3");

#ifdef USE_IMGUI
	RegisterBebugParam();
#endif 
	ApplyDebugParam();

	// 速度を設定
	velocity_ = dir * speed_;

}

void RockBullet::Update() {

	timer_ += FpsCounter::deltaTime / lifeTime_;
	// 生存フラグを無効
	if (timer_ >= 1.0f) {
		isAlive_ = false;
	}

	// 移動
	worldTransform_.transform_.translate += velocity_ * FpsCounter::deltaTime;

	// 行列を更新
	worldTransform_.UpdateTransformMatrix();

	// 当たり判定の位置を更新
	collider_->SetWorldPosition(worldTransform_.GetWorldPosition());

	// 地面に着いたら生存フラグを無効
	if (worldTransform_.transform_.translate.y <= 0.0f) {
		isAlive_ = false;

		// 破壊音
		AudioManager::GetInstance().Play(breakSH_, 0.5f, false);

		// 破壊演出を追加
		effectManager_->AddBreakIceParticle(worldTransform_.GetWorldPosition());
	}
}

void RockBullet::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {
	bool isPlayer = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Player));

	// 接触したら、生存フラグを無効
	if (isPlayer) {
		isAlive_ = false;
		// 破壊音
		AudioManager::GetInstance().Play(breakSH_, 0.5f, false);

		// 破壊演出を追加
		effectManager_->AddBreakIceParticle(worldTransform_.GetWorldPosition());
	}
}

void RockBullet::RegisterBebugParam() {
	// 値の登録
	GameParamEditor::GetInstance()->AddItem(groupName_, "Speed", speed_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "LifeTime", lifeTime_);
}

void RockBullet::ApplyDebugParam() {
	speed_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "Speed");
	lifeTime_ = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "LifeTime");

	// マテリアル
	iceMaterial_->materialData_->color = GameParamEditor::GetInstance()->GetValue<Vector4>("Boss_Material", "IceColor");
	Vector4 specularColor = GameParamEditor::GetInstance()->GetValue<Vector4>("Boss_Material", "SpecularColor");
	Vector4 rimColor = GameParamEditor::GetInstance()->GetValue<Vector4>("Boss_Material", "RimColor");
	iceMaterial_->materialData_->shininess = GameParamEditor::GetInstance()->GetValue<float>("Boss_Material", "Shininess");
	iceMaterial_->materialData_->rimIntensity = GameParamEditor::GetInstance()->GetValue<float>("Boss_Material", "RimIntensity");
	iceMaterial_->materialData_->rimPower = GameParamEditor::GetInstance()->GetValue<float>("Boss_Material", "RimPower");

	iceMaterial_->materialData_->rimColor.x = rimColor.x;
	iceMaterial_->materialData_->rimColor.y = rimColor.y;
	iceMaterial_->materialData_->rimColor.z = rimColor.z;
	iceMaterial_->materialData_->specularColor.x = specularColor.x;
	iceMaterial_->materialData_->specularColor.y = specularColor.y;
	iceMaterial_->materialData_->specularColor.z = specularColor.z;
}

