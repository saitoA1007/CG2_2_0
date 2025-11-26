#pragma once
#include<list>

#include"LongLangeAttack/IceFall.h"

// ボスの遠距離攻撃を管理する
class EnemyAttackManager {
public:
	EnemyAttackManager() = default;
	~EnemyAttackManager();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 氷柱の発生処理
	/// </summary>
	/// <param name="pos"></param>
	void AddIceFall(const Vector3& pos);

	/// <summary>
	/// 氷柱を生成する
	/// </summary>
	void CreateIceFallPositions();

	/// <summary>
	/// 氷柱のリスト
	/// </summary>
	/// <returns></returns>
	std::list<std::unique_ptr<IceFall>>& GetIceFalls() { return IceFallsList_; }

	/// <summary>
	/// ステージの半径を設定
	/// </summary>
	/// <param name="radius"></param>
	void SetStageRadius(const float& radius) { stageRadius_ = radius; }

private:

	// 氷柱攻撃
	std::list<std::unique_ptr<IceFall>> IceFallsList_;

	// ステージの半径
	float stageRadius_ = 0.0f;
};

// ヘルプ関数
namespace {

	// 2点の距離を計算する
	float GetDistance(const Vector2& c1, const Vector2& c2);
}