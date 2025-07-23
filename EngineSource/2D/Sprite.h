#pragma once
#include <d3d12.h>
#include<iostream>
#include <wrl.h>
#include"EngineSource/Math/Matrix4x4.h"
#include"EngineSource/Math/Vector2.h"
#include"EngineSource/Math/Vector4.h"
#include"EngineSource/Math/Transform.h"

#include"EngineSource/Core/PSO/SpritePSO.h"

namespace GameEngine {

	// 前方宣言
	class TextureManager;

	class Sprite final {
	public:

		// 頂点データ
		struct VertexPosUv {
			Vector4 position;
			Vector2 texcoord;
		};

		// 定数バッファ
		struct ConstBufferData {
			Vector4 color;
			Matrix4x4 uvTransform;
			Matrix4x4 WVP;
		};

	public:
		/// <summary>
		/// 静的初期化
		/// </summary>
		/// <param name="device">デバイス</param>
		/// <param name="window_width">画面幅</param>
		/// <param name="window_height">画面高さ</param>
		static void StaticInitialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, TextureManager* textureManager, SpritePSO* spritePSO, int32_t width, int32_t height);

		/// <summary>
		/// 描画前処理
		/// </summary>
		/// <param name="blendMode">ブレンドモード = BlendMode::??</param>
		static void PreDraw(BlendMode blendMode);

		/// <summary>
		/// スプライト生成
		/// </summary>
		/// <param name="texNumber">テクスチャハンドル</param>
		/// <param name="position">座標</param>
		/// <param name="color">色</param>
		/// <returns>生成されたスプライト</returns>
		static Sprite* Create(Vector2 position, Vector2 size, Vector4 color = { 1, 1, 1, 1 });

	public:

		/// <summary>
		/// 描画
		/// </summary>
		void Draw(const uint32_t& textureHandle = 1024);

		/// <summary>
		/// 座標の設定
		/// </summary>
		/// <param name="position">座標</param>
		void SetPosition(const Vector2& position);

		/// <summary>
		/// 座標を取得
		/// </summary>
		/// <returns></returns>
		const Vector2& GetPosition() const { return position_; }

		/// <summary>
		/// サイズの設定
		/// </summary>
		/// <param name="size">サイズ</param>
		void SetSize(const Vector2& size);

		/// <summary>
		/// 大きさを取得
		/// </summary>
		/// <returns></returns>
		const Vector2& GetSize() const { return size_; }

		/// <summary>
		/// 色の設定
		/// </summary>
		/// <param name="color">色</param>
		void SetColor(const Vector4& color);

		/// <summary>
		/// 色を取得
		/// </summary>
		/// <returns></returns>
		const Vector4& GetColor() const { return constBufferData_->color; }

		/// <summary>
		/// 透明度の設定
		/// </summary>
		/// <param name="alpha"></param>
		void SetAlpha(const float& alpha) { constBufferData_->color.w = alpha; }

		/// <summary>
		/// uvMatrixの設定
		/// </summary>
		/// <param name="transform"></param>
		void SetUvMatrix(const Transform& transform);

	private:

		// デバイス
		static ID3D12Device* device_;
		// コマンドリスト
		static ID3D12GraphicsCommandList* commandList_;
		// 射影行列
		static Matrix4x4 orthoMatrix_;
		// テクスチャ
		static TextureManager* textureManager_;
		// スプライトのPSO
		static SpritePSO* spritePSO_;

		// 座標
		Vector2 position_{};
		// スプライト幅、高さ
		Vector2 size_ = { 100.0f, 100.0f };
		// ワールド行列
		Matrix4x4 worldMatrix_;

		// 頂点バッファビューを作成する
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
		// Sprite用の頂点リソース
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
		VertexPosUv* vertexData_ = nullptr;

		// 頂点バッファビューを作成する
		D3D12_INDEX_BUFFER_VIEW indexBufferView_{};
		// Sprite用の頂点インデックスのリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;

		// Sprite用のマテリアルリソース
		Microsoft::WRL::ComPtr<ID3D12Resource> constBufferResource_;
		// マテリアルにデータを書き込む
		ConstBufferData* constBufferData_ = nullptr;

	private:

		/// <summary>
		/// メッシュを作成
		/// </summary>
		void CreateMesh();

		/// <summary>
		/// マテリアルを作成
		/// </summary>
		/// <param name="color"></param>
		void CreateConstBufferData(const Vector4& color);
	};
}
