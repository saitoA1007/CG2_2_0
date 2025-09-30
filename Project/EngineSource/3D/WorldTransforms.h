#pragma once
#pragma once
#include"Matrix4x4.h"
#include"Transform.h"
#include"TransformationMatrix.h"
#include <d3d12.h>
#include <wrl.h>
#include <queue>
#include <unordered_set>

#include"Externals/DirectXTex/d3dx12.h"

#include"DirectXCommon.h"
#include"ResourceCounter.h"

namespace GameEngine {

	/// <summary>
	/// 複数描画用のワールド行列
	/// </summary>
	class WorldTransforms final {
	public:

		struct TransformData {
			Matrix4x4 worldMatrix;
			Transform transform;
			Vector4 color;
		};

	public:
		WorldTransforms() = default;
		~WorldTransforms();

		/// <summary>
		/// 静的初期化
		/// </summary>
		/// <param name="device"></param>
		static void StaticInitialize(DirectXCommon* dxCommon);

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="transform"></param>
		void Initialize(const uint32_t& kNumInstance, const Transform& transform);

		/// <summary>
		/// SRTを適応
		/// </summary>
		void UpdateTransformMatrix(const uint32_t& numInstance);

		const CD3DX12_GPU_DESCRIPTOR_HANDLE* GetInstancingSrvGPU() const { return &instancingSrvHandleGPU_; }

		/// <summary>
		/// 描画するモデルの数
		/// </summary>
		/// <returns></returns>
		const uint32_t GetNumInstance() { return numInstance_; }
	public:

		// 各要素のトランスフォーム
		std::vector<TransformData> transformDatas_;

		/// <summary>
		/// WVP行列を作成
		/// </summary>
		/// <param name="VPMatrix"></param>
		void SetWVPMatrix(const uint32_t& numInstance,const Matrix4x4& VPMatrix);

		void SetWVPMatrix(const uint32_t& numInstance, const Matrix4x4& localMatrix, const Matrix4x4& VPMatrix);

	private:
		// コピー禁止
		WorldTransforms(const WorldTransforms&) = delete;
		WorldTransforms& operator=(const WorldTransforms&) = delete;

		static DirectXCommon* dxCommon_;

		// srvの管理に使用する変数
		static uint32_t StaticSrvIndex_;
		static std::queue<uint32_t> availableIndices_;  // 利用出来るインデックス
		static std::unordered_set<uint32_t> usedIndices_; // 利用しているインデックスを保持
		static uint32_t nextNewIndex_; // 次のsrvインデックス
		static const uint32_t kMaxSrvIndex_; // 最大のsrvインデックス

		// インスタンスが持つsrvインデックス
		uint32_t srvIndex_ = 0;

		// transformData配列数
		uint32_t numInstance_ = 0;

		// リソース
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource_;
		ParticleForGPU* instancingData_ = nullptr;

		// シェーダリソースビューのハンドル(CPU)
		CD3DX12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU_;
		// シェーダリソースビューのハンドル(CPU)
		CD3DX12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU_;

	private:

		/// <summary>
		/// srvインデックスを取得
		/// </summary>
		/// <returns></returns>
		uint32_t AddSrvIndex();

		/// <summary>
		/// srvインデックスを解放
		/// </summary>
		/// <param name="index"></param>
		void ReleaseSrvIndex(const uint32_t& index);
	};
}

