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

using namespace GameEngine;

void BossEnemy::Initialize(const float& stageRadius, EnemyAttackManager* enemyAttackManager) {

    // 取得
    enemyAttackManager_ = enemyAttackManager;
    enemyAttackManager_->SetStageRadius(stageRadius);

    // ワールド行列を初期化
    worldTransform_.Initialize({ {5.0f,5.0f,5.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

    // コンテキストの設定
    bossContext_.worldTransform = &worldTransform_;
    bossContext_.hp = kMaxHp_;
    bossContext_.bossStateRequest_ = std::nullopt;

    // 状態の生成
    statesTable_[static_cast<size_t>(BossState::In)] = std::make_unique<BossStateIn>(bossContext_);
    statesTable_[static_cast<size_t>(BossState::Battle)] = std::make_unique<BossStateBattle>(bossContext_, stageRadius);
    statesTable_[static_cast<size_t>(BossState::Out)] = std::make_unique<BossStateOut>(bossContext_);

    // 最初の状態を設定する
    currentState_ = statesTable_[static_cast<size_t>(BossState::In)].get();
    currentState_->Enter();
    Log("BossState : In","Enemy");

    // 当たり判定を設定する
    bodyCollider_ = std::make_unique<SphereCollider>();
    bodyCollider_->SetRadius(4.0f);
    bodyCollider_->SetWorldPosition(worldTransform_.transform_.translate);
    bodyCollider_->SetCollisionAttribute(kCollisionAttributeEnemy);
    bodyCollider_->SetCollisionMask(~kCollisionAttributeEnemy);
    // 当たり判定の関数を登録する
    bodyCollider_->SetOnCollisionEnterCallback([this](const CollisionResult& result) {
        this->OnCollisionEnter(result);
    });
}

void BossEnemy::Update(const Vector3& targetPos) {

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
        //enemyAttackManager_->AddIceFall(worldTransform_.transform_.translate);
        enemyAttackManager_->CreateIceFallPositions();
    }

    // 行列を更新する
    worldTransform_.UpdateTransformMatrix();

    // 体の当たり判定の位置を設定する
    bodyCollider_->SetWorldPosition(worldTransform_.transform_.translate);
}

void BossEnemy::OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result) {
    
    if (bossContext_.hp > 0) {
        bossContext_.hp -= 1;
    }
}

void BossEnemy::RegisterBebugParam() {

}

void BossEnemy::ApplyDebugParam() {

}