#include"BossStateBattle.h"
#include"Application/Enemy/BossState.h"
using namespace GameEngine;

BossStateBattle::BossStateBattle(BossContext& context, const float& stageRadius) : bossContext_(context) {

	// ステージの半径を取得
	stageRadius_ = stageRadius;


#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
#else
	// 値を適応させる
	ApplyDebugParam();
#endif
}

void BossStateBattle::Enter() {

}

void BossStateBattle::Update() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif



}

void BossStateBattle::Exit() {

}

void BossStateBattle::RegisterBebugParam() {

}

void BossStateBattle::ApplyDebugParam() {

}