#pragma once
#include"WorldTransform.h"

/// <summary>
/// ステージ壁オブジェクト
/// </summary>
class StageWallPlane {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="transform"></param>
	void Initialilze(const Transform &transform);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// ワールド行列を取得
	/// </summary>
	/// <returns></returns>
	GameEngine::WorldTransform &GetWorldTransform() { return worldTransform_; }
	const GameEngine::WorldTransform &GetWorldTransform() const { return worldTransform_; }

	/// <summary>
	/// 生存フラグを取得する
	/// </summary>
	/// <returns></returns>
	bool GetIsAlive() const { return isAlive_; }

	/// <summary>
	/// マテリアル用のデバッグパラメータを登録する
	/// </summary>
	static void RegisterDebugParam(class IceMaterial *material);

	/// <summary>
	/// マテリアル用のデバッグパラメータを適応する
	/// </summary>
	/// <param name="material">適応先のIceMaterial</param>
	static void ApplyDebugParam(class IceMaterial* material);

private:
	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// 生存フラグ
	bool isAlive_ = true;
};