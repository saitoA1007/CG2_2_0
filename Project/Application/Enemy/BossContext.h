#pragma once
#include <optional>
#include"WorldTransform.h"
#include"BossState.h"
#include"Animator.h"

// 敵のアニメーション
enum class enemyAnimationType : size_t {
	BaseMove,  // 基本移動
	Rush,      // 突進
	Scream,    // 叫ぶ
	IceBreath, // ブレス

	MaxCount
};

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
	// 現在の氷柱の数
	uint32_t iceFallCount = 0;
	float waitIceFallMaxTime = 1.0f;

	// 突進攻撃の演出を管理
	bool isRushAttack_ = false;
	// 突進攻撃が壁に有効にするか
	bool isRushCollisionActive_ = false;
	// 突進する速度
	Vector3 rushVelocity = {};

	// 風攻撃の演出を管理
	bool isWindAttack_ = false;
	float windMaxTime_ = 0.0f;

	// アニメーションデータ
	std::array<std::map<std::string, AnimationData>, static_cast<size_t>(enemyAnimationType::MaxCount)>* animationData_;
	GameEngine::Animator* animator_ = nullptr;
	float animationTimer = 0.0f;
	float animationMaxTime = 0.0f;
};