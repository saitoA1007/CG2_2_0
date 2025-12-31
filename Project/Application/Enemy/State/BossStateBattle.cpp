#include"BossStateBattle.h"
#include"RandomGenerator.h"
#include"GameParamEditor.h"

// 各攻撃行動
#include"BattleState/StampFall.h"
#include"BattleState/ShotBall.h"
#include"BattleState/Wait.h"

using namespace GameEngine;

BossStateBattle::BossStateBattle(BossContext& context) : bossContext_(context) {

	// 各攻撃行動を登録する
	behaviorsTable_[static_cast<size_t>(BattleBehavior::StampFall)] = std::make_unique<StampFall>(bossContext_);
	behaviorsTable_[static_cast<size_t>(BattleBehavior::ShotBall)] = std::make_unique<ShotBall>(bossContext_);
	behaviorsTable_[static_cast<size_t>(BattleBehavior::Wait)] = std::make_unique<Wait>(bossContext_);

	// 初期シーンを設定する
	currentBehavior_ = BattleBehavior::Wait;
	behaviorsTable_[static_cast<size_t>(currentBehavior_)]->Initialize();

	// 各攻撃行動に重み付けする
	lotteryList_.resize(static_cast<size_t>(BattleBehavior::MaxCount));
	lotteryList_ = {
		{ BattleBehavior::StampFall,stampFallWeight_ }, // スタンプ攻撃
		{ BattleBehavior::Wait,waitWeight_ }, // 待機
	};

#ifdef USE_IMGUI
	RegisterBebugParam();
#endif
	ApplyDebugParam();
}
	
void BossStateBattle::Enter() {
	// 初期化する
	currentBehavior_ = BattleBehavior::Wait;
	behaviorsTable_[static_cast<size_t>(currentBehavior_)]->Initialize();
}
	
void BossStateBattle::Update() {
#ifdef USE_IMGUI
	ApplyDebugParam();
#endif

	// 切り替え処理
	if (behaviorsTable_[static_cast<size_t>(currentBehavior_)]->IsFinished()) {
		// 終了処理をおこなう
		behaviorsTable_[static_cast<size_t>(currentBehavior_)]->Finalize();
		// 次の行動を取得する
		currentBehavior_ = SelectWeightedAttack();
		// 初期化する
		behaviorsTable_[static_cast<size_t>(currentBehavior_)]->Initialize();
	}

	// 指定した状態による更新処理をおこなう
	behaviorsTable_[static_cast<size_t>(currentBehavior_)]->Update();
}
	
void BossStateBattle::Exit() {

}

BossStateBattle::BattleBehavior BossStateBattle::SelectWeightedAttack() {
	BattleBehavior result = BattleBehavior::Wait;

	// 全体の重みを計算する
	int32_t totalWeight = 0;
	for (const auto& item : lotteryList_) {
		totalWeight += item.weight;
	}

	int32_t randomValue = RandomGenerator::Get<int32_t>(0, totalWeight - 1);

	for (const auto& item : lotteryList_) {
		if (randomValue < item.weight) {
			result = item.behavior;
			break;
		}
		// 次の範囲へ進むために値を引く
		randomValue -= item.weight;
	}

	return result;
}

void BossStateBattle::RegisterBebugParam() {
	// 値の登録
	GameParamEditor::GetInstance()->AddItem(groupName_, "StampFallWeight", stampFallWeight_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "ShotBallWeight", shotBallWeight_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "RotateAttackMoveWeight", rotateAttackMoveWeight_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "RushAttackWeight", rushAttackWeight_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "RandBallAttackWeight", randBallAttackWeight_);
	GameParamEditor::GetInstance()->AddItem(groupName_, "WaitWeight", waitWeight_);
}

void BossStateBattle::ApplyDebugParam() {
	// 値の適応
	stampFallWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<int32_t>(groupName_, "StampFallWeight"));
	shotBallWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<int32_t>(groupName_, "ShotBallWeight"));
	rotateAttackMoveWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<int32_t>(groupName_, "RotateAttackMoveWeight"));
	rushAttackWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<int32_t>(groupName_, "RushAttackWeight"));
	randBallAttackWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<int32_t>(groupName_, "RandBallAttackWeight"));
	waitWeight_ = static_cast<int32_t>(GameParamEditor::GetInstance()->GetValue<int32_t>(groupName_, "WaitWeight"));
}