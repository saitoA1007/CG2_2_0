#pragma once
#include"EngineSource/Math/Matrix4x4.h"
#include"EngineSource/Math/Transform.h"
#include"EngineSource/Math/TransformationMatrix.h"
#include"AnimationData.h"
#include <d3d12.h>
#include <wrl.h>

namespace GameEngine {

	/// <summary>
	/// 単体描画用のワールド行列
	/// </summary>
	class WorldTransform final {
	public:
		WorldTransform() = default;
		~WorldTransform() = default;

		/// <summary>
		/// 静的初期化
		/// </summary>
		/// <param name="device"></param>
		static void StaticInitialize(ID3D12Device* device);

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="transform">Scale,Rotate,Translate : 各型Vector3</param>
		void Initialize(const Transform& transform);

		/// <summary>
		/// SRTを適応
		/// </summary>
		void UpdateTransformMatrix();

		/// <summary>
		/// アニメーションがある場合の更新処理
		/// </summary>
		/// <param name="animation">アニメーションデータ</param>
		/// <param name="modelName">モデル名</param>
		void UpdateAnimation(AnimationData& animation,const std::string& modelName);

		// トラスフォームリソースのゲッター
		const Microsoft::WRL::ComPtr<ID3D12Resource>& GetTransformResource() const { return transformationMatrixResource_; }

	public:

		void SetWorldMatrix(const Matrix4x4 worldMatrix) { worldMatrix_ = worldMatrix; }
		Matrix4x4 GetWorldMatrix() const { return worldMatrix_; }

		/// <summary>
		/// WVP行列を作成
		/// </summary>
		/// <param name="VPMatrix"></param>
		void SetWVPMatrix(const Matrix4x4& localMatrix, const Matrix4x4& VPMatrix);

		void SetWVPMatrix(const Matrix4x4& VPMatrix);

		/// <summary>
		/// 親を設定
		/// </summary>
		/// <param name="parent"></param>
		void SetParent(const WorldTransform* parent){ parent_ = const_cast<WorldTransform*>(parent); }

		/// <summary>
		/// 親を取得
		/// </summary>
		/// <returns></returns>
		WorldTransform* GetParent() { return parent_; }

	public:

		// SRT要素
		Transform transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	private:
		// コピー禁止
		WorldTransform(const WorldTransform&) = delete;
		WorldTransform& operator=(const WorldTransform&) = delete;

		static ID3D12Device* device_;

		Matrix4x4 worldMatrix_;

		// リソース
		Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
		TransformationMatrix* transformationMatrixData_ = nullptr;

		// 親
		WorldTransform* parent_ = nullptr;
	};
}