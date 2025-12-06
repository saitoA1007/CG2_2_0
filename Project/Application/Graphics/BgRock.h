#pragma once
#pragma once
#include"WorldTransform.h"
#include"Extension/CustomMaterial/IceRockMaterial.h"

/// <summary>
/// 地面オブジェクト
/// </summary>
class BgRock {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(const uint32_t& texture);

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
	IceRockMaterial* GetMaterial() { return iceMaterial_.get(); }

private:
	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// マテリアル
	std::unique_ptr<IceRockMaterial> iceMaterial_;

	// デバック用
	Vector4 rimColor;
	Vector4 specularColor;

private:

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};