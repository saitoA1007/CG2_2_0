#pragma once
#include <optional>
#include"WorldTransform.h"
#include"BossState.h"

// ボスの共通するパラメータ
struct BossContext {
	// ワールド行列
	GameEngine::WorldTransform* worldTransform = nullptr;

	// 攻撃する対称の位置
	Vector3 targetPos = { 0.0f,0.0f,0.0f };

	// 体力
	uint32_t hp = 0;

	// 状態の切り替えを管理
	std::optional<BossState> bossStateRequest_ = std::nullopt;

	// 氷柱の発射を管理
	bool isActiveIceFall = false;

	// 突進攻撃の演出を管理
	bool isRushAttack_ = false;

	// アニメーションの状態のリクエストデータ

};