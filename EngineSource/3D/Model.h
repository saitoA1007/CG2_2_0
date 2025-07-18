#pragma once
#include <d3d12.h>
#include<vector>
#include <wrl.h>

#include"VertexData.h"
#include"Mesh.h"
#include"Material.h"
#include"WorldTransforms.h"

#include"EngineSource/Math/TransformationMatrix.h"
#include"EngineSource/Common/LogManager.h"
#include"EngineSource/Core/PSO/TrianglePSO.h"
#include"EngineSource/Core/PSO/ParticlePSO.h"
#include"EngineSource/3D/Light/LightManager.h"
#include"EngineSource/3D/Camera/Camera.h"

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include"EngineSource/Core/PSO/GridPSO.h"

namespace GameEngine {

	// テクスチャの前方宣言
	class TextureManager;
	class WorldTransform;

	enum class PSOMode {
		Triangle, // 単体描画用
		Partilce, // 複数描画用
		Grid,  // グリッド描画用
	};
	
	class Model final {
	public:
		Model() = default;
		~Model() = default;

		/// <summary>
		/// 静的初期化
		/// </summary>
		/// <param name="device"></param>
		/// <param name="commandList"></param>
		static void StaticInitialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, TextureManager* textureManager,TrianglePSO* trianglePSO, ParticlePSO* particlePSO, GridPSO* gridPSO, LogManager* logManager);

		/// <summary>
		/// 描画前処理
		/// </summary>
		static void PreDraw(PSOMode psoMode,BlendMode blendMode);

		/// <summary>
		/// 描画前処理
		/// </summary>
		/// <param name="drawMode"></param>
		static void PreDraw(DrawModel drawMode);

		/// <summary>
		/// OBJファイルからメッシュ生成
		/// </summary>
		/// <param name="objFilename">.objファイル名</param>
		/// <param name="filename">格納ファイル名</param>
		/// <returns></returns>
		[[nodiscard]]
		static Model* CreateModel(const std::string& objFilename, const std::string& filename);

		/// <summary>
		/// 球モデル生成
		/// </summary>
		/// <returns>生成されたモデル</returns>
		[[nodiscard]]
		static Model* CreateSphere(uint32_t subdivision);

		/// <summary>
		/// 三角形の平面を生成
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		static Model* CreateTrianglePlane();

		/// <summary>
		/// グリッドを描画
		/// </summary>
		/// <param name="size">グリッドサイズ</param>
		/// <returns></returns>
		[[nodiscard]]
		static Model* CreateGridPlane(const Vector2& size);

		/// <summary>
		/// 生成したモデルを描画
		/// </summary>
		/// <param name="worldMatrix">ワールド行列</param>
		/// <param name="directionalLightResource">光源</param>
		/// <param name="textureHandle">テクスチャハンドル</param>
		/// <param name="material">マテリアル : 何の書かなければデフォルトのマテリアルを適応</param>
		void Draw(WorldTransform& worldTransform, const uint32_t& textureHandle, const Matrix4x4& VPMatrix,const Material* material = nullptr);

		/// <summary>
		/// 生成したモデルを複数描画
		/// </summary>
		/// <param name="worldTransforms"></param>
		/// <param name="textureHandle"></param>
		/// <param name="VPMatrix"></param>
		/// <param name="material"></param>
		void Draw(const uint32_t& numInstance,WorldTransforms& worldTransforms, const uint32_t& textureHandle, const Matrix4x4& VPMatrix, const Material* material = nullptr);

		/// <summary>
		/// グリッドを描画
		/// </summary>
		void DrawGrid(WorldTransform& worldTransform, const Matrix4x4& VPMatrix, ID3D12Resource* cameraResource);

		/// <summary>
		/// モデルに光源を適応させる
		/// </summary>
		/// <param name="directionalLightResource"></param>
		void DrawLight(ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource);

		/// <summary>
		/// デフォルトの色を設定
		/// </summary>
		/// <param name="color"></param>
		void SetDefaultColor(const Vector4& color);

		/// <summary>
		/// 鏡面反射の色を設定
		/// </summary>
		/// <param name="specularColor"></param>
		void SetDefaultSpecularColor(const Vector3& specularColor);

		/// <summary>
		/// 輝度の設定
		/// </summary>
		/// <param name="shininess"></param>
		void SetDefaultShiness(const float& shininess);

		/// <summary>
		/// デフォオルトの光源の有無を設定
		/// </summary>
		/// <param name="isEnableLight"></param>
		void SetDefaultIsEnableLight(const bool& isEnableLight);

		/// <summary>
		/// デフォルトのuvMatrixを設定
		/// </summary>
		/// <param name="uvMatrix"></param>
		void SetDefaultUVMatrix(const Matrix4x4& uvMatrix);

	private:

		/// <summary>
		/// モデルデータのファイル読み込み
		/// </summary>
		/// <param name="directoryPath"></param>
		/// <param name="objFilename"></param>
		/// <param name="filename"></param>
		/// <returns></returns>
		[[nodiscard]]
		ModelData LoadModelFile(const std::string& directoryPath, const std::string& objFilename, const std::string& filename);

	private:
		Model(Model&) = delete;
		Model& operator=(Model&) = delete;

		// デバイス
		static ID3D12Device* device_;
		// コマンドリスト
		static ID3D12GraphicsCommandList* commandList_;

		// ログ
		static LogManager* logManager_;

		// PSO設定
		static TrianglePSO* trianglePSO_;
		static ParticlePSO* particlePSO_;
		static GridPSO* gridPSO_;

		// テクスチャ
		static TextureManager* textureManager_;

		// メッシュ
		std::unique_ptr<Mesh> mesh_ = nullptr;

		// デフォルトのマテリアル
		std::unique_ptr<Material> defaultMaterial_ = nullptr;

		// Nodeのローカル行列を保持しておく変数
		Matrix4x4 localMatrix_;

	private:

		/// <summary>
		/// Node情報を取得
		/// </summary>
		/// <param name="node"></param>
		/// <returns></returns>
		[[nodiscard]]
		Node ReadNode(aiNode* node);
	};
}