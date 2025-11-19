#include"BossEnemy.h"

#include"States/BossStateIn.h"
#include"States/BossStateBattle.h"
#include"States/BossStateOut.h"

using namespace GameEngine;

void BossEnemy::Initialize(const float& stageRadius) {

    // ワールド行列を初期化
    worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

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

    // 当たり判定の関数を登録する
    bodyCollider_->SetOnCollisionEnterCallback([this](const CollisionResult& result) {
        this->OnCollisionEnter(result);
    });
}

void BossEnemy::Update() {

    // 体の当たり判定の位置を設定する
    //bodyCollider_->SetWorldPosition(worldTransform_.transform_.translate);
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