#pragma once
#include<list>

#include"ParticleSystem/ParticleBehavior.h"

#include"LongLangeAttack/IceFall.h"

// ボスの遠距離攻撃を管理する
class EnemyAttackManager {
public:

	struct IceFallEffectData {
		float timer = 0.0f;
		bool isActive = false;
		std::unique_ptr<GameEngine::ParticleBehavior> particle;
	};

public:
	EnemyAttackManager() = default;
	~EnemyAttackManager();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(const Matrix4x4& cameraWorldMatrix,const Matrix4x4& viewMatrix);

public:

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

	/// <summary>
	/// 演出データを取得
	/// </summary>
	/// <returns></returns>
	std::vector<IceFallEffectData>& GetIceFallEffectDatas() { return iceFallEffectDatas_; }

private:

	// 氷柱攻撃
	std::list<std::unique_ptr<IceFall>> IceFallsList_;

	// 氷の演出データ
	std::vector<IceFallEffectData> iceFallEffectDatas_;

	// ステージの半径
	float stageRadius_ = 0.0f;

	// 氷柱を落とすまでの時間
	float maxIceFallEmitTime_ = 1.0f;

private:

	/// <summary>
	/// 演出の更新処理
	/// </summary>
	void EffectUpdate(const Matrix4x4& cameraWorldMatrix, const Matrix4x4& viewMatrix);

};

// ヘルプ関数
namespace {

	// 2点の距離を計算する
	float GetDistance(const Vector2& c1, const Vector2& c2);
}