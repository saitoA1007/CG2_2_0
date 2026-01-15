#pragma once
#include"WorldTransform.h"
#include"Extension/CustomMaterial/IceMaterial.h"

/// <summary>
/// 地面オブジェクト
/// </summary>
class Terrain {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="baseTexture"></param>
	/// <param name="iceTexture"></param>
	/// <param name="iceNormalTex"></param>
	void Initialize(const uint32_t& baseTexture, const uint32_t& iceTexture, const uint32_t& iceNormalTex);

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
	IceMaterial* GetMaterial() { return iceMaterial_.get(); }

private:
	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// マテリアル
	std::unique_ptr<IceMaterial> iceMaterial_;

	// デバック用
	Vector4 rimColor;
	Vector4 specularColor;

private:

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterDebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};