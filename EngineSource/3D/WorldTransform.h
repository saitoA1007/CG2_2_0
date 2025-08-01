#pragma once
#include"EngineSource/Math/Matrix4x4.h"
#include"EngineSource/Math/Transform.h"
#include"EngineSource/Math/TransformationMatrix.h"
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

		// トラスフォームリソースのゲッター
		const Microsoft::WRL::ComPtr<ID3D12Resource>& GetTransformResource() const { return transformationMatrixResource_; }

	public:

		void SetScale(const Vector3& scale) { transform_.scale = scale; }

		void SetRotateY(const float& rotateY) { transform_.rotate.y = rotateY; }
		void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
		Vector3 GetRotate() { return transform_.rotate; }

		void SetTranslateY(const float& translateY) { transform_.translate.y = translateY; }
		void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
		Vector3 GetTranslate() const { return transform_.translate; }

		void SetTransform(const Transform transform) { transform_ = transform; }

		void SetWorldMatrix(const Matrix4x4 worldMatrix) { worldMatrix_ = worldMatrix; }
		Matrix4x4 GetWorldMatrix() const { return worldMatrix_; }

		/// <summary>
		/// WVP行列を作成
		/// </summary>
		/// <param name="VPMatrix"></param>
		void SetWVPMatrix(const Matrix4x4& localMatrix, const Matrix4x4& VPMatrix);

		void SetWVPMatrix(const Matrix4x4& VPMatrix);

		Transform transform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };

	private:
		// コピー禁止
		WorldTransform(const WorldTransform&) = delete;
		WorldTransform& operator=(const WorldTransform&) = delete;

		static ID3D12Device* device_;

		Matrix4x4 worldMatrix_;

		Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
		TransformationMatrix* transformationMatrixData_ = nullptr;
	};
}