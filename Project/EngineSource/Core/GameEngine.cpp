#include"GameEngine.h"

#pragma comment(lib,"d3d12.lib")
#pragma comment(lib,"dxgi.lib")
#pragma comment(lib,"dxguid.lib")

using namespace GameEngine;

void Engine::RunEngine(HINSTANCE& hInstance) {

	// 初期化
	Initialize(L"LE2A_07_サイトウ_アオイ", 1280, 720, hInstance);

	// 更新処理
	Update();

	// 終了処理
	Finalize();
}

void Engine::Initialize(const std::wstring& title, const uint32_t& width, const uint32_t& height, HINSTANCE hInstance) {

	// 誰も補足しなかった場合に(Unhandled)、補足する関数を登録
	SetUnhandledExceptionFilter(ExportDump);

	// ログの初期化
	LogManager::GetInstance().Create();

	// ウィンドウの作成
	windowsApp_ = std::make_unique<WindowsApp>();
	windowsApp_->CreateGameWindow(title, width, height);

	// リソースチェックのデバック
	D3DResourceLeakChecker leakCheck;

	// srvManagerを生成
	srvManager_ = std::make_unique<SrvManager>();

	// DirectXの機能を生成
	graphicsDevice_ = std::make_unique<GraphicsDevice>();
	graphicsDevice_->Initialize(windowsApp_->GetHwnd(), windowsApp_->kWindowWidth, windowsApp_->kWindowHeight, srvManager_.get());

	// 描画の流れを管理するクラスを初期化
	renderPipeline_ = std::make_unique<RenderPipeline>();
	renderPipeline_->Initialize(windowsApp_->kWindowWidth, windowsApp_->kWindowHeight, srvManager_.get(), graphicsDevice_.get());

	// dxcCompilerの初期化
	dxc_ = std::make_unique<DXC>();
	dxc_->Initialize();

	// PSOを作成
	CreatePSO();

	// PSOを作成
	psoManager_ = std::make_unique<PSOManager>();
	psoManager_->Initialize(graphicsDevice_->GetDevice(), dxc_.get());
	psoManager_->DefaultLoadPSO();

	// ImGuiの初期化
	imGuiManager_ = std::make_unique<ImGuiManager>();
	imGuiManager_->Initialize(graphicsDevice_->GetDevice(), graphicsDevice_->GetCommandList(), graphicsDevice_->GetSwapChainDesc(),
		windowsApp_.get(), srvManager_.get());

	// 入力処理を初期化
	input_ = std::make_unique<Input>();
	input_->Initialize(hInstance, windowsApp_->GetHwnd());

	// 音声の初期化
	GameEngine::AudioManager::GetInstance().Initialize();

	// テクスチャの初期化
	textureManager_ = std::make_shared<TextureManager>();
	textureManager_->Initialize(graphicsDevice_->GetDevice(), graphicsDevice_->GetCommandList(), srvManager_.get());
	// 初期の画像をロードする
	textureManager_->Load("Resources/Textures/white2x2.png");
	// エラーカラーをロードする
	textureManager_->Load("Resources/Textures/errorColor.png");

	// 入力処理のコマンドシステムを生成
	inputCommand_ = std::make_unique<InputCommand>(input_.get());
	// モデルを管理するクラスを生成
	modelManager_ = std::make_unique<ModelManager>();
	// アニメーションデータを管理するクラスを生成する
	animationManager_ = std::make_unique<AnimationManager>();

	// ポストエフェクトの初期化
	PostEffectManager::StaticInitialize(bloomPSO_.get(), scanLinePSO_.get(), vignettingPSO_.get(), radialBlurPSO_.get(), outLinePSO_.get());

	// 画像の初期化
	Sprite::StaticInitialize(graphicsDevice_->GetDevice(), windowsApp_->kWindowWidth, windowsApp_->kWindowHeight);
	SpriteRenderer::StaticInitialize(graphicsDevice_->GetCommandList(), textureManager_.get(), psoManager_.get());
	// 3dを描画する処理の初期化
	Model::StaticInitialize(graphicsDevice_->GetDevice(), textureManager_.get(), srvManager_.get());
	ModelRenderer::StaticInitialize(graphicsDevice_->GetCommandList(), textureManager_.get(), srvManager_.get(), psoManager_.get());
	// ワールドトランスフォームの初期化
	WorldTransform::StaticInitialize(graphicsDevice_->GetDevice());
	WorldTransforms::StaticInitialize(graphicsDevice_->GetDevice(),srvManager_.get());
	// マテリアルの初期化
	Material::StaticInitialize(graphicsDevice_->GetDevice());
	// デバック描画用
	DebugRenderer::StaticInitialize(graphicsDevice_->GetDevice(), graphicsDevice_->GetCommandList(), linePSO_.get());
	// パラメータシステムの初期化
	ParticleBehavior::StatcInitialize(textureManager_.get());

	// 軸方向表示の初期化
	AxisIndicator::StaticInitialize(graphicsDevice_->GetCommandList());

	// カスタム描画機能
	CustomRenderer::StaticInitialize(graphicsDevice_->GetDevice(), graphicsDevice_->GetCommandList(), srvManager_.get(), psoManager_.get());

	// fpsを計測する
	fpsCounter_ = std::make_unique<FpsCounter>();
	fpsCounter_->Initialize();

	// ランダム生成器を初期化
	RandomGenerator::Initialize();

	// 全てのデバック用ファイルを読み込み
	GameParamEditor::GetInstance()->LoadFiles();

	// ゲームシーンで使用するエンジン機能を取得
	sceneContext.input = input_.get();
	sceneContext.inputCommand = inputCommand_.get();
	sceneContext.textureManager = textureManager_.get();
	sceneContext.modelManager = modelManager_.get();
	sceneContext.graphicsDevice = graphicsDevice_.get();
	sceneContext.animationManager = animationManager_.get();

	// シーンの初期化
	sceneManager_ = std::make_unique<SceneManager>();
	sceneManager_->Initialize(&sceneContext);

	// エディターの初期化
#ifdef USE_IMGUI
	// シーン切り替えの通知を管理する機能を初期化
	sceneChangeRequest_ = std::make_unique<SceneChangeRequest>();
	sceneChangeRequest_->SetCurrentSceneState(sceneManager_->GetCurrentSceneState());

	editorCore_ = std::make_unique<EditorCore>();
	editorCore_->Initialize(textureManager_.get(), sceneChangeRequest_.get(), renderPipeline_->GetRendererManager());
#endif
}

void Engine::Update() {
	// ウィンドウのxボタンが押されるまでループ
	while (true) {
		if (IsWindowOpen()) {
			break;
		}

		//==================================================================
		// 更新処理
		//==================================================================

		// 更新前処理
		PreUpdate();

		// シーンの更新処理
		if (isActiveUpdate_ && !isPause_) {
			sceneManager_->Update();
		} else {
			sceneManager_->DebugSceneUpdate();
		}

		// 更新後処理
		PostUpdate();

		//====================================================================
		// 描画処理
		//====================================================================

		// 描画前処理
		PreDraw();

		// シーンの描画処理
		sceneManager_->Draw();

		// 描画後処理
		PostDraw();
	}
}

void Engine::PreUpdate() {

	// fpsを計測する
	fpsCounter_->Update();

	// キー入力の更新処理
	input_->Update();

	// ImGuiにフレームが始まる旨を伝える
	imGuiManager_->BeginFrame();

#ifdef USE_IMGUI

	// エディターの処理
	editorCore_->Run();
	// 更新処理の実行状態を取得する
	isActiveUpdate_ = editorCore_->IsActiveUpdate();

	// 一時停止状態を取得する
	isPause_ = editorCore_->IsPause();

	// 更新処理を管理する
	if (isActiveUpdate_) {
		if (isReset) {
			isReset = false;
		}
	} else {
		// 更新処理が停止してる時、リセットされていなければリセット
		if (!isReset) {
			sceneManager_->ResetCurrentScene();
			isReset = true;
		}
	}

	// シーン切り替えリクエストを処理
	if (sceneChangeRequest_->HasChangeRequest()) {
		// シーンを切り替える
		sceneManager_->ChangeScene(sceneChangeRequest_->GetRequestScene());
		sceneChangeRequest_->ClearChangeRequest();
		// 変更したシーンの状態を取得
		sceneChangeRequest_->SetCurrentSceneState(sceneManager_->GetCurrentSceneState());
	}

	// シーンのデバックに必要な処理を更新する
	sceneManager_->DebugUpdate();
#endif
}

void Engine::PostUpdate() {
	// ImGuiの受付終了
	imGuiManager_->EndFrame();
}

void Engine::PreDraw() {
	// 描画前処理
	renderPipeline_->BeginFrame();
}

void Engine::PostDraw() {
	// 描画後処理
	renderPipeline_->EndFrame(imGuiManager_.get());
}

void Engine::Finalize() {

	// エディターの終了処理
#ifdef USE_IMGUI
	editorCore_->Finalize();
#endif
	
	// 音声の終了処理
	GameEngine::AudioManager::GetInstance().Finalize();

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

	// 線のPSO設定の初期化
	linePSO_ = std::make_unique<LinePSO>();
	linePSO_->Initialize(L"Resources/Shaders/Primitive.VS.hlsl", L"Resources/Shaders/Primitive.PS.hlsl", graphicsDevice_->GetDevice(), dxc_.get());

	// CopyPSOの初期化
	copyPSO_ = std::make_unique<CopyPSO>();
	copyPSO_->Initialize(graphicsDevice_->GetDevice(), L"Resources/Shaders/PostEffect/Copy.VS.hlsl", L"Resources/Shaders/PostEffect/Copy.PS.hlsl", dxc_.get());
	renderPipeline_->SetCopyPSO(copyPSO_.get());

	/// PostProcessのPSOを初期化

	// BloomPSOの初期化
	bloomPSO_ = std::make_unique<BloomPSO>();
	bloomPSO_->Initialize(graphicsDevice_->GetDevice(), L"Resources/Shaders/PostEffect/Bloom.VS.hlsl", dxc_.get(),
		L"Resources/Shaders/PostEffect/HighLumMask.PS.hlsl",
		L"Resources/Shaders/PostEffect/Bloom.PS.hlsl",
		L"Resources/Shaders/PostEffect/BloomResult.PS.hlsl",
		L"Resources/Shaders/PostEffect/BloomComposite.hlsl");

	// scanLinePSOの初期化
	scanLinePSO_ = std::make_unique<ScanLinePSO>();
	scanLinePSO_->Initialize(graphicsDevice_->GetDevice(), dxc_.get());

	// ヴィネットPSOの初期化
	vignettingPSO_ = std::make_unique<VignettingPSO>();
	vignettingPSO_->Initialize(graphicsDevice_->GetDevice(), dxc_.get());

	// ラジアルブルーPSOの初期化
	radialBlurPSO_ = std::make_unique<RadialBlurPSO>();
	radialBlurPSO_->Initialize(graphicsDevice_->GetDevice(), dxc_.get());

	// アウトラインPSOの初期化
	outLinePSO_ = std::make_unique<OutLinePSO>();
	outLinePSO_->Initialize(graphicsDevice_->GetDevice(), dxc_.get());
}