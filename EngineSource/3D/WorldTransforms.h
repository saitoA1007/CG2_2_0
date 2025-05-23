#pragma once
#pragma once
#include"EngineSource/Math/Matrix4x4.h"
#include"EngineSource/Math/Transform.h"
#include"EngineSource/Math/TransformationMatrix.h"
#include <d3d12.h>
#include <wrl.h>

#include"externals/DirectXTex/d3dx12.h"

#include"EngineSource/Core/DirectXCommon.h"

namespace GameEngine {

	/// <summary>
	/// 複数描画用のワールド行列
	/// </summary>
	class WorldTransforms {
	public:

		struct TransformData {
			Matrix4x4 worldMatrix;
			Transform transform;
			Vector4 color;
		};

	public:
		WorldTransforms() = default;
		~WorldTransforms() = default;

		/// <summary>
		/// 静的初期化
		/// </summary>
		/// <param name="device"></param>
		static void StaticInitialize(DirectXCommon* dxCommon);

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="transform"></param>
		void Initialize(const uint32_t& kNumInstance);

		/// <summary>
		/// SRTを適応
		/// </summary>
		void UpdateTransformMatrix(const uint32_t& numInstance);

		const CD3DX12_GPU_DESCRIPTOR_HANDLE* GetInstancingSrvGPU() const { return &instancingSrvHandleGPU_; }

		/// <summary>
		/// 描画するモデルの数
		/// </summary>
		/// <returns></returns>
		const uint32_t GetNumInstance() { return numInstance; }
	public:

		std::vector<TransformData> transformDatas_;

		/// <summary>
		/// WVP行列を作成
		/// </summary>
		/// <param name="VPMatrix"></param>
		void SetWVPMatrix(const uint32_t& numInstance,const Matrix4x4& VPMatrix);

	private:
		// コピー禁止
		WorldTransforms(const WorldTransforms&) = delete;
		WorldTransforms& operator=(const WorldTransforms&) = delete;

		static DirectXCommon* dxCommon_;
		// srvに書き込む要素
		static uint32_t StaticSrvIndex_;

		// transformData配列数
		uint32_t numInstance = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
		ParticleForGPU* instancingData_ = nullptr;

		// シェーダリソースビューのハンドル(CPU)
		CD3DX12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_;
		// シェーダリソースビューのハンドル(CPU)
		CD3DX12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_;
	};
}

