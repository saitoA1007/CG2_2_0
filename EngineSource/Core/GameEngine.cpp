#include"GameEngine.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

using namespace GameEngine;

void Engine::Initialize(const std::wstring& title, const uint32_t& width, const uint32_t& height, HINSTANCE hInstance) {

	// 誰も補足しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	// ログの初期化
	LogManager::Create();
	logManager_ = std::make_unique<LogManager>();

	// ウィンドウの作成
	windowsApp_ = std::make_unique<WindowsApp>();
	windowsApp_->CreateGameWindow(title, width, height);

	// リソースチェックのデバック
	D3DResourceLeakChecker leakCheck;

	// DirectXCommonの初期化
	dxCommon_ = std::make_unique<DirectXCommon>();
	dxCommon_->Initialize(windowsApp_->GetHwnd(), windowsApp_->kWindowWidth, windowsApp_->kWindowHeight, logManager_.get());

	// dxcCompilerの初期化
	dxc_ = std::make_unique<DXC>();
	dxc_->Initialize(logManager_.get());

	// PSOを作成
	CreatePSO();

	// ImGuiの初期化
	imGuiManager_ = std::make_unique<ImGuiManager>();
	imGuiManager_->Initialize(windowsApp_.get(), dxCommon_.get());

	// 入力処理を初期化
	input_ = std::make_unique<Input>();
	input_->Initialize(hInstance, windowsApp_->GetHwnd());

	// 音声の初期化
	audioManager_ = std::make_unique<AudioManager>();
	audioManager_->Initialize();

	// テクスチャの初期化
	textureManager_ = std::make_shared<TextureManager>();
	textureManager_->Initialize(dxCommon_.get(), logManager_.get());

	//=====================================================================================
	// 静的初期化
	//=====================================================================================

	// ポストエフェクトの初期化
	PostEffectManager::StaticInitialize(bloomPSO_.get(), scanLinePSO_.get(), vignettingPSO_.get(), radialBlurPSO_.get(), logManager_.get());

	// 画像の初期化
	Sprite::StaticInitialize(dxCommon_->GetDevice(), dxCommon_->GetCommandList(), textureManager_.get(), spritePSO_.get(), windowsApp_->kWindowWidth, windowsApp_->kWindowHeight);
	// 3dを描画する処理の初期化
	Model::StaticInitialize(dxCommon_->GetDevice(), dxCommon_->GetCommandList(), textureManager_.get(), trianglePSO_.get(), particlePSO_.get(), gridPSO_.get(), logManager_.get());
	// 線を描画する処理の初期化
	PrimitiveRenderer::StaticInitialize(dxCommon_->GetDevice(), dxCommon_->GetCommandList(), linePSO_.get(), logManager_.get());
	// ワールドトランスフォームの初期化
	WorldTransform::StaticInitialize(dxCommon_->GetDevice());
	WorldTransforms::StaticInitialize(dxCommon_.get());
	// マテリアルの初期化
	Material::StaticInitialize(dxCommon_->GetDevice());
	// 線を描画する為のメッシュの初期化
	LineMesh::StaticInitialize(dxCommon_->GetDevice());

	// 軸方向表示の初期化
	AxisIndicator::StaticInitialize(dxCommon_->GetCommandList());
}

void Engine::PreUpdate() {
	// キー入力の更新処理
	input_->Update();
	// ImGuiにフレームが始まる旨を伝える
	imGuiManager_->BeginFrame();
}

void Engine::PostUpdate() {
	// ImGuiの受付終了
	imGuiManager_->EndFrame();
}

void Engine::PreDraw() {
	// 描画前処理
	dxCommon_->PreDraw();
}

void Engine::PostDraw() {
	// 描画後処理
	dxCommon_->PostDraw(imGuiManager_.get());
}

void Engine::Finalize() {
	// テクスチャの解放
	textureManager_->Finalize();
	// ImGuiの解放処理
	imGuiManager_->Finalize();
	// WindowAppの解放
	windowsApp_->BreakGameWindow();
}

bool Engine::IsWindowOpen() {
	return windowsApp_->ProcessMessage();
}

void Engine::CreatePSO() {

	// 三角形のPSO設定の初期化
	trianglePSO_ = std::make_unique<TrianglePSO>();
	trianglePSO_->Initialize(L"Resources/Shaders/Object3d.VS.hlsl", L"Resources/Shaders/Object3d.PS.hlsl", dxCommon_->GetDevice(), dxc_.get(), logManager_.get());

	// パーティクル(複数描画用)のPSO設定を初期化
	particlePSO_ = std::make_unique<ParticlePSO>();
	particlePSO_->Initialize(L"Resources/Shaders/Particle.VS.hlsl", L"Resources/Shaders/particle.PS.hlsl", dxCommon_->GetDevice(), dxc_.get(), logManager_.get());

	// 線のPSO設定の初期化
	linePSO_ = std::make_unique<LinePSO>();
	linePSO_->Initialize(L"Resources/Shaders/Primitive.VS.hlsl", L"Resources/Shaders/Primitive.PS.hlsl", dxCommon_->GetDevice(), dxc_.get(), logManager_.get());

	// グリッド用の初期化
	gridPSO_ = std::make_unique<GridPSO>();
	gridPSO_->Initialize(L"Resources/Shaders/Grid.VS.hlsl", L"Resources/Shaders/Grid.PS.hlsl", dxCommon_->GetDevice(), dxc_.get(), logManager_.get());

	// CopyPSOの初期化
	copyPSO_ = std::make_unique<CopyPSO>();
	copyPSO_->Initialize(dxCommon_->GetDevice(), L"Resources/Shaders/PostEffect/Copy.VS.hlsl", L"Resources/Shaders/PostEffect/Copy.PS.hlsl", dxc_.get(), logManager_.get());
	dxCommon_->SetCopyPSO(copyPSO_.get());

	// スプライトのPSOを初期化
	spritePSO_ = std::make_unique<SpritePSO>();
	spritePSO_->Initialize(L"Resources/Shaders/Sprite.VS.hlsl", L"Resources/Shaders/Sprite.PS.hlsl", dxCommon_->GetDevice(), dxc_.get(), logManager_.get());

	/// PostProcessのPSOを初期化

	// BloomPSOの初期化
	bloomPSO_ = std::make_unique<BloomPSO>();
	bloomPSO_->Initialize(dxCommon_->GetDevice(), L"Resources/Shaders/PostEffect/Bloom.VS.hlsl", dxc_.get(), logManager_.get(),
		L"Resources/Shaders/PostEffect/HighLumMask.PS.hlsl",
		L"Resources/Shaders/PostEffect/Bloom.PS.hlsl",
		L"Resources/Shaders/PostEffect/BloomResult.PS.hlsl",
		L"Resources/Shaders/PostEffect/BloomComposite.hlsl");

	// scanLinePSOの初期化
	scanLinePSO_ = std::make_unique<ScanLinePSO>();
	scanLinePSO_->Initialize(dxCommon_->GetDevice(), dxc_.get(), logManager_.get());

	// ヴィネットPSOの初期化
	vignettingPSO_ = std::make_unique<VignettingPSO>();
	vignettingPSO_->Initialize(dxCommon_->GetDevice(), dxc_.get(), logManager_.get());

	// ラジアルブルーPSOの初期化
	radialBlurPSO_ = std::make_unique<RadialBlurPSO>();
	radialBlurPSO_->Initialize(dxCommon_->GetDevice(), dxc_.get(), logManager_.get());
}