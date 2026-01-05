#pragma once
#include<cstdint>

/// <summary>
/// 当たり判定がもつID
/// </summary>
enum class CollisionTypeID : uint32_t {
	Default, // 通常
	Player,  // プレイヤー
	Weapon,  // 武器
	Boss,    // ボス
	EnemyBullet, // 敵の遠距離攻撃
	Wall,    // 壁
};