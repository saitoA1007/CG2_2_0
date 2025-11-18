#pragma once
#include"IWeapon.h"

class Sword : public IWeapon {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize() override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// 攻撃開始
	/// </summary>
	void Attack() override;

	/// <summary>
	/// 攻撃中かどうかを取得
	/// </summary>
	/// <returns></returns>
	bool IsAttacking() const override;

	/// <summary>
	/// 所有者の行列を設定
	/// </summary>
	/// <param name="position"></param>
	void SetOwnerPosition(GameEngine::WorldTransform* OwnerTransform) override { worldTransform_.SetParent(OwnerTransform); };

private:

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

};