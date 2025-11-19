#pragma once
#include <optional>
#include"WorldTransform.h"
#include"BossState.h"

// ボスの共通するパラメータ
struct BossContext {
	// ワールド行列
	GameEngine::WorldTransform* worldTransform = nullptr;

	// 体力
	uint32_t hp = 0;

	// 状態の切り替えを管理
	std::optional<BossState> bossStateRequest_ = std::nullopt;

	// アニメーションの状態のリクエストデータ

};