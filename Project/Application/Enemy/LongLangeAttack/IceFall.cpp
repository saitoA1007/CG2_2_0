#include"IceFall.h"
#include"FPSCounter.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
#include"Application/Player/Player.h"
#include"GameParamEditor.h"
#include"RandomGenerator.h"
#include"EasingManager.h"

using namespace GameEngine;

void IceFall::Initialize(const Vector3& pos) {

    // ワールド行列を初期化
    float rotY = RandomGenerator::Get(0.0f, 6.4f);
    worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,rotY,0.0f},pos});

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

#ifdef _DEBUG
    // 値を登録する
    RegisterBebugParam();
#endif
    // 値を適応させる
    ApplyDebugParam();
}

void IceFall::Update() {
#ifdef _DEBUG
    ApplyDebugParam();
#endif

    if (isDeadNotified_) {
        isAlive_ = false;
        return;
    }

    // 影の更新処理
    shadow_->Update();

    if (timer_ <= 1.0f) {

        timer_ += FpsCounter::deltaTime / maxTime_;

        worldTransform_.transform_.translate.y = Lerp(startPosY, endPosY, EaseIn(timer_));

        // 行列の更新処理
        worldTransform_.UpdateTransformMatrix();

        // 当たり判定の位置を更新
        collider_->SetWorldPosition(worldTransform_.transform_.translate);
    }

#ifdef _DEBUG
    worldTransform_.UpdateTransformMatrix();
#endif
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

void IceFall::RegisterBebugParam() {
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "ColliderSize", colliderSize_);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "Scale", scale_);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "MaxTime", maxTime_);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "StartPosY", startPosY);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "EndPosY", endPosY);
}

void IceFall::ApplyDebugParam() {
    colliderSize_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "ColliderSize");
    scale_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "Scale");
    maxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "MaxTime");
    startPosY = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "StartPosY");
    endPosY = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "EndPosY");

    // 当たり判定を適応
    collider_->SetRadius(colliderSize_);
    // スケールを設定
    worldTransform_.transform_.scale = { scale_ ,scale_ ,scale_ };
}