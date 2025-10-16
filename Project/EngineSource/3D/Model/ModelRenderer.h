#pragma once
#include <d3d12.h>
#include<vector>
#include <unordered_map>
#include <wrl.h>

//#include"VertexData.h"
//#include"Mesh.h"
//#include"Material.h"
//#include"WorldTransform.h"
//#include"WorldTransforms.h"
//#include"AnimationData.h"
//#include"Animation.h"

#include"TextureManager.h"
#include"TrianglePSO.h"
#include"ParticlePSO.h"
#include"GridPSO.h"
#include"BasePSO.h"
#include"AnimationPSO.h"

#include"Model.h"

namespace GameEngine {

	// モデルを描画するモード
	enum class RenderMode {
		DefaultModel, // 通常モデルを描画用
		Instancing,   // インスタンシング描画用
		Grid,  // グリッド描画用
		AnimationModel, // アニメーション描画用
	};

	class ModelRenderer {
	public:
		ModelRenderer() = default;
		~ModelRenderer() = default;

		/// <summary>
		/// 静的初期化
		/// </summary>
		/// <param name="device"></param>
		/// <param name="commandList"></param>
		static void StaticInitialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, TextureManager* textureManager,
			TrianglePSO* trianglePSO, ParticlePSO* particlePSO, AnimationPSO* animationPSO, GridPSO* gridPSO);

		/// <summary>
		/// 描画前処理
		/// </summary>
		/// <param name="mode"></param>
		/// <param name="blendMode"></param>
		static void PreDraw(RenderMode mode, BlendMode blendMode);

		/// <summary>
		/// 描画前処理
		/// </summary>
		/// <param name="drawMode">描画状態を設定</param>
		static void PreDraw(DrawModel drawMode);

	public:

		/// <summary>
		/// 生成したモデルを描画(画像差し替え用)
		/// </summary>
		/// <param name="worldMatrix">ワールド行列</param>
		/// <param name="directionalLightResource">光源</param>
		/// <param name="textureHandle">テクスチャハンドル</param>
		/// <param name="material">マテリアル : 何の書かなければデフォルトのマテリアルを適応</param>
		static void Draw(const Model* model,WorldTransform& worldTransform, const uint32_t& textureHandle, const Matrix4x4& VPMatrix, const Material* material = nullptr);

		/// <summary>
		/// 生成したモデルを描画(元のテクスチャを使用)
		/// </summary>
		/// <param name="worldMatrix">ワールド行列</param>
		/// <param name="directionalLightResource">光源</param>
		/// <param name="material">マテリアル : 何の書かなければデフォルトのマテリアルを適応</param>
		static void Draw(const Model* model, WorldTransform& worldTransform, const Matrix4x4& VPMatrix, const Material* material = nullptr);

		/// <summary>
		/// 生成したモデルの描画(ライト適応、複数マテリアル対応)
		/// </summary>
		/// <param name="worldTransform"></param>
		/// <param name="VPMatrix"></param>
		/// <param name="lightGroupResource"></param>
		/// <param name="cameraResource"></param>
		/// <param name="material"></param>
		static void Draw(const Model* model, WorldTransform& worldTransform, const Matrix4x4& VPMatrix, ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource, const Material* material = nullptr);

		/// <summary>
		/// 生成したモデルの複数描画
		/// </summary>
		/// <param name="worldTransforms"></param>
		/// <param name="textureHandle"></param>
		/// <param name="VPMatrix"></param>
		/// <param name="material"></param>
		static void Draw(const Model* model, const uint32_t& numInstance, WorldTransforms& worldTransforms, const uint32_t& textureHandle, const Matrix4x4& VPMatrix, const Material* material = nullptr);

		/// <summary>
		/// 生成したモデルの複数描画
		/// </summary>
		/// <param name="worldTransforms"></param>
		/// <param name="textureHandle"></param>
		/// <param name="VPMatrix"></param>
		/// <param name="material"></param>
		static void Draw(const Model* model, const uint32_t& numInstance, WorldTransforms& worldTransforms, const Matrix4x4& VPMatrix, const Material* material = nullptr);

		/// <summary>
		/// グリッドを描画
		/// </summary>
		static void DrawGrid(const Model* model, WorldTransform& worldTransform, const Matrix4x4& VPMatrix, ID3D12Resource* cameraResource);

		/// <summary>
		/// アニメーションのあるモデルを描画
		/// </summary>
		/// <param name="worldTransform"></param>
		/// <param name="VPMatrix"></param>
		/// <param name="material"></param>
		static void DrawAnimation(const Model* model, WorldTransform& worldTransform, const Matrix4x4& VPMatrix, const SkinCluster& skinCluster, const Material* material = nullptr);

		/// <summary>
		/// モデルに光源を適応させる
		/// </summary>
		/// <param name="directionalLightResource"></param>
		static void DrawLight(ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource);

	private:

		// デバイス
		static ID3D12Device* device_;
		// コマンドリスト
		static ID3D12GraphicsCommandList* commandList_;

		// PSO設定
		static TrianglePSO* trianglePSO_;
		static ParticlePSO* particlePSO_;
		static GridPSO* gridPSO_;
		static AnimationPSO* animationPSO_;

		// テクスチャ
		static TextureManager* textureManager_;
	};
}
