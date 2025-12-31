#include"BossEnemy.h"

// 敵の各状態
#include"State/BossStateIn.h"
#include"State/BossStateBattle.h"
#include"State/BossStateOut.h"

#include"CollisionConfig.h"
#include"FPSCounter.h"
#include"Application/Player/Player.h"
#include"Application/CollisionTypeID.h"
#include"LogManager.h"
using namespace GameEngine;

void BossEnemy::Initialize(EnemyProjectileManager* projectile) {

	// ワールド行列を初期化
	worldTransform_.Initialize({ {2.0f,2.0f,2.0f},{0.0f,0.0f,0.0f},{0.0f,2.0f,10.0f} });

	// コンテキストの設定
	bossContext_.worldTransform = &worldTransform_;
	bossContext_.hp = kMaxHp_;
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

	// コールバック関数を登録する
	collider_->SetOnCollisionEnterCallback([this](const CollisionResult& result) {
		this->OnCollisionEnter(result);
	});

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();	
#endif
	// 値を適応させる
	ApplyDebugParam();
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

		if (hitTimer_ >= maxHitTime_) {
			isHit_ = false;
			alpha_ = 1.0f;
			hitTimer_ = 0.0f;
		}
	}
}

void BossEnemy::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {

	if (isHit_) { return; }

	if (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Player)) {
		Player* player = result.userData.As<Player>();

		if (player->GetPlayerBehavior() == Player::Behavior::Jump) {
			Log("isHitBoss");
			bossContext_.hp -= 1;
			isHit_ = true;
			if (bossContext_.hp <= 0) {
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