#include"Heart.h"
#include"FPSCounter.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
#include"Application/Player/Player.h"
#include"Application/Enemy/BossEnemy.h"
#include"GameParamEditor.h"
#include"RandomGenerator.h"
#include"EasingManager.h"
#include<numbers>
using namespace GameEngine;

void Heart::Initialize(const Vector3& pos, const uint32_t& texture) {

    // ワールド行列を初期化
    float rotY = RandomGenerator::Get(0.0f, 6.4f);
    worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,rotY,0.0f},{pos.x,3.0f,pos.z} });

    startRotateY_ = rotY;
    endRotateY_ = rotY + std::numbers::pi_v<float> * 2.0f;

    // 当たり判定を設定する
    collider_ = std::make_unique<SphereCollider>();
    collider_->SetRadius(2.0f);
    collider_->SetWorldPosition(worldTransform_.transform_.translate);
    collider_->SetCollisionAttribute(kCollisionAttributeEnemy);
    collider_->SetCollisionMask(~kCollisionAttributeEnemy);
    UserData userData;
    userData.typeID = static_cast<uint32_t>(CollisionTypeID::Heart);
    collider_->SetUserData(userData);

    // プレイヤーの影
    shadow_ = std::make_unique<PlaneProjectionShadow>();
    shadow_->Initialize(&worldTransform_);

    // 当たり判定の関数を登録する
    collider_->SetOnCollisionCallback([this](const CollisionResult& result) {
        this->OnCollision(result);
        });

    // マテリアルを初期化
    material_.Initialize({ 1.0f,1.0f,1.0f,1.0f }, { 1.0f,1.0f,1.0f }, 10.0f, false);
    material_.SetTextureHandle(texture);

    isStop_ = true;
    InMove_ = true;

#ifdef _DEBUG
    // 値を登録する
    RegisterBebugParam();
#endif
    // 値を適応させる
    ApplyDebugParam();

    // 当たり判定の位置を更新
    collider_->SetWorldPosition(worldTransform_.transform_.translate);
}

void Heart::Update() {
#ifdef _DEBUG
    ApplyDebugParam();
#endif

    if (isDeadNotified_) {
        isAlive_ = false;
        return;
    }

    if (isStop_) {

        if (InMove_) {
            if (timer_ <= 1.0f) {
                timer_ += FpsCounter::deltaTime / iceMoveMaxTime_;
                worldTransform_.transform_.translate.y = Lerp(startPosY, endPosY, timer_ * timer_ * timer_);
                // 行列の更新処理
                worldTransform_.UpdateTransformMatrix();
            } else {
                timer_ = 0.0f;
                InMove_ = false;
            }
        }
    } else {

        timer_ += FpsCounter::deltaTime / rotMaxTime_;

        if (timer_ <= 0.5f) {
            float localT = timer_ / 0.5f;
            worldTransform_.transform_.translate.y = endPosY + Lerp(-height_, height_, EaseInOut(localT));
        } else {
            float localT = (timer_ - 0.5f) / 0.5f;
            worldTransform_.transform_.translate.y = endPosY + Lerp(height_, -height_, EaseInOut(localT));
        }

        worldTransform_.transform_.rotate.y = Lerp(startRotateY_, endRotateY_, timer_);

        if (timer_ >= 1.0f) {
            timer_ = 0.0f;
        }

        // 行列の更新処理
        //shadow_->Update();
        worldTransform_.UpdateTransformMatrix();
    }

    // 影の更新処理
    //shadow_->Update();
}

void Heart::OnCollision([[maybe_unused]] const GameEngine::CollisionResult& result) {
    Player* player = dynamic_cast<Player*>(result.userData.object);
    if (!player) {
        return;
    }
    // プレイヤーが突進中なら消滅
    if (player) {
        isDeadNotified_ = true;
    }
}

void Heart::RegisterBebugParam() {
    int index = 0;
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "ColliderSize", colliderSize_, index++);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "Scale", scale_, index++);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "RotMaxTime", rotMaxTime_, index++);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "Height", height_, index++);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "StartPosY", startPosY, index++);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "EndPosY", endPosY, index++);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "Color", color_, index++);
}

void Heart::ApplyDebugParam() {
    colliderSize_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "ColliderSize");
    scale_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "Scale");
    rotMaxTime_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "RotMaxTime");
    height_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "Height");
    startPosY = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "StartPosY");
    endPosY = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "EndPosY");

    color_ = GameParamEditor::GetInstance()->GetValue<Vector4>(kGroupName_, "Color");

    material_.SetColor(color_);

    // 当たり判定を適応
    collider_->SetRadius(colliderSize_);
    // スケールを設定
    worldTransform_.transform_.scale = { scale_ ,scale_ ,scale_ };
    worldTransform_.UpdateTransformMatrix();
}