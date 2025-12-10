#include"BossEnemy.h"

// 敵の各状態
#include"States/BossStateEgg.h"
#include"States/BossStateIn.h"
#include"States/BossStateBattle.h"
#include"States/BossStateOut.h"

#include<string>
#include "LogManager.h"
#include"FPSCounter.h"
#include"GameParamEditor.h"
#include"AudioManager.h"
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
    statesTable_[static_cast<size_t>(BossState::Egg)] = std::make_unique<BossStateEgg>(bossContext_);
    statesTable_[static_cast<size_t>(BossState::In)] = std::make_unique<BossStateIn>(bossContext_);
    statesTable_[static_cast<size_t>(BossState::Battle)] = std::make_unique<BossStateBattle>(bossContext_, stageRadius, debugRenderer);
    statesTable_[static_cast<size_t>(BossState::Out)] = std::make_unique<BossStateOut>(bossContext_);

    // 最初の状態を設定する（Egg）
    bossState_ = BossState::Egg;
    currentState_ = statesTable_[static_cast<size_t>(BossState::Egg)].get();
    currentState_->Enter();
    Log("BossState : Egg","Enemy");

    // 当たり判定を設定する
    bodyCollider_ = std::make_unique<SphereCollider>();
    bodyCollider_->SetRadius(bodyColliderSize_);
    bodyCollider_->SetWorldPosition(worldTransform_.transform_.translate);
    bodyCollider_->SetCollisionAttribute(kCollisionAttributeEnemy);
    bodyCollider_->SetCollisionMask(~kCollisionAttributeEnemy);

    // データ設定（IDのみ暫定）
    UserData userData;
    userData.typeID = static_cast<uint32_t>(CollisionTypeID::Boss);
    userData.object = this;
    bodyCollider_->SetUserData(userData);
    // 当たり判定の関数を登録する
    bodyCollider_->SetOnCollisionEnterCallback([this](const CollisionResult& result) {
        this->OnCollisionEnter(result);
    });

    // ダメージ音声を取得する
    bossDamagedSH_ = AudioManager::GetInstance().GetHandleByName("BossDamaged.mp3");

#ifdef _DEBUG
    // 値を登録する
    RegisterBebugParam();
#endif
    // 値を適応させる
    ApplyDebugParam();
    bossContext_.hp = kMaxHp_;
}

void BossEnemy::Update(const Vector3& targetPos) {
#ifdef _DEBUG
    // 値を適応
    ApplyDebugParam();
#endif

    if (bossContext_.hp <= 0) {
        if (bossState_ != BossState::Out) {
            bossContext_.bossStateRequest_ = BossState::Out;
        }
    }

    // 攻撃する目標の位置を取得する
    bossContext_.targetPos = targetPos;

    // 状態変更が有効であれば、切り替える
    if (bossContext_.bossStateRequest_) {
        currentState_->Exit();
        bossState_ = bossContext_.bossStateRequest_.value();
        currentState_ = nullptr;
#ifdef _DEBUG
        // 切り替わった状態のログを出す
        uint32_t i = static_cast<uint32_t>(*bossContext_.bossStateRequest_);
        std::string s = "Egg";
        if (i == static_cast<uint32_t>(BossState::In)) {  s = "In"; }
        else if (i == static_cast<uint32_t>(BossState::Battle)) { s = "Battle";}
        else if (i == static_cast<uint32_t>(BossState::Out)) { s = "Out";}
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
        enemyAttackManager_->CreateIceFallPositions(bossContext_.waitIceFallMaxTime);
        //enemyAttackManager_->SetIsRoat(true);
    }

    // 現在の氷柱の数を取得する
    bossContext_.iceFallCount = enemyAttackManager_->GetCurrentIceFall();

    // 風を発射する
    if (bossContext_.isWindAttack_) {
        enemyAttackManager_->StartWindAttack(worldTransform_.transform_.translate, bossContext_.windMaxTime_);
    }

    // 行列を更新する
    worldTransform_.UpdateTransformMatrix();

    // 体の当たり判定の位置を設定する
    bodyCollider_->SetWorldPosition(worldTransform_.transform_.translate + Vector3(0.0f, worldTransform_.transform_.scale.y * 0.7f,0.0f));

    // アニメーションの更新処理
    bossContext_.animator_->NormalizeUpdate(bossContext_.animationTimer);

    // ヒットタイマー
    if (isHit_) {

        timer_ += FpsCounter::deltaTime / hitCoolTime_;

        // ヒット時に点滅させる
        if (static_cast<int>(timer_ * 20.0f) % 2 == 0) {
            alpha_ = 1.0f;
        } else {
            alpha_ = 0.2f;
        }

        // 9割を超えた時点で1.0fに固定する
        if (timer_ >= 0.9f) {
            alpha_ = 1.0f;
        }

        // 終了
        if (timer_ >= 1.0f) {
            isHit_ = false;
            timer_ = 0.0f;
        }
    }
}

void BossEnemy::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {
    if (isHit_) { return; }

    // ヒットした相手がプレイヤーの時
    if (result.userData.typeID == static_cast<uint32_t>(CollisionTypeID::Player)) {

        // プレイヤーデータを取得
        Player* player = nullptr;
        player = result.userData.As<Player>();
        if (player == nullptr) { return; }

        // Egg状態で急降下攻撃を受けたらInへ遷移
        if (bossState_ == BossState::Egg && player->IsAttackDown()) {
            bossContext_.bossStateRequest_ = BossState::In;
            isHit_ = true;
            return;
        }

        // 突進、または上からの攻撃の時のみダメージ
        //player->IsRushing();
        if (player->IsAttackDown()) {

            // フラグ
            isHit_ = true;

            if (bossContext_.hp > 0) {

                if (static_cast<int32_t>(bossContext_.hp) - player->GetAttackDownPower() <= 0) {
                    bossContext_.hp = 0;
                } else {
                    bossContext_.hp -= player->GetAttackDownPower();
                }

                AudioManager::GetInstance().Play(bossDamagedSH_, 0.5f, false);

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
    kMaxHp_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<uint32_t>(kGroupName_, "MaxHp"));
    bodyColliderSize_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "BodyColliderSize");
    scale_ = GameParamEditor::GetInstance()->GetValue<float>(kGroupName_, "Scale");


    bodyCollider_->SetRadius(bodyColliderSize_);

    // スケールを設定
    worldTransform_.transform_.scale = { scale_ ,scale_ ,scale_ };
}