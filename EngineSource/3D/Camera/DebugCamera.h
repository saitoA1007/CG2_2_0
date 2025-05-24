#pragma once
#include"EngineSource/Math/Vector3.h"
#include"EngineSource/Math/Matrix4x4.h"
#include"EngineSource/Input/InPut.h"

#include"EngineSource/Math/TransformationMatrix.h"

#include <d3d12.h>
#include <wrl.h>

namespace GameEngine {

	class DebugCamera {
	public:

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="translate">カメラ座標</param>
		/// <param name="width">画面横幅</param>
		/// <param name="height">画面縦幅</param>
		void Initialize(const Vector3& translate, int width, int height, ID3D12Device* device);

		/// <summary>
		/// 更新処理
		/// </summary>
		void Update(Input* input);

	public:

		Matrix4x4 GetVPMatrix();

		Matrix4x4 GetRotateMatrix();

		Matrix4x4 GetWorldMatrix() const { return worldMatrix_; }

		inline ID3D12Resource* GetCameraResource() const { return cameraResource_.Get(); }

		Vector3 GetWorldPosition();

	private:
		// 拡縮
		Vector3 scale_ = { 1.0f,1.0f,1.0f };
		// X,Y,Z軸回りのローカル座標角
		Vector3 rotate_ = { 0.0f,0.0f,0.0f };
		// ローカル座標
		Vector3 translate_ = { 0.0f,0.0f,-10.0f };
		// ワールド行列
		Matrix4x4 worldMatrix_;
		// ビュー行列
		Matrix4x4 viewMatrix_;
		// 射影行列
		Matrix4x4 projectionMatrix_;

		// 累積回転行列
		Matrix4x4 rotateMatrix_;

		Microsoft::WRL::ComPtr<ID3D12Resource> cameraResource_;
		CameraForGPU* cameraForGPU_ = nullptr;
	};
}