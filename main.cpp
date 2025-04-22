
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
#include"ImGuiManager.h"
#include"TextureManager.h"
#include"Sprite.h"
#include"Model.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

// windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
	
	// 誰も補足しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	// COMの初期化
	CoInitializeEx(0, COINIT_MULTITHREADED);

	// ログの初期化
	LogManager::Create();
	std::shared_ptr<LogManager> logManager = std::make_shared<LogManager>();

	// ウィンドウの作成
	std::shared_ptr<WindowsApp> windowsApp = std::make_shared<WindowsApp>();
	windowsApp->CreateGameWindow(L"CG2", 1280, 720);

	// リソースチェックのデバック
	D3DResourceLeakChecker leakCheck;

	// DirectXCommonの初期化
	std::shared_ptr<DirectXCommon> dxCommon = std::make_shared<DirectXCommon>();
	dxCommon->Initialize(windowsApp->GetHwnd(), windowsApp->kWindowWidth, windowsApp->kWindowHeight, logManager.get());

	// PSO設定の初期化
	GraphicsPipeline* graphicsPipeline = GraphicsPipeline::GetInstance();
	graphicsPipeline->Initialize(dxCommon->GetDevice(), logManager.get());

	// 画像の初期化
	Sprite::StaticInitialize(dxCommon->GetDevice(), dxCommon->GetCommandList(), windowsApp->kWindowWidth, windowsApp->kWindowHeight);

	// 3dを描画する処理の初期化
	Model::StaticInitialize(dxCommon->GetDevice(), dxCommon->GetCommandList());

	// ImGuiの初期化
	std::shared_ptr<ImGuiManager> imGuiManager = std::make_shared<ImGuiManager>();
	imGuiManager->Initialize(windowsApp.get(), dxCommon.get());

	// テクスチャの初期化
	std::shared_ptr<TextureManager> textureManager = std::make_shared<TextureManager>();
	textureManager->Initialize(dxCommon.get(),logManager.get());

	//=========================================================================
	// 宣言と初期化
	//=========================================================================

	// 画像をロード
	uint32_t uvCheckerGH = textureManager->Load("Resources/uvChecker.png");
	uint32_t monsterBallGH = textureManager->Load("Resources/monsterBall.png");

	// カメラ
	Camera camera;
	camera.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, windowsApp->kWindowWidth, windowsApp->kWindowHeight);

	// 球
	//Sphere sphere;
	//sphere.Initialize(dxCommon->GetDevice(), 16);
	Transform transform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} };
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	//sphere.SetTransformationMatrix(camera.MakeWVPMatrix(worldMatrix));

	Model* plane = Model::CreateFromOBJ("axis.obj","Axis/");
	plane->SetTransformationMatrix(camera.MakeWVPMatrix(worldMatrix));

	// 2d用のspriteを作成
	//Sprite* sprite = Sprite::Create({ 0.0f,0.0f }, { 640.0f,360.0f },{ 1.0f,1.0f,1.0f,1.0f });
	//Vector2 translateSprite = { 0.0f,0.0f};
	//Vector2 sizeSprite = { 640.0f,360.0f };
	//Vector4 colorSprite = { 1.0f,1.0f,1.0f,1.0f };
	//// UVTransform用の変数
	//Transform uvTransformSprite = {
	//	{1.0f,1.0f,1.0f},
	//	{0.0f,0.0f,0.0f},
	//	{0.0f,0.0f,0.0f}
	//};

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
		// モデル
		ImGui::DragFloat3("UVTranslate", &transform.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("UVrotate", &transform.rotate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat3("UVScale", &transform.scale.x, 0.01f, -10.0f, 10.0f);
		worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		plane->SetTransformationMatrix(camera.MakeWVPMatrix(worldMatrix));

		// 光の色を変更
		ImGui::ColorEdit3("Light_Color", &lightColor.x); 
		directionalLight.SetLightColor(lightColor);
		// 光の方向を変更
		ImGui::SliderFloat3("Light_Direction", &lightDir.x, -1.0f, 1.0f); 
		directionalLight.SetLightDir(lightDir);
		// 光の強度を変更
		ImGui::SliderFloat("Light_Intensity", &intensity, 0.0f, 10.0f); 
		directionalLight.SetLightIntensity(intensity);

		//ImGui::SliderFloat2("transformSprite", &translateSprite.x, 0.0f, 720.0f);
		//sprite->SetPosition(translateSprite);
		//ImGui::SliderFloat2("transformSize", &sizeSprite.x, 0.0f, 720.0f);
		//sprite->SetSize(sizeSprite);
		//ImGui::ColorEdit3("transformColor", &colorSprite.x);
		//sprite->SetColor(colorSprite);
		//ImGui::Checkbox("useMonsterBall", &useMonsterBall);

		// 画像のuvを動かすimGui
		//ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		//ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		//ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);
		//sprite->SetUvMatrix(uvTransformSprite);
		ImGui::End();

		// ImGuiの受付終了
		imGuiManager->EndFrame();

		//====================================================
		// 描画処理
		//====================================================

		// 描画前処理
		dxCommon->PreDraw(graphicsPipeline->GetRootSignature(), graphicsPipeline->GetPipelineState());

		
		plane->Draw(directionalLight.GetResource(), &textureManager->GetTextureSrvHandlesGPU(uvCheckerGH));

		// Spriteの描画
		//sprite->Draw(&textureManager->GetTextureSrvHandlesGPU(uvCheckerGH));

		// ImGuiの描画処理
		imGuiManager->Draw();
		// 描画後処理
		dxCommon->PostDraw();
	}

	// COMの終了処理
	CoUninitialize();

	/// 解放処理

	// スプライトのインスタンスを解放
	//delete sprite;

	delete plane;

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

