#pragma once
// Core
#include"WindowsApp.h"
#include"DirectXCommon.h"
#include"TextureManager.h"
#include"SrvManager.h"

// Core/PSO
#include"LinePSO.h"
#include"TrianglePSO.h"
#include"ParticlePSO.h"
#include"AnimationPSO.h"
#include"SpritePSO.h"
#include"GridPSO.h"
#include"PostProcess/CopyPSO.h"
#include"PostProcess/BloomPSO.h"
#include"PostProcess/ScanLinePSO.h"
#include"PostProcess/VignettingPSO.h"
#include"PostProcess/RadialBlurPSO.h"
#include"PostProcess/OutLinePSO.h"

#include"PostProcess/PostEffectManager.h"

#include"LogManager.h"
#include"ResourceLeakChecker.h"
#include"CrashHandle.h"

// 3D
#include"Camera.h"
#include"DebugCamera.h"
#include"LightManager.h"
#include"Model.h"
#include"WorldTransform.h"
#include"WorldTransforms.h"
#include"AxisIndicator.h"
#include"PrimitiveRenderer.h"
#include"LineMesh.h"
#include"Animation.h"
#include"ModelRenderer.h"

// 2D
#include"ImGuiManager.h"
#include"Sprite.h"

#include"TransformationMatrix.h"

// Input
#include"InPut.h"

// Audio
#include"AudioManager.h"

namespace GameEngine {

	class Engine final {
	public:

		/// <summary>
		/// エンジンの初期化
		/// </summary>
		/// <param name="title">タイトルバーの名前</param>
		/// <param name="width">画面の横幅</param>
		/// <param name="height">画面の縦幅</param>
		void Initialize(const std::wstring& title, const uint32_t& width, const uint32_t& height, HINSTANCE hInstance);

		/// <summary>
		/// 更新前処理
		/// </summary>
		void PreUpdate();

		/// <summary>
		/// 更新後処理
		/// </summary>
		void PostUpdate();

		/// <summary>
		/// 描画前処理
		/// </summary>
		void PreDraw();

		/// <summary>
		/// 描画後処理
		/// </summary>
		void PostDraw();

		/// <summary>
		/// エンジンの終了処理
		/// </summary>
		void Finalize();

		/// <summary>
		/// ウィンドウが開いているかを判断する
		/// </summary>
		/// <returns></returns>
		bool IsWindowOpen();

	public:

		/// 2D ===========================================

		// ImGuiの機能
		std::unique_ptr<ImGuiManager> imGuiManager_;


		/// Audio ========================================

		// 音声機能
		std::unique_ptr<AudioManager> audioManager_;

		/// Common =======================================

		// ログ
		std::unique_ptr<LogManager> logManager_;

		/// Core ========================================

		// Windowsのアプリ機能
		std::unique_ptr<WindowsApp> windowsApp_;

		// DirectXのアプリ機能
		std::unique_ptr<DirectXCommon> dxCommon_;

		// dxcCompilerの機能
		std::unique_ptr<DXC> dxc_;

		// テクスチャの機能
		std::shared_ptr<TextureManager> textureManager_;

		// srvメモリを管理する機能
		std::unique_ptr<SrvManager> srvManager_;

		/// PSO ======================================

		// 通常のPSO設定
		std::unique_ptr<TrianglePSO> trianglePSO_;

		// パーティクル(複数描画)のPSO設定
		std::unique_ptr<ParticlePSO> particlePSO_;

		// アニメーション用のPSO設定
		std::unique_ptr<AnimationPSO> animationPSO_;

		// 線のPSO設定
		std::unique_ptr<LinePSO> linePSO_;

		// デバック用グリッドのPSO設定
		std::unique_ptr<GridPSO> gridPSO_;

		// スプライトのPSO設定
		std::unique_ptr<SpritePSO> spritePSO_;

		/// ↓ポストエフェクト用のPSO設定

		// ポストエフェクトの内容をSwapChainに持ってくるためのPSO設定
		std::unique_ptr<CopyPSO> copyPSO_;

		// ブルーム用のPSO設定
		std::unique_ptr<BloomPSO> bloomPSO_;

		// ScanLine用のPSO設定
		std::unique_ptr<ScanLinePSO> scanLinePSO_;

		// ヴィネット用のPSO設定
		std::unique_ptr<VignettingPSO> vignettingPSO_;

		// ラジアルブルー用のPSO設定
		std::unique_ptr<RadialBlurPSO> radialBlurPSO_;

		// アウトライン用のPSO設定
		std::unique_ptr<OutLinePSO> outLinePSO_;

		/// Input =============================

		// 入力処理
		std::unique_ptr<Input> input_;

	private:

		/// <summary>
		/// PSOを作成
		/// </summary>
		void CreatePSO();
	};
}