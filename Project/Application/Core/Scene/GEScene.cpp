#include"GEScene.h"
#include"ImguiManager.h"
#include"ModelRenderer.h"
#include"SpriteRenderer.h"
#include"GameParamEditor.h"
#include"LogManager.h"
#include<numbers>
using namespace GameEngine;

GEScene::~GEScene() {
}

void GEScene::Initialize(SceneContext* context) {
	// ゲームシーンに必要な低レイヤー機能
#pragma region SceneSystem 
	// エンジン機能を取得
	context_ = context;

	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} }, 1280, 720);
	// デバックカメラの初期化
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize({ 0.0f,2.0f,-20.0f }, 1280, 720, context_->graphicsDevice->GetDevice());

	// グリッドの初期化
	gridModel_ = context_->modelManager->GetNameByModel("Grid");
	gridWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// デバック用描画の初期化
	debugRenderer_ = DebugRenderer::Create();

	// 登録するパラメータを設定
	GameParamEditor::GetInstance()->SetActiveScene("GEScene");

#pragma endregion

	// プレイヤーモデルを生成
	playerModel_ = context_->modelManager->GetNameByModel("cube.obj");
	// プレイヤークラスを初期化
	player_ = std::make_unique<Player>();
	player_->Initialize();

	// カメラをコントロールするクラスを初期化
	cameraController_ = std::make_unique<CameraController>();
	cameraController_->Initialize();

	// 画像
	sprite_ = Sprite::Create({0.0f,0.0f},{256.0f,256.0f},{0.0f,0.0f});
	// uvCheckerの画像を取得
	uvCheckerGH_ = context_->textureManager->GetHandleByName("uvChecker");

	// パーティクルのシステムを初期化
	testParticle_ = std::make_unique<ParticleBehavior>();
	testParticle_->Initialize("testParticle",16, uvCheckerGH_);
	testParticle_->Emit({ 0.0f,0.0f,0.0f });

	// 平面モデル
	planeModel_ = context_->modelManager->GetNameByModel("plane.obj");

	// ログのテスト
	Log("HallWorldConsloe","test");
	Log("HallWorldConsloe");

	// 入力コマンドを設定する
	InputRegisterCommand();
}

void GEScene::Update() {

	// デバックリストを削除
	debugRenderer_->Clear();

	// プレイヤーの更新処理
	player_->Update(context_->inputCommand);

	// パーティクルの更新処理
	testParticle_->Update(camera_->GetWorldMatrix());

	// カメラコントロールの更新処理
	cameraController_->Update(context_->inputCommand,player_->GetPlayerPos());

	// カメラ処理
#pragma region Camera
	if (isDebugCameraActive_) {
		// デバックカメラの更新
		debugCamera_->Update(context_->input);
		// デバックカメラの値をカメラに代入
		camera_->SetVPMatrix(debugCamera_->GetVPMatrix());

		// グリッドの更新処理
		gridWorldTransform_.transform_.translate = Vector3(debugCamera_->GetTargetPosition().x, -0.1f, debugCamera_->GetTargetPosition().z);
		gridWorldTransform_.UpdateTransformMatrix();

	} else {
		// カメラの更新処理
		//camera_->Update();
		camera_->SetterWorldMatrix(cameraController_->GetWorldMatrix());
		camera_->SetVPMatrix(cameraController_->GetVPMatirx());

		// グリッドの更新処理
		gridWorldTransform_.transform_.translate = Vector3(camera_->GetWorldPosition().x, -0.1f, camera_->GetWorldPosition().z);
		gridWorldTransform_.UpdateTransformMatrix();
	}
#pragma endregion

#ifdef _DEBUG

	// カメラの切り替え処理
#pragma region CameraTransition
	if (context_->inputCommand->IsCommandAcitve("CameraChange")) {
		if (isDebugCameraActive_) {
			isDebugCameraActive_ = false;
		} else {
			isDebugCameraActive_ = true;
		}
	}
#pragma endregion
#endif
}

void GEScene::Draw() {

	//===========================================================
	// 3D描画
	//===========================================================09

	// 複数モデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::Instancing);

	// パーティクルを描画
	ModelRenderer::DrawInstancing(planeModel_, testParticle_->GetCurrentNumInstance(), *testParticle_->GetWorldTransforms(), testParticle_->GetTexture(), camera_->GetVPMatrix());

	// 3Dモデルの描画前処理
	ModelRenderer::PreDraw(RenderMode3D::DefaultModel);

	// プレイヤーを描画
	ModelRenderer::Draw(playerModel_, player_->GetWorldTransform(),0, camera_->GetVPMatrix());

#ifdef _DEBUG
	// モデルの単体描画前処理
	ModelRenderer::PreDraw(RenderMode3D::Grid);

	// グリッドを描画
	ModelRenderer::DrawGrid(gridModel_, gridWorldTransform_, camera_->GetVPMatrix(), debugCamera_->GetCameraResource());

	// デバック描画
	debugRenderer_->DrawAll(camera_->GetVPMatrix());
#endif

	//========================================================================
	// 2D描画
	//========================================================================

	// 画像の描画前処理
	SpriteRenderer::PreDraw(RenderMode2D::Normal);

	SpriteRenderer::Draw(sprite_.get(), uvCheckerGH_);
}

void GEScene::InputRegisterCommand() {
	// カメラを操作を切り替える入力コマンドを登録
	context_->inputCommand->RegisterCommand("CameraChange", { {InputState::KeyTrigger, DIK_F },{ InputState::KeyTrigger, DIK_G } });

	// 移動の入力コマンドを登録する
	context_->inputCommand->RegisterCommand("MoveUp", { {InputState::KeyPush, DIK_W },{InputState::PadLeftStick,0,{0.0f,1.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_UP } });
	context_->inputCommand->RegisterCommand("MoveDown", { {InputState::KeyPush, DIK_S },{InputState::PadLeftStick,0,{0.0f,-1.0f},0.2f}, {InputState::PadPush, XINPUT_GAMEPAD_DPAD_DOWN} });
	context_->inputCommand->RegisterCommand("MoveLeft", { {InputState::KeyPush, DIK_A },{InputState::PadLeftStick,0,{-1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_LEFT } });
	context_->inputCommand->RegisterCommand("MoveRight", { {InputState::KeyPush, DIK_D },{InputState::PadLeftStick,0,{1.0f,0.0f},0.2f}, { InputState::PadPush, XINPUT_GAMEPAD_DPAD_RIGHT } });
	// ジャンプコマンドを登録する
	context_->inputCommand->RegisterCommand("Jump", { {InputState::KeyTrigger, DIK_SPACE},{InputState::PadTrigger, XINPUT_GAMEPAD_A} });

	// カメラ操作のコマンドを登録する
	context_->inputCommand->RegisterCommand("CameraMoveLeft", { { InputState::KeyPush, DIK_LEFT },{InputState::PadRightStick,0,{-1.0f,0.0f},0.2f} });
	context_->inputCommand->RegisterCommand("CameraMoveRight", { { InputState::KeyPush, DIK_RIGHT },{InputState::PadRightStick,0,{1.0f,0.0f},0.2f} });
}
