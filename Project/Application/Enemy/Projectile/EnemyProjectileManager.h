#pragma once
#include"IProjectile.h"
#include<vector>
#include<memory>

// 生成パラメーター
struct ProjectileSpwanPrams {
	ProjectileType type; // 弾の種類
	Vector3 pos; // 発生位置
	Vector3 dir; // 方向
};

class EnemyProjectileManager {
public:
	EnemyProjectileManager();
	~EnemyProjectileManager();

	void Initialize();

	void Update();

	// 追加
	void AddProjectile(const ProjectileSpwanPrams& prams);

	// 全ての
	std::vector<std::unique_ptr<IProjectile>>& GetProjectiles() {return projectiles_;}

	// 指定したタイプを取得
	std::vector<IProjectile*> GetProjectilesByType(ProjectileType type) const;

private:

	// 敵の全ての遠距離攻撃のインスタンス達
	std::vector<std::unique_ptr<IProjectile>> projectiles_;

private:

	// 生成する
	std::unique_ptr<IProjectile> CreateProjectile(const ProjectileSpwanPrams& prams);

};