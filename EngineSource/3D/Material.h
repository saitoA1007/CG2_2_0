#pragma once
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Vector3.h"
#include"EngineSource/Math/Matrix4x4.h"
#include<iostream>
#include <d3d12.h>
#include <wrl.h>

namespace GameEngine {

	class Material final {
	public:
		struct alignas(16) MaterialData {
			Vector4 color;
			int32_t enableLighting;
			float padding[3];
			Matrix4x4 uvTransform;
			Vector3 specularColor;
			float shininess;
		};

	public:

		/// <summary>
		/// 静的初期化
		/// </summary>
		/// <param name="device"></param>
		static void StaticInitialize(ID3D12Device* device);

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="color">物体の色</param>
		/// <param name="specularColor">specularの色</param>
		/// <param name="shininess">輝度</param>
		/// <param name="isEnableLighting">ライトを有効化</param>
		void Initialize(const Vector4& color, const Vector3& specularColor, const float& shininess, const bool& isEnableLighting);

		// マテリアルリソースのゲッター
		const Microsoft::WRL::ComPtr<ID3D12Resource>& GetMaterialResource() const { return materialResource_; }

	public:

		/// <summary>
		/// 色を設定
		/// </summary>
		/// <param name="color"></param>
		void SetColor(Vector4 color) { materialData_->color = color; }

		/// <summary>
		/// specularの色を設定
		/// </summary>
		/// <param name="specularColor"></param>
		void SetSpecularColor(Vector3 specularColor) { materialData_->specularColor = specularColor; }

		/// <summary>
		/// 透明度を設定
		/// </summary>
		/// <param name="alpha"></param>
		void SetAplha(const float& alpha) { materialData_->color.w = alpha; }

		/// <summary>
		/// 輝度を設定
		/// </summary>
		/// <param name="shininess"></param>
		void SetShiness(const float& shininess) { materialData_->shininess = shininess; }

		/// <summary>
		/// ライトを適応させるかを設定
		/// </summary>
		/// <param name="isEnableLighting"></param>
		void SetEnableLighting(bool isEnableLighting) { materialData_->enableLighting = isEnableLighting; }

		/// <summary>
		/// UV行列を設定
		/// </summary>
		/// <param name="uvMatrix"></param>
		void SetUVMatrix(Matrix4x4 uvMatrix) { materialData_->uvTransform = uvMatrix; }

	private:

		static ID3D12Device* device_;

		Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
		// マテリアルにデータを書き込む
		MaterialData* materialData_ = nullptr;
	};
}