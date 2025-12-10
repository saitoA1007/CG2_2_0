#pragma once
#include<cstdint>

/// <summary>
/// 当たり判定がもつID
/// </summary>
enum class CollisionTypeID : uint32_t {
	Default, // 通常
	Player,  // プレイヤー
	Boss,    // ボス
	Wall,    // 壁
    Ground,  // 地面
	IceFall, // つらら
	Wind,    // ボスの風攻撃
	Heart,   // 回復
	BoundaryWall, // 移動範囲制限用の壁
};