#include"BossEnemy.h"

// 敵の各状態
#include"States/BossStateIn.h"
#include"States/BossStateBattle.h"
#include"States/BossStateOut.h"

#include<string>
#include "LogManager.h"
#include"GameParamEditor.h"
#include"CollisionConfig.h"
#include"Application/CollisionTypeID.h"

#include"Application/Player/Player.h"

using namespace GameEngine;

void BossEnemy::Initialize(const float& stageRadius, EnemyAttackManager* enemyAttackManager, GameEngine::Animator* animator,
    std::array<std::map<std::string, AnimationData>, static_cast<size_t>(enemyAnimationType::MaxCount)>* animationData, GameEngine::DebugRenderer* debugRenderer) {

    // 取得
    enemyAttackManager_ = enemyAttackManager;
    enemyAttackManager_->SetStageRadius(stageRadius);

    // ワールド行列を初期化
    worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

    // コンテキストの設定
    bossContext_.worldTransform = &worldTransform_;
    bossContext_.hp = kMaxHp_;
    bossContext_.bossStateRequest_ = std::nullopt;
    bossContext_.animationData_ = animationData;
    bossContext_.animator_ = animator;

    // 状態の生成
    statesTable_[static_cast<size_t>(BossState::In)] = std::make_unique<BossStateIn>(bossContext_);
    statesTable_[static_cast<size_t>(BossState::Battle)] = std::make_unique<BossStateBattle>(bossContext_, stageRadius, debugRenderer);
    statesTable_[static_cast<size_t>(BossState::Out)] = std::make_unique<BossStateOut>(bossContext_);

    // 最初の状態を設定する
    currentState_ = statesTable_[static_cast<size_t>(BossState::In)].get();
    currentState_->Enter();
    Log("BossState : In","Enemy");

    // 当たり判定を設定する
    bodyCollider_ = std::make_unique<SphereCollider>();
    bodyCollider_->SetRadius(bodyColliderSize_);
    bodyCollider_->SetWorldPosition(worldTransform_.transform_.translate);
    bodyCollider_->SetCollisionAttribute(kCollisionAttributeEnemy);
    bodyCollider_->SetCollisionMask(~kCollisionAttributeEnemy);
    // 当たり判定の関数を登録する
    bodyCollider_->SetOnCollisionEnterCallback([this](const CollisionResult& result) {
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

    // 攻撃する目標の位置を取得する
    bossContext_.targetPos = targetPos;

    // 状態変更が有効であれば、切り替える
    if (bossContext_.bossStateRequest_) {
        //currentState_->Exit();
        currentState_ = nullptr;
#ifdef _DEBUG
        // 切り替わった状態のログを出す
        uint32_t i = static_cast<uint32_t>(*bossContext_.bossStateRequest_);
        std::string s = "In";
        if (i == 1) {  s = "Battle"; }
        else if (i == 2) { s = "Out";}
        Log("BossState : " + s, "Enemy");
#endif
        currentState_ = statesTable_[static_cast<size_t>(*bossContext_.bossStateRequest_)].get();
        currentState_->Enter();
        bossContext_.bossStateRequest_ = std::nullopt;
    }

    // 現在の状態の更新処理
    currentState_->Update();

    // 氷柱の発射をする
    if (bossContext_.isActiveIceFall) {
        enemyAttackManager_->CreateIceFallPositions();
        //enemyAttackManager_->SetIsRoat(true);
    }

    // 風を発射する
    if (bossContext_.isWindAttack_) {
        enemyAttackManager_->StartWindAttack(worldTransform_.transform_.translate);
    }

    // 行列を更新する
    worldTransform_.UpdateTransformMatrix();

    // 体の当たり判定の位置を設定する
    bodyCollider_->SetWorldPosition(worldTransform_.transform_.translate);

    // アニメーションの更新処理
    bossContext_.animator_->NormalizeUpdate(bossContext_.animationTimer);
}

void BossEnemy::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {
    
    // ヒットした相手がプレイヤーの時
    if (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Player)) {

        // プレイヤーデータを取得
        Player* player = nullptr;
        player = result.userData.As<Player>();
        if (player == nullptr) { return; }

        // 突進、または上からの攻撃の時のみダメージ
        if (player->IsRushing() || player->IsAttackDown()) {
            if (bossContext_.hp > 0) {
                bossContext_.hp -= 1;

                Log("CurrentHp : " + std::to_string(bossContext_.hp), "Enemy");
            }
        } 
    }
}

Sphere BossEnemy::GetSphereData() {
    return Sphere(bodyCollider_->GetWorldPosition(), bodyCollider_->GetRadius());
}

void BossEnemy::RegisterBebugParam() {
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "MaxHp", kMaxHp_);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "BodyColliderSize", bodyColliderSize_);
    GameParamEditor::GetInstance()->AddItem(kGroupName_, "Scale", scale_);
}

void BossEnemy::ApplyDebugParam() {
    kMaxHp_ = GameParamEditor::GetInstance()->GetValue<uint32_t>(kGroupName_, "MaxHp");
    bodyColliderSize_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "BodyColliderSize");
    scale_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "Scale");


    bodyCollider_->SetRadius(bodyColliderSize_);

    // スケールを設定
    worldTransform_.transform_.scale = { scale_ ,scale_ ,scale_ };
}