#pragma once
#include"Transform.h"
#include"WorldTransform.h"

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
	/// 攻撃開始
	/// </summary>
	virtual void Attack() = 0;

	/// <summary>
	/// 攻撃中かどうかを取得
	/// </summary>
	/// <returns></returns>
	virtual bool IsAttacking() const = 0;

	/// <summary>
	/// 所有者の行列を設定
	/// </summary>
	/// <param name="position"></param>
	virtual void SetOwnerPosition(GameEngine::WorldTransform* OwnerTransform) = 0;
};