#pragma once
#include"EngineSource/Core/WindowsApp.h"
#include"EngineSource/Core/DirectXCommon.h"
#include"EngineSource/Core/TextureManager.h"

#include"EngineSource/Core/PSO/LinePSO.h"
#include"EngineSource/Core/PSO/TrianglePSO.h"
#include"EngineSource/Core/PSO/ParticlePSO.h"
#include"EngineSource/Core/PSO/PostProcessPSO.h"
#include"EngineSource/Core/PSO/BloomPSO.h"
#include"EngineSource/Core/PSO/CopyPSO.h"
#include"EngineSource/Core/PSO/SpritePSO.h"
#include"EngineSource/Core/PSO/GridPSO.h"

#include"EngineSource/Core/PostProcess/PostEffectManager.h"

#include"EngineSource/Common/LogManager.h"
#include"EngineSource/Common/ResourceLeakChecker.h"
#include"EngineSource/Common/CrashHandle.h"

#include"EngineSource/3D/Camera/Camera.h"
#include"EngineSource/3D/Camera/DebugCamera.h"
#include"EngineSource/3D/Light/LightManager.h"
#include"EngineSource/3D/Model.h"
#include"EngineSource/3D/WorldTransform.h"
#include"EngineSource/3D/WorldTransforms.h"
#include"EngineSource/3D/AxisIndicator.h"
#include"EngineSource/3D/PrimitiveRenderer.h"
#include"EngineSource/3D/LineMesh.h"

#include"EngineSource/2D/ImGuiManager.h"
#include"EngineSource/2D/Sprite.h"

#include"EngineSource/Math/TransformationMatrix.h"

#include"EngineSource/Input/InPut.h"

#include"EngineSource/Audio/AudioManager.h"

namespace GameEngine {

	class Engine final {
	public:

		/// <summary>
		/// エンジンの初期化
		/// </summary>
		/// <param name="title"></param>
		/// <param name="width"></param>
		/// <param name="height"></param>
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

		/// PSO ======================================

		// 通常のPSO設定
		std::unique_ptr<TrianglePSO> trianglePSO_;

		// パーティクル(複数描画)のPSO設定
		std::unique_ptr<ParticlePSO> particlePSO_;

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

		/// Input =============================

		std::unique_ptr<Input> input_;

	private:

		/// <summary>
		/// PSOを作成
		/// </summary>
		void CreatePSO();
	};
}