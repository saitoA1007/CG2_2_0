#pragma once
#include<list>
#include<vector>

// エンジン機能
#include"PostProcess/PostEffectManager.h"
#include"ParticleSystem/ParticleBehavior.h"
#include"Collider.h"

// アプリ機能
#include"LongLangeAttack/IceFall.h"

// ボスの遠距離攻撃を管理する
class EnemyAttackManager {
public:

	struct IceFallEffectData {
		float timer = 0.0f;
		bool isActive = false;
		std::unique_ptr<GameEngine::ParticleBehavior> particle;
	};

	struct windPoint {
		Vector3 pos;
		float radius = 0.0f;
		float startRadius;
		float endRadius;
	};

public:
	EnemyAttackManager() = default;
	~EnemyAttackManager();

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(GameEngine::PostEffectManager* postEffectManager, const uint32_t& texture);

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
	void CreateIceFallPositions(const float& waitIceFallTime);

	/// <summary>
	/// 風ブレスを開始する
	/// </summary>
	void StartWindAttack(const Vector3& pos,const float& maxTime);

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

	// 咆哮を設定
	void SetIsRoat(const bool& isRoat);

	// 風ブレスの状態を取得
	bool IsWind() const { return isWind_; }

	// 当たり判定の取得
	std::vector<std::unique_ptr<GameEngine::SphereCollider>>& GetWindColliders() { return windColliders_; }

	// 風の速度を取得
	Vector3 GetWindVelocity() const { return windVelocity_; }

	// 現在の氷柱の数
	uint32_t GetCurrentIceFall() { return static_cast<uint32_t>(IceFallsList_.size()); }

private:

	// ポストエフェクトの管理
	static GameEngine::PostEffectManager* postEffectManager_;

	// 氷柱攻撃
	std::list<std::unique_ptr<IceFall>> IceFallsList_;

	// 氷の演出データ
	std::vector<IceFallEffectData> iceFallEffectDatas_;

	// ステージの半径
	float stageRadius_ = 0.0f;

	// 氷柱を落とすまでの時間
	float maxIceFallEmitTime_ = 1.0f;
	// 氷柱の調整項目
	float minDistance = 5.0f; // 離れる距離
	int targetCount = 3; // 出す数
	int maxIter = 100; // 回数
	float radiusRatio_ = 0.8f; // ステージの半径に対する使用する割合

	// 咆哮したかのフラグ
	bool isRoat_ = false;

	float roatTimer_ = 0.0f;

	// 風ブレスに使う変数
	bool isWind_ = false;
	std::vector<windPoint> windPositions_;
	float windTimer_ = 0.0f;
	float maxWindTime_ = 5.0f;
	Vector3 centerPos_ = {0.0f,0.0f,0.0f};
	float startAngle_ = 0.0f;
	float endAngle_ = 0.0f;

	// 風の速度
	Vector3 windVelocity_ = {};
	float windSpeed_ = 50.0f;

	uint32_t iceFallTexture_ = 0;

	// 当たり判定
	std::vector<std::unique_ptr<GameEngine::SphereCollider>> windColliders_;

private:

	/// <summary>
	/// 演出の更新処理
	/// </summary>
	void EffectUpdate(const Matrix4x4& cameraWorldMatrix, const Matrix4x4& viewMatrix);

	// 咆哮演出の更新処理
	void RoatUpdate();

	// 風攻撃の更新処理
	void WindUpdate();

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};

// ヘルプ関数
namespace {

	// 2点の距離を計算する
	float GetDistance(const Vector2& c1, const Vector2& c2);

	float EaseOutBounce(float t);

	float LerpShortAngle(float a, float b, float t);
}