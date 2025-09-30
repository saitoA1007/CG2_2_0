#pragma once
#include <d3d12.h>
#include<vector>
#include <unordered_map>
#include <wrl.h>

#include"VertexData.h"
#include"Mesh.h"
#include"Material.h"
#include"WorldTransform.h"
#include"WorldTransforms.h"
#include"AnimationData.h"
#include"Animation.h"

#include"LogManager.h"

#include"TextureManager.h"
#include"TrianglePSO.h"
#include"ParticlePSO.h"
#include"GridPSO.h"
#include"BasePSO.h"

#include"LightManager.h"
#include"Camera.h"

#include"TransformationMatrix.h"

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

namespace GameEngine {

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
		/// <param name="psoMode">描画モードを設定</param>
		/// <param name="blendMode">ブレンドモードを設定</param>
		static void PreDraw(PSOMode psoMode,BlendMode blendMode);

		/// <summary>
		/// 描画前処理
		/// </summary>
		/// <param name="drawMode">描画状態を設定</param>
		static void PreDraw(DrawModel drawMode);

		/// <summary>
		/// 描画前処理
		/// </summary>
		/// <param name="pso">psoを設定</param>
		static void PreDraw(BasePSO* pso);

		/// <summary>
		/// OBJファイルからメッシュ生成
		/// </summary>
		/// <param name="objFilename">.objファイル名</param>
		/// <param name="filename">格納ファイル名</param>
		/// <returns></returns>
		[[nodiscard]]
		static std::unique_ptr<Model> CreateModel(const std::string& objFilename, const std::string& filename);

		/// <summary>
		/// 球モデル生成
		/// </summary>
		/// <returns>生成されたモデル</returns>
		[[nodiscard]]
		static std::unique_ptr<Model> CreateSphere(uint32_t subdivision);

		/// <summary>
		/// 三角形の平面を生成
		/// </summary>
		/// <returns></returns>
		[[nodiscard]]
		static std::unique_ptr<Model> CreateTrianglePlane();

		/// <summary>
		/// 平面モデルを生成
		/// </summary>
		/// <param name="size"></param>
		/// <returns></returns>
		[[nodiscard]]
		static std::unique_ptr<Model> CreatePlane(const Vector2& size);

		/// <summary>
		/// グリッドを描画
		/// </summary>
		/// <param name="size">グリッドサイズ</param>
		/// <returns></returns>
		[[nodiscard]]
		static std::unique_ptr<Model> CreateGridPlane(const Vector2& size);

		/// <summary>
		/// アニメーションデータを読み込み
		/// </summary>
		/// <param name="directoryPath"></param>
		/// <param name="objFilename"></param>
		/// <param name="filename"></param>
		/// <returns></returns>
		[[nodiscard]]
		static AnimationData LoadAnimationFile(const std::string& objFilename, const std::string& filename);

		/// <summary>
		/// スケルトンを作成する
		/// </summary>
		/// <param name="rootNode"></param>
		/// <returns></returns>
		[[nodiscard]]
		static Skeleton CreateSkeleton(const Node& rootNode);

	public:

		/// <summary>
		/// 生成したモデルを描画(画像差し替え用)
		/// </summary>
		/// <param name="worldMatrix">ワールド行列</param>
		/// <param name="directionalLightResource">光源</param>
		/// <param name="textureHandle">テクスチャハンドル</param>
		/// <param name="material">マテリアル : 何の書かなければデフォルトのマテリアルを適応</param>
		void Draw(WorldTransform& worldTransform, const uint32_t& textureHandle, const Matrix4x4& VPMatrix, const Material* material = nullptr);

		/// <summary>
		/// 生成したモデルを描画(元のテクスチャを使用)
		/// </summary>
		/// <param name="worldMatrix">ワールド行列</param>
		/// <param name="directionalLightResource">光源</param>
		/// <param name="material">マテリアル : 何の書かなければデフォルトのマテリアルを適応</param>
		void Draw(WorldTransform& worldTransform, const Matrix4x4& VPMatrix, const Material* material = nullptr);

		/// <summary>
		/// 生成したモデルの描画(ライト適応、複数マテリアル対応)
		/// </summary>
		/// <param name="worldTransform"></param>
		/// <param name="VPMatrix"></param>
		/// <param name="lightGroupResource"></param>
		/// <param name="cameraResource"></param>
		/// <param name="material"></param>
		void Draw(WorldTransform& worldTransform, const Matrix4x4& VPMatrix, ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource, const Material* material = nullptr);

		/// <summary>
		/// 生成したモデルの複数描画
		/// </summary>
		/// <param name="worldTransforms"></param>
		/// <param name="textureHandle"></param>
		/// <param name="VPMatrix"></param>
		/// <param name="material"></param>
		void Draw(const uint32_t& numInstance,WorldTransforms& worldTransforms, const uint32_t& textureHandle, const Matrix4x4& VPMatrix, const Material* material = nullptr);

		/// <summary>
		/// 生成したモデルの複数描画
		/// </summary>
		/// <param name="worldTransforms"></param>
		/// <param name="textureHandle"></param>
		/// <param name="VPMatrix"></param>
		/// <param name="material"></param>
		void Draw(const uint32_t& numInstance, WorldTransforms& worldTransforms, const Matrix4x4& VPMatrix, const Material* material = nullptr);

		/// <summary>
		/// グリッドを描画
		/// </summary>
		void DrawGrid(WorldTransform& worldTransform, const Matrix4x4& VPMatrix, ID3D12Resource* cameraResource);

		/// <summary>
		/// モデルに光源を適応させる
		/// </summary>
		/// <param name="directionalLightResource"></param>
		void DrawLight(ID3D12Resource* lightGroupResource, ID3D12Resource* cameraResource);

	public:

		/// <summary>
		/// デフォルトの色を設定
		/// </summary>
		/// <param name="color"></param>
		void SetDefaultColor(const Vector4& color,const std::string& materialName = "default");

		/// <summary>
		/// 鏡面反射の色を設定
		/// </summary>
		/// <param name="specularColor"></param>
		void SetDefaultSpecularColor(const Vector3& specularColor, const std::string& materialName = "default");

		/// <summary>
		/// 輝度の設定
		/// </summary>
		/// <param name="shininess"></param>
		void SetDefaultShiness(const float& shininess, const std::string& materialName = "default");

		/// <summary>
		/// デフォオルトの光源の有無を設定
		/// </summary>
		/// <param name="isEnableLight"></param>
		void SetDefaultIsEnableLight(const bool& isEnableLight, const std::string& materialName = "default");

		/// <summary>
		/// デフォルトのuvMatrixを設定
		/// </summary>
		/// <param name="uvMatrix"></param>
		void SetDefaultUVMatrix(const Matrix4x4& uvMatrix, const std::string& materialName = "default");

		/// <summary>
		/// デフォルトのuvMatrixを設定
		/// </summary>
		/// <param name="uvTransform"></param>
		/// <param name="index"></param>
		void SetDefaultUVMatrix(const Transform& uvTransform, const std::string& materialName = "default");

		/// <summary>
		/// モデルの名前を取得
		/// </summary>
		/// <returns></returns>
		const std::string GetModelName() const { return modelName_; }

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

		// ファイル名
		static inline const std::string kDirectoryPath_ = "Resources/Models";

		// 複数メッシュに対応
		std::vector<std::unique_ptr<Mesh>> meshes_;

		// 複数マテリアルに対応
		std::unordered_map<std::string, std::unique_ptr<Material>> materials_;

		// Nodeのローカル行列を保持しておく変数
		Matrix4x4 localMatrix_;

		bool isLoad_ = false;

		// モデルの名前
		std::string modelName_;

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

		/// <summary>
		/// Node情報を取得
		/// </summary>
		/// <param name="node"></param>
		/// <returns></returns>
		[[nodiscard]]
		Node ReadNode(aiNode* node);

		/// <summary>
		/// ジョイントを作成する
		/// </summary>
		/// <param name="node"></param>
		/// <param name="parent"></param>
		/// <param name="joints"></param>
		/// <returns></returns>
		[[nodiscard]]
		static int32_t CreateJoint(const Node& node, const std::optional<int32_t>& parent, std::vector<Joint> joints);
	};
}