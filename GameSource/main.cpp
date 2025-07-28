#include"EngineSource/Core/GameEngine.h"
#include<iostream>

#include"GameScene.h"

using namespace GameEngine;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) {

	// エンジン部分の初期化処理
	std::unique_ptr<Engine> engine = std::make_unique<Engine>();
	engine->Initialize(L"CG2_LE2A_05_サイトウ_アオイ", 1280, 720, hInstance);

	//=================================================================
	// 宣言と初期化
	//=================================================================

	// ゲームシーンのインスタンスを生成
	std::unique_ptr<GameScene> gameScene = std::make_unique<GameScene>();
	gameScene->Initialize(engine->textureManager_.get(), engine->dxCommon_.get(),engine->audioManager_.get());

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

		// 更新前処理
		engine->PreUpdate();

		// ゲームシーンの更新処理
		gameScene->Update(engine->input_.get());

		// 取り敢えず仮置き
		ImGui::Begin("Bloom");
		ImGui::SliderInt("iteration", &iteration, 1, 5);
		engine->bloomPSO_->constBuffer_->bloomIteration = static_cast<uint32_t>(iteration);
		ImGui::SliderFloat("HighLumMask", &highLumMask, 0.0f,1.0f);
		engine->bloomPSO_->constBuffer_->highLumMask = highLumMask;
		ImGui::SliderFloat("Intensity", &intensity, 0.0f, 10.0f);
		engine->bloomPSO_->constBuffer_->intensity = intensity;
		ImGui::End();

		// 更新後処理
		engine->PostUpdate();

		//====================================================================
		// 描画処理
		//====================================================================

		// 描画前処理
		engine->PreDraw();
		
		// ゲームシーンの描画処理
		gameScene->Draw();

		// 描画後処理
		engine->PostDraw();
	}

	/// 解放処理
	engine->Finalize();
	return 0;
}