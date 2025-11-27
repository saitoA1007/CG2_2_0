#include"IceFall.h"
#include"FPSCounter.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
#include"Application/Player/Player.h"

using namespace GameEngine;

void IceFall::Initialize(const Vector3& pos) {

    // ワールド行列を初期化
    worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},pos});

    // 当たり判定を設定する
    collider_ = std::make_unique<SphereCollider>();
    collider_->SetRadius(2.0f);
    collider_->SetWorldPosition(worldTransform_.transform_.translate);
    collider_->SetCollisionAttribute(kCollisionAttributeEnemy);
    collider_->SetCollisionMask(~kCollisionAttributeEnemy);
    UserData userData;
    userData.typeID = static_cast<uint32_t>(CollisionTypeID::IceFall);
    collider_->SetUserData(userData);

    // プレイヤーの影
    shadow_ = std::make_unique<PlaneProjectionShadow>();
    shadow_->Initialize(&worldTransform_);

    // 当たり判定の関数を登録する
    collider_->SetOnCollisionCallback([this](const CollisionResult& result) {
        this->OnCollision(result);
    });
}

void IceFall::Update() {
    if (isDeadNotified_) {
        isAlive_ = false;
        return;
    }

    // 影の更新処理
    shadow_->Update();

    // 地面に着地していなければ移動する
    if (worldTransform_.transform_.translate.y > 0.0f) {

        // 速度
        velocity_.y += -fallAcceleration_ * FpsCounter::deltaTime;

        // 移動
        worldTransform_.transform_.translate += velocity_;

        // 行列の更新処理
        worldTransform_.UpdateTransformMatrix();

        // 当たり判定の位置を更新
        collider_->SetWorldPosition(worldTransform_.transform_.translate);
    }    
}

void IceFall::OnCollision([[maybe_unused]] const GameEngine::CollisionResult& result) {
    Player *player = dynamic_cast<Player *>(result.userData.object);
    if (!player) {
        return;
    }
    // プレイヤーが突進中なら消滅
    if (player->IsRushing()) {
        isDeadNotified_ = true;
    }
}