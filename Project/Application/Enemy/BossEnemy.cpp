#define NOMINMAX
#include"BossEnemy.h"
#include<numbers>
#include"GameParamEditor.h"
#include"FPSCounter.h"
#include"LogManager.h"
#include"AudioManager.h"

// 敵の各状態
#include"State/BossStateIn.h"
#include"State/BossStateBattle.h"
#include"State/BossStateOut.h"

// 当たり判定
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
#include"Application/Weapon/Sword.h"
using namespace GameEngine;

void BossEnemy::Initialize(EnemyProjectileManager* projectile, const uint32_t& texture) {

	// ワールド行列を初期化
	worldTransform_.Initialize({ {1.5f,1.5f,1.5f},{0.0f,std::numbers::pi_v<float>,0.0f},{0.0f,2.0f,10.0f} });

	// マテリアルの初期化
	iceMaterial_ = std::make_unique<IceRockMaterial>();
	iceMaterial_->Initialize();
	iceMaterial_->materialData_->textureHandle = texture;

	// コンテキストの設定
	bossContext_.worldTransform = &worldTransform_;
	bossContext_.bossStateRequest_ = std::nullopt;
	bossContext_.projectileManager = projectile;

	// 状態の生成
	statesTable_[static_cast<size_t>(BossState::In)] = std::make_unique<BossStateIn>(bossContext_);
	statesTable_[static_cast<size_t>(BossState::Battle)] = std::make_unique<BossStateBattle>(bossContext_);
	statesTable_[static_cast<size_t>(BossState::Out)] = std::make_unique<BossStateOut>(bossContext_);

	// 最初の状態を設定する
	bossState_ = BossState::In;
	currentState_ = statesTable_[static_cast<size_t>(BossState::In)].get();
	currentState_->Enter();
	Log("BossState : In", "Enemy");

	// 当たり判定を設定
	collider_ = std::make_unique<SphereCollider>();
	collider_->SetWorldPosition(worldTransform_.transform_.translate);
	collider_->SetRadius(collisionRadius_);
	collider_->SetCollisionAttribute(kCollisionAttributeEnemy);
	collider_->SetCollisionMask(~kCollisionAttributeEnemy);
	// データを設定
	UserData userData;
	userData.typeID = static_cast<uint32_t>(CollisionTypeID::Boss);
	//userData.object = this;
	collider_->SetUserData(userData);

	// コールバック関数を登録する
	collider_->SetOnCollisionEnterCallback([this](const CollisionResult& result) {
		this->OnCollisionEnter(result);
	});
	collider_->SetOnCollisionCallback([this](const CollisionResult& result) {
		this->OnCollisionStay(result);
	});

	// 音声を取得
	hitSH_ = AudioManager::GetInstance().GetHandleByName("playerAttack.mp3");

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();	
#endif
	// 値を適応させる
	ApplyDebugParam();

	// hpを設定
	bossContext_.hp = kMaxHp_;
}

void BossEnemy::Update(const Vector3& targetPos) {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

	// ターゲットの位置を更新する
	bossContext_.targetPos = targetPos;

	// 状態変更が有効であれば、切り替える
	if (bossContext_.bossStateRequest_) {
		currentState_->Exit();
		bossState_ = bossContext_.bossStateRequest_.value();
		currentState_ = nullptr;
#ifdef _DEBUG
		// 切り替わった状態のログを出す
		uint32_t i = static_cast<uint32_t>(*bossContext_.bossStateRequest_);
		std::string s = "In";
		if (i == static_cast<uint32_t>(BossState::Battle)) { s = "Battle"; } else if (i == static_cast<uint32_t>(BossState::Out)) { s = "Out"; }
		Log("BossState : " + s, "Enemy");
#endif
		currentState_ = statesTable_[static_cast<size_t>(*bossContext_.bossStateRequest_)].get();
		currentState_->Enter();
		bossContext_.bossStateRequest_ = std::nullopt;
	}

	// 現在の状態の更新処理
	currentState_->Update();

	// 行列の更新
	worldTransform_.UpdateTransformMatrix();

	// 当たり判定の位置を更新
	collider_->SetWorldPosition(worldTransform_.transform_.translate);

	// ヒットした時に点滅する処理
	if (isHit_) {
		hitTimer_ += FpsCounter::deltaTime / maxHitTime_;

		if (static_cast<int>(hitTimer_ * 20.0f) % 2 == 0) {
			alpha_ = 1.0f;
		} else {
			alpha_ = 0.5f;
		}

		iceMaterial_->materialData_->color.w = alpha_;

		if (hitTimer_ >= maxHitTime_) {
			isHit_ = false;
			alpha_ = 1.0f;
			iceMaterial_->materialData_->color.w = alpha_;
			hitTimer_ = 0.0f;
		}
	}
}

void BossEnemy::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {

	if (isHit_) { return; }

	bool isWeapon = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Weapon));

	// 武器が当たった時、ダメージを食らう
	if (isWeapon) {
		Log("isHitBoss");
		Sword* sword = result.userData.As<Sword>();

		if (sword) {
			// ダメージを食らう
			if (bossContext_.hp > 0) {
				if (static_cast<int32_t>(bossContext_.hp) - sword->GetDamage() <= 0) {
					bossContext_.hp = 0;
					isAlive_ = false;
				} else {
					bossContext_.hp -= static_cast<uint32_t>(sword->GetDamage());
				}
			}

			// ヒット音声
			AudioManager::GetInstance().Play(hitSH_, 0.5f, false);
		}
	}
}

void BossEnemy::OnCollisionStay([[maybe_unused]] const GameEngine::CollisionResult& result) {

	bool isWall = (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Wall));

	// 壁に当たった時、押し戻す
	if (isWall) {
		Vector3 n = result.contactNormal;
		Vector3 nXZ = { n.x,0.0f,n.z };
		if (nXZ.x != 0.0f || nXZ.z != 0.0f) { nXZ = Normalize(nXZ); }
		float depth = std::max(result.penetrationDepth, 0.0f);
		Vector3 correction = { nXZ.x * depth, 0.0f, nXZ.z * depth };

		worldTransform_.transform_.translate.x += correction.x;
		worldTransform_.transform_.translate.z += correction.z;
	}
}

Sphere BossEnemy::GetSphereData() {
	return Sphere{ collider_->GetWorldPosition(),collider_->GetRadius() };
}

void BossEnemy::RegisterBebugParam() {

	GameParamEditor::GetInstance()->AddItem("Boss", "MaxHp", kMaxHp_);
	GameParamEditor::GetInstance()->AddItem("Boss", "CollisionSize", collisionRadius_);

	// マテリアル
	GameParamEditor::GetInstance()->AddItem(groupName_, "IceColor", iceMaterial_->materialData_->color);
	GameParamEditor::GetInstance()->AddItem(groupName_, "SpecularColor", specularColor);
	GameParamEditor::GetInstance()->AddItem(groupName_, "RimColor", rimColor);
	GameParamEditor::GetInstance()->AddItem(groupName_, "Shininess", iceMaterial_->materialData_->shininess);
	GameParamEditor::GetInstance()->AddItem(groupName_, "RimIntensity", iceMaterial_->materialData_->rimIntensity);
	GameParamEditor::GetInstance()->AddItem(groupName_, "RimPower", iceMaterial_->materialData_->rimPower);
}

void BossEnemy::ApplyDebugParam() {

	kMaxHp_ = GameParamEditor::GetInstance()->GetValue<uint32_t>("Boss", "MaxHp");
	collisionRadius_ = GameParamEditor::GetInstance()->GetValue<float>("Boss", "CollisionSize");
	collider_->SetRadius(collisionRadius_);

	// マテリアル
	iceMaterial_->materialData_->color = GameParamEditor::GetInstance()->GetValue<Vector4>(groupName_, "IceColor");
	specularColor = GameParamEditor::GetInstance()->GetValue<Vector4>(groupName_, "SpecularColor");
	rimColor = GameParamEditor::GetInstance()->GetValue<Vector4>(groupName_, "RimColor");
	iceMaterial_->materialData_->shininess = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "Shininess");
	iceMaterial_->materialData_->rimIntensity = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "RimIntensity");
	iceMaterial_->materialData_->rimPower = GameParamEditor::GetInstance()->GetValue<float>(groupName_, "RimPower");

	iceMaterial_->materialData_->rimColor.x = rimColor.x;
	iceMaterial_->materialData_->rimColor.y = rimColor.y;
	iceMaterial_->materialData_->rimColor.z = rimColor.z;
	iceMaterial_->materialData_->specularColor.x = specularColor.x;
	iceMaterial_->materialData_->specularColor.y = specularColor.y;
	iceMaterial_->materialData_->specularColor.z = specularColor.z;
}