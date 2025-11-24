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
	/// 氷柱のリスト
	/// </summary>
	/// <returns></returns>
	std::list<std::unique_ptr<IceFall>>& GetIceFalls() { return IceFallsList_; }

private:

	// 氷柱攻撃
	std::list<std::unique_ptr<IceFall>> IceFallsList_;
};