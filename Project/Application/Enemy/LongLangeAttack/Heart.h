#pragma once
#include"Collider.h"
#include"WorldTransform.h"
#include"Application/Graphics/PlaneProjectionShadow.h"
#include"Material.h"

class Heart {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="pos"></param>
	void Initialize(const Vector3& pos, const uint32_t& texture);

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
	GameEngine::SphereCollider* GetCollider() { return collider_.get(); }

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

	// マテリアルを取得
	GameEngine::Material& GetMaterial() { return material_; }

	// 当たり判定の球データ
	Sphere GetSphereData() { return Sphere(collider_->GetWorldPosition(), collider_->GetRadius()); }

	Vector3 GetWorldPosition() { return worldTransform_.GetWorldPosition(); }

	// 止まる演出
	void SetIsStop(const bool& isStop) { 

		isStop_ = isStop;

		if (!isStop_) {
			timer_ = 0.0f;
			InMove_ = false;
		}
	}

	bool GetIsStop() const { return isStop_; }

private:

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// 影
	std::unique_ptr<PlaneProjectionShadow> shadow_;

	// 当たり判定
	std::unique_ptr<GameEngine::SphereCollider> collider_;

	// マテリアル
	GameEngine::Material material_;

	// 生存フラグ
	bool isAlive_ = true;
	// 死亡通知フラグ
	bool isDeadNotified_ = false;

	bool isStop_ = true;

	float timer_ = 0.0f;

	float startRotateY_ = 0.0f;
	float endRotateY_ = 0.0f;

	// 氷柱の移動時間
	float iceMoveMaxTime_ = 0.4f;
	float startPosY = 0.0f;
	float endPosY = 0.0f;

	bool InMove_ = true;

private: // 調整項目

	float colliderSize_ = 2.0f;
	float scale_ = 1.0f;

	std::string kGroupName_ = "Heart";

	// 回転速度
	float rotMaxTime_ = 2.0f;

	float height_ = 1.0f;

	Vector4 color_ = { 1.0f,1.0f,1.0f,1.0f };

private:

	/// <summary>
	/// 当たり判定
	/// </summary>
	void OnCollision([[maybe_unused]] const GameEngine::CollisionResult& result);

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};