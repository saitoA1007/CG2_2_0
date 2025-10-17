#pragma once
#include <d3d12.h>

#include"Vector3.h"
#include"Vector4.h"
#include"Matrix4x4.h"
#include"LinePSO.h"

#include"LineMesh.h"

namespace GameEngine {

	class PrimitiveRenderer final {
	public:
		// カメラ行列を送るデータ
		struct TransformMatrix {
			Matrix4x4 VP;
		};

	public:

		PrimitiveRenderer() = default;
		~PrimitiveRenderer() = default;

		/// <summary>
		/// 静的初期化
		/// </summary>
		/// <param name="device"></param>
		/// <param name="commandList"></param>
		/// <param name="linePSO"></param>
		/// <param name="logManager"></param>
		static void StaticInitialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, LinePSO* linePSO);

		/// <summary>
		/// 描画前処理
		/// </summary>
		static void PreDraw();

		/// <summary>
		/// 線を作成
		/// </summary>
		/// <returns></returns>
		static PrimitiveRenderer* Create();

		/// <summary>
		/// 描画処理
		/// </summary>
		/// <param name="VPMatrix">カメラのビュープロジェクション行列</param>
		void DrawLine3d(const LineMesh* lineMesh, const Matrix4x4& VPMatrix);

	private:
		// コピー禁止
		PrimitiveRenderer(PrimitiveRenderer&) = delete;
		PrimitiveRenderer& operator=(PrimitiveRenderer&) = delete;

		// デバイス
		static ID3D12Device* device_;
		// コマンドリスト
		static ID3D12GraphicsCommandList* commandList_;

		// PSO設定
		static LinePSO* linePSO_;

		uint32_t totalVertices_ = 0;

		Microsoft::WRL::ComPtr<ID3D12Resource> transformMatrixResource_;
		TransformMatrix* transformMatrixData_ = nullptr;
	};
}