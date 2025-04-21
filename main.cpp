
#include<cstdint>
#include<d3d12.h>
#include<cassert>
#include<iostream>
#include"Log.h"
#include"Math/Vector4.h"
#include"CreateBufferResource.h"
#include"DescriptorHeap.h"
#include"Math.h"
#include"VertexData.h"
#include"DescriptorHandle.h"
#include"Material.h"
#include"DirectionalLight.h"
#include"CrashHandler.h"
#include"Camera.h"
#include <string>

#include"ResorceLeakChecker.h"
#include"WindowsAPI.h"
#include"DirectXCommon.h"
#include"GraphicsPipeline.h"
#include"Sphere.h"
#include"ImGuiManager.h"
#include"TextureManager.h"
#include"Sprite.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

// windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	
	// 誰も補足しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);

	/// ログを出力するための準備
	// ログディレクトリがなければ作成
	if (!std::filesystem::exists("logs")) {
		std::filesystem::create_directory("logs");
	}
	// 現在時刻を取得(UTC時刻)
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	// ログファイルの名前にコンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	// 日本時間(PCの設定時間)に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(),nowSeconds };
	// formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	// 時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	// ファイルを作って書き込み準備
	std::ofstream logStream(logFilePath);

	// ウィンドウの作成
	std::shared_ptr<WindowsApp> windowsApp = std::make_shared<WindowsApp>();
	windowsApp->CreateGameWindow(L"CG2", 1280, 720);

	// リソースチェックのデバック
	D3DResourceLeakChecker leakCheck;

	// DirectXCommonの初期化
	std::shared_ptr<DirectXCommon> dxCommon = std::make_shared<DirectXCommon>();
	dxCommon->Initialize(windowsApp->GetHwnd(), windowsApp->kWindowWidth, windowsApp->kWindowHeight, logStream);

	// PSO設定の初期化
	GraphicsPipeline* graphicsPipeline = GraphicsPipeline::GetInstance();
	graphicsPipeline->Initialize(dxCommon->GetDevice(), logStream);

	// 画像の初期化
	Sprite::StaticInitialize(dxCommon->GetDevice(), dxCommon->GetCommandList(), windowsApp->kWindowWidth, windowsApp->kWindowHeight);

	// ImGuiの初期化
	std::shared_ptr<ImGuiManager> imGuiManager = std::make_shared<ImGuiManager>();
	imGuiManager->Initialize(windowsApp.get(), dxCommon.get());

	// テクスチャの初期化
	std::shared_ptr<TextureManager> textureManager = std::make_shared<TextureManager>();
	textureManager->Initialize(dxCommon.get());

	//=========================================================================
	// 宣言と初期化
	//=========================================================================

	// 画像をロード
	uint32_t uvCheckerGH = textureManager->Load("Resources/uvChecker.png", logStream);
	uint32_t monsterBallGH = textureManager->Load("Resources/monsterBall.png", logStream);

	// カメラ
	Camera camera;
	camera.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, windowsApp->kWindowWidth, windowsApp->kWindowHeight);

	// 球
	Sphere sphere;
	sphere.Initialize(dxCommon->GetDevice(), 16);
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	sphere.SetTransformationMatrix(camera.MakeWVPMatrix(worldMatrix));

	// 2d用のspriteを作成
	Sprite* sprite = Sprite::Create({ 0.0f,0.0f }, { 640.0f,360.0f },{ 1.0f,1.0f,1.0f,1.0f });
	Vector2 translateSprite = { 0.0f,0.0f};
	Vector2 sizeSprite = { 640.0f,360.0f };
	Vector4 colorSprite = { 1.0f,1.0f,1.0f,1.0f };
	// UVTransform用の変数
	Transform uvTransformSprite = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	// 平行根源
	DirectionalLight directionalLight;
	directionalLight.Initialize(dxCommon->GetDevice());
	Vector4 lightColor = directionalLight.GetLightColor();
	Vector3 lightDir = directionalLight.GetLightDir();
	float intensity = directionalLight.GetLightIntensity();

	// テクスチャを切り替えるフラグ
	bool useMonsterBall = true;

	//===========================================================================
	// メインループ
	//===========================================================================

	// ウィンドウのxボタンが押されるまでループ
	while (true) {
		if (windowsApp->ProcessMessage()) {
			break;
		}

		//=====================================================
		// 更新処理
		//=====================================================

		// ImGuiにフレームが始まる旨を伝える
		imGuiManager->BeginFrame();

		// 画像を動かすimGui
		ImGui::Begin("DebugWindow");
		ImGui::SliderFloat2("transformSprite", &translateSprite.x, 0.0f, 720.0f);
		sprite->SetPosition(translateSprite);
		ImGui::SliderFloat2("transformSize", &sizeSprite.x, 0.0f, 720.0f);
		sprite->SetSize(sizeSprite);
		ImGui::ColorEdit3("transformColor", &colorSprite.x);
		sprite->SetColor(colorSprite);
		ImGui::Checkbox("useMonsterBall", &useMonsterBall);

		// 色を変更
		ImGui::ColorEdit3("Light_Color", &lightColor.x); 
		directionalLight.SetLightColor(lightColor);
		// 方向を変更
		ImGui::SliderFloat3("Light_Direction", &lightDir.x, -1.0f, 1.0f); 
		directionalLight.SetLightDir(lightDir);
		// 強度を変更
		ImGui::SliderFloat("Light_Intensity", &intensity, 0.0f, 10.0f); 
		directionalLight.SetLightIntensity(intensity);

		// 画像のuvを動かすimGui
		ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
		sprite->SetUvMatrix(uvTransformSprite);
		ImGui::End();

		// 球の処理
		transform.rotate.y += 0.03f;
		worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		sphere.SetTransformationMatrix(camera.MakeWVPMatrix(worldMatrix));

		// ImGuiの受付終了
		imGuiManager->EndFrame();

		//====================================================
		// 描画処理
		//====================================================

		// 描画前処理
		dxCommon->PreDraw(graphicsPipeline->GetRootSignature(), graphicsPipeline->GetPipelineState());

		// 球の描画
		sphere.Draw(dxCommon->GetCommandList(), directionalLight.GetResource(), useMonsterBall ? &textureManager->GetTextureSrvHandlesGPU(monsterBallGH) : &textureManager->GetTextureSrvHandlesGPU(uvCheckerGH));

		// Spriteの描画
		sprite->Draw(&textureManager->GetTextureSrvHandlesGPU(uvCheckerGH));

		// ImGuiの描画処理
		imGuiManager->Draw();
		// 描画後処理
		dxCommon->PostDraw();
	}

	// COMの終了処理
	CoUninitialize();

	/// 解放処理

	// スプライトのインスタンスを解放
	delete sprite;

	// 球の解放
	sphere.Release();

	// テクスチャの解放
	textureManager->Finalize();

	// ImGuiの解放処理
	imGuiManager->Finalize();

	// GraphicsPipeline解放
	graphicsPipeline->Finalize();

	// DirectXCommonの解放処理
	dxCommon->Finalize();

	// WindowAppの解放
	windowsApp->BreakGameWindow();
	return 0;
}

