#include"GameEngine.h"
#include<iostream>

#include"FPSCounter.h"
#include"RandomGenerator.h"
#include"GameParamEditor.h"

#include"SceneManager.h"

using namespace GameEngine;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {

	// エンジン部分の初期化処理
	std::unique_ptr<Engine> engine = std::make_unique<Engine>();
	engine->Initialize(L"CG2_LE2A_05_サイトウ_アオイ", 1280, 720, hInstance);

	// fpsを計測する
	std::unique_ptr<FpsCounter> fpsCounter = std::make_unique<FpsCounter>();
	fpsCounter->Initialize();

	// ランダム生成器を初期化
	RandomGenerator::Initialize();

	//=================================================================
	// 宣言と初期化
	//=================================================================

	// 全てのデバック用ファイルを読み込み
	GameParamEditor::GetInstance()->LoadFiles();

	// シーンの初期化
	std::unique_ptr<SceneManager> sceneManager = std::make_unique<SceneManager>();
	sceneManager->Initialize(engine->input_.get(), engine->textureManager_.get(), engine->audioManager_.get(), engine->dxc_.get(), engine->dxCommon_.get());

	int iteration = 1;
	float highLumMask = 0.8f;
	float intensity = 0.0f;

	//=========================================================================
	// メインループ
	//=========================================================================

	// ウィンドウのxボタンが押されるまでループ
	while (true) {
		if (engine->IsWindowOpen()) {
			break;
		}

		//==================================================================
		// 更新処理
		//==================================================================

		// fpsを計測する
		fpsCounter->Update();

		// 更新前処理
		engine->PreUpdate();

		// シーンの更新処理
		sceneManager->Update();

		// 取り敢えず仮置き
		ImGui::Begin("Bloom");
		ImGui::SliderInt("iteration", &iteration, 1, 5);
		engine->bloomPSO_->constBuffer_->bloomIteration = static_cast<uint32_t>(iteration);
		ImGui::SliderFloat("HighLumMask", &highLumMask, 0.0f,1.0f);
		engine->bloomPSO_->constBuffer_->highLumMask = highLumMask;
		ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f);
		engine->bloomPSO_->constBuffer_->intensity = intensity;
		ImGui::End();

#ifdef _DEBUG

		// パラメーターの更新処理
		GameParamEditor::GetInstance()->Update();

		// Fps計測器の描画
		fpsCounter->DrawImGui();
#endif

		// 更新後処理
		engine->PostUpdate();

		//====================================================================
		// 描画処理
		//====================================================================

		// 描画前処理
		engine->PreDraw();
		
		// シーンの描画処理
		sceneManager->Draw();

		// 描画後処理
		engine->PostDraw();
	}

	/// 解放処理
	engine->Finalize();
	return 0;
}