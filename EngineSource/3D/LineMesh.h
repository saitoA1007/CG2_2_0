#pragma once
#include <d3d12.h>
#include <wrl.h>
#include"EngineSource/Math/Vector3.h"
#include"EngineSource/Math/Vector4.h"

// 頂点データ
struct VertexPosColor {
	Vector4 pos;   // xyz座標
	Vector4 color; // RGBA
};

namespace GameEngine {

	class LineMesh {
	public:

		/// <summary>
		/// 静的初期化
		/// </summary>
		/// <param name="device"></param>
		static void StaticInitialize(ID3D12Device* device);

		/// <summary>
		/// 初期化
		/// </summary>
		/// <param name="p1">始点</param>
		/// <param name="p2">終点</param>
		/// <param name="color">色</param>
		void Initialize(const Vector3& p1, const Vector3& p2, const Vector4& color);

	public:

		const D3D12_VERTEX_BUFFER_VIEW* GetVertexBuffer() const { return &vertexBufferView_; }
		
		/// <summary>
		/// 座標を設定
		/// </summary>
		/// <param name="p1"></param>
		/// <param name="p2"></param>
		void SetPosition(const Vector3& p1, const Vector3& p2);

		/// <summary>
		/// 色を設定
		/// </summary>
		/// <param name="color"></param>
		void SetColor(const Vector4& color);

	private:

		static ID3D12Device* device_;

		VertexPosColor* vertexPosColor_ = nullptr;
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	};
}

