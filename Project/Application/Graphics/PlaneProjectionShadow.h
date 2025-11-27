#pragma once
#include"WorldTransform.h"
#include"Material.h"

class PlaneProjectionShadow {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="casterWorldTransform">影にする</param>
	void Initialize(GameEngine::WorldTransform* casterWorldTransform);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ワールド行列を取得
	/// </summary>
	/// <returns></returns>
	GameEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

	/// <summary>
	/// マテリアルを取得
	/// </summary>
	/// <returns></returns>
	GameEngine::Material& GetMaterial() { return material_; }

private:

	// 壁にするモデルの位置
	GameEngine::WorldTransform* casterWorldTransform_ = nullptr;

	// 影のワールド行列
	GameEngine::WorldTransform worldTransform_;
	// マテリアル
	GameEngine::Material material_;

	// 平面投影行列
	Matrix4x4 shadowMatrix_;
};