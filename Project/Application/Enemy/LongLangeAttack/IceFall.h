#pragma once
#include"Collider.h"
#include"WorldTransform.h"
#include"Application/Graphics/PlaneProjectionShadow.h"

class IceFall {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="pos"></param>
	void Initialize(const Vector3& pos);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 行列の取得
	/// </summary>
	/// <returns></returns>
	GameEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

	/// <summary>
	/// コライダー取得
	/// </summary>
	GameEngine::Collider* GetCollider() { return collider_.get(); }

	/// <summary>
	/// 生存フラグを取得
	/// </summary>
	/// <returns></returns>
	bool IsAlive() const { return isAlive_; }

	/// <summary>
	/// 影のワールド行列
	/// </summary>
	/// <returns></returns>
	GameEngine::WorldTransform& GetShadowWorldTransform() { return shadow_->GetWorldTransform(); }

	/// <summary>
	/// 影のマテリアル
	/// </summary>
	/// <returns></returns>
	GameEngine::Material& GetShadowMaterial() { return shadow_->GetMaterial(); }

private:

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// 影
	std::unique_ptr<PlaneProjectionShadow> shadow_;

	// 当たり判定
	std::unique_ptr<GameEngine::SphereCollider> collider_;

	// 生存フラグ
	bool isAlive_ = true;
    // 死亡通知フラグ
    bool isDeadNotified_ = false;

	// 速度
	Vector3 velocity_ = {0.0f,0.0f,0.0f};

	// 落下加速度
	float fallAcceleration_ = 0.2f;

private:

	/// <summary>
	/// 当たり判定
	/// </summary>
	void OnCollision([[maybe_unused]] const GameEngine::CollisionResult& result);
};