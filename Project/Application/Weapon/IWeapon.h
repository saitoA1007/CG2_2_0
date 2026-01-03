#pragma once
#include"Transform.h"
#include"WorldTransform.h"
#include"Collider.h"
#include<vector>

class IWeapon {
public:
	virtual ~IWeapon() = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	virtual void Initialize() = 0;

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 所有者の行列を設定
	/// </summary>
	/// <param name="position"></param>
	virtual void SetOwnerPosition(GameEngine::WorldTransform* OwnerTransform) = 0;

	/// <summary>
	/// トランスフォーム
	/// </summary>
	/// <param name="transform"></param>
	virtual void SetTransform(const Transform& transform) = 0;

	/// <summary>
	/// ワールド行列を取得
	/// </summary>
	/// <returns></returns>
	virtual GameEngine::WorldTransform& GetWorldTransform() = 0;

	/// <summary>
	/// 当たり判定を取得
	/// </summary>
	/// <returns></returns>
	virtual std::vector<GameEngine::Collider*> GetCollider() = 0;
};