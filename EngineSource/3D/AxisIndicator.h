#pragma once
#include"EngineSource/3D/Camera/Camera.h"
#include"WorldTransform.h"
#include"Model.h"


namespace GameEngine {

	class AxisIndicator final {
	public:
		AxisIndicator() = default;
		~AxisIndicator() = default;

		/// <summary>
		/// 静的初期化
		/// </summary>
		/// <param name="commandList"></param>
		static void StaticInitialize(ID3D12GraphicsCommandList* commandList);

		/// <summary>
		/// 初期化
		/// </summary>
		void Initialize();

		/// <summary>
		/// 更新
		/// </summary>
		void Update(const Matrix4x4& rotate);

		/// <summary>
		/// 描画
		/// </summary>
		void Draw();

	private:
		AxisIndicator(const AxisIndicator&) = delete;
		AxisIndicator& operator=(const AxisIndicator&) = delete;

		// コマンドリスト
		static ID3D12GraphicsCommandList* commandList_;

		// モデル
		std::unique_ptr<Model> model_;
		// カメラ
		Camera camera_;
		// ワールドトランスフォーム
		WorldTransform worldTransform_;

		Transform transform_;

		// ビューポート
		D3D12_VIEWPORT viewport_{};
		// シザー矩形
		D3D12_RECT scissorRect_{};
	};
}