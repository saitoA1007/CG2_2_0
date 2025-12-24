#pragma once
#include <optional>
#include"WorldTransform.h"
#include"BossState.h"

struct BossContext {
	// ワールド行列
	GameEngine::WorldTransform* worldTransform = nullptr;

	// 攻撃する対称の位置
	Vector3 targetPos = { 0.0f,0.0f,0.0f };

	// 体力
	uint32_t hp = 0;

	// 状態の切り替えを管理
	std::optional<BossState> bossStateRequest_ = std::nullopt;
};