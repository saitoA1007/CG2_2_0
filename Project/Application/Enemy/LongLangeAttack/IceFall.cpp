#include"IceFall.h"
#include"FPSCounter.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"
#include"Application/Player/Player.h"
#include"Application/Enemy/BossEnemy.h"
#include"GameParamEditor.h"
#include"RandomGenerator.h"
#include"EasingManager.h"

using namespace GameEngine;

void IceFall::Initialize(const Vector3& pos, const uint32_t& texture) {

    // ワールド行列を初期化
    float rotY = RandomGenerator::Get(0.0f, 6.4f);
    worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,rotY,0.0f},{pos.x,-10.0f,pos.z} });

    // 当たり判定を設定する
    collider_ = std::make_unique<SphereCollider>();
    collider_->SetRadius(2.0f);
    collider_->SetWorldPosition(worldTransform_.transform_.translate);
    collider_->SetCollisionAttribute(kCollisionAttributeTerrain);
    collider_->SetCollisionMask(~kCollisionAttributeTerrain);
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

    iceMaterial_ = std::make_unique<IceRockMaterial>();
    iceMaterial_->Initialize();
    iceMaterial_->materialData_->textureHandle = texture;

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
    BossEnemy* bossEnemy = result.userData.As<BossEnemy>();
    if (!player && !bossEnemy) {
        return;
    }
    // プレイヤーが突進中なら消滅
    if (player) {
        if (player->IsRushing()) {
            isDeadNotified_ = true;
        }
    }

    if (bossEnemy) {
        if (bossEnemy->IsRushAttack()) {
            isDeadNotified_ = true;
        }
    }
}

void IceFall::RegisterBebugParam() {
    int index = 0;
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "ColliderSize", colliderSize_, index++);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "Scale", scale_, index++);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "MaxTime", maxTime_, index++);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "StartPosY", startPosY, index++);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "EndPosY", endPosY, index++);

    GameParamEditor::GetInstance()->AddItem(kGroupName_, "IceColor", iceMaterial_->materialData_->color);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "SpecularColor", specularColor);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "RimColor", rimColor);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "Shininess", iceMaterial_->materialData_->shininess);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "RimIntensity", iceMaterial_->materialData_->rimIntensity);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "RimPower", iceMaterial_->materialData_->rimPower);

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
}