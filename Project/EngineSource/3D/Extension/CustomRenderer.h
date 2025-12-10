#pragma once
#include <d3d12.h>
#include<vector>
#include <unordered_map>
#include <wrl.h>

#include"SrvManager.h"

#include"PSO/Core/PSOManager.h"
#include"PSO/Core/DrawPSOData.h"

#include"Model.h"
#include"CustomMaterial/ICustomMaterial.h"
#include"CustomMaterial/IceRockMaterial.h"
#include"CustomMaterial/BossMaterial.h"

using namespace GameEngine;

enum class CustomRenderMode {
	Ice, // 氷
	Rock, // 背景の岩
	RockBack, // 背面描画
	RockBoth, // 両面描画
	BossAnimation, // ボスのアニメーション

	MaxCount
};

class CustomRenderer {
public:
	CustomRenderer() = default;
	~CustomRenderer() = default;

	/// <summary>
	/// 静的初期化
	/// </summary>
	/// <param name="commandList"></param>
	/// <param name="srvManager"></param>
	/// <param name="psoManager"></param>
	static void StaticInitialize(ID3D12Device* device,ID3D12GraphicsCommandList* commandList, SrvManager* srvManager, PSOManager* psoManager);

	/// <summary>
	/// 描画前処理
	/// </summary>
	/// <param name="mode"></param>
	static void PreDraw(CustomRenderMode mode);

	/// <summary>
	/// カメラを設定
	/// </summary>
	/// <param name="vpMatrix"></param>
	/// <param name="cameraResource"></param>
	static void SetCamera(const Matrix4x4& vpMatrix, ID3D12Resource* cameraResource);

public:

	/// <summary>
	/// 氷を描画
	/// </summary>
	/// <param name="model"></param>
	/// <param name="worldTransform"></param>
	/// <param name="material"></param>
	static void DrawIce(const Model* model, WorldTransform& worldTransform, ID3D12Resource* lightGroupResource,ICustomMaterial* material);

	static void DrawRock(const Model* model, WorldTransform& worldTransform, ID3D12Resource* lightGroupResource, IceRockMaterial* material);

	static void DrawAnimationWithLight(const Model* model, WorldTransform& worldTransform, ID3D12Resource* lightGroupResource, BossMaterial* material);

private:

	// コマンドリスト
	static ID3D12GraphicsCommandList* commandList_;

	// psoデータのリスト
	static std::unordered_map<CustomRenderMode, DrawPsoData> psoList_;

	// srv
	static SrvManager* srvManager_;

	// カメラ行列
	static Matrix4x4 vpMatrix_;
	// カメラリソース
	static ID3D12Resource* cameraResource_;
};