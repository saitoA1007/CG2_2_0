#include"IceFall.h"
#include"FPSCounter.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"

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
    // 当たり判定の関数を登録する
    collider_->SetOnCollisionEnterCallback([this](const CollisionResult& result) {
        this->OnCollisionEnter(result);
    });
}

void IceFall::Update() {

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

void IceFall::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {

    // 生存フラグをfalse
    isAlive_ = false;
}