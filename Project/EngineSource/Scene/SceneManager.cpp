#include"SceneManager.h"

// 各シーン
#include"Application/Core/Scene/TitleScene.h"
#include"Application/Core/Scene/GameScene.h"
#include"Application/Core/Scene/GEScene.h"

#include"ImguiManager.h"

using namespace GameEngine;

SceneManager::~SceneManager() {
	currentScene_.reset();
	currentScene_.release();
}

void SceneManager::Initialize(GameEngine::Input* input, GameEngine::TextureManager* textureManager, GameEngine::AudioManager* audioManager, GameEngine::DXC* dxc, GameEngine::DirectXCommon* dxCommon) {

	// 入力処理を取得
	input_ = input;
	// 画像処理を取得
	textureManager_ = textureManager;
	// 音声処理を取得
	audioManager_ = audioManager;
	// shader機能
	dxc_ = dxc;
	// DirectXの機能を取得
	dxCommon_ = dxCommon;

	// 入力処理のコマンドシステムを生成
	inputCommand_ = std::make_unique<InputCommand>(input_);

	// モデルを管理するクラスを生成
	modelManager_ = std::make_unique<ModelManager>();

	// white2x2の画像をロード
	whiteGH_ = textureManager_->Load("Resources/Textures/white2x2.png");

	// モデルを読み込む
	LoadModelData();
	
	// シーンの初期化。最初はタイトルシーンに設定
	ChangeScene(SceneState::GE);
}

void SceneManager::Update() {

	// 入力処理のコマンドシステムを更新処理
	inputCommand_->Update();

	// 終了したらシーンの切り替え処理を有効
	if (currentScene_->IsFinished() && !isChangeScene_) {
		isChangeScene_ = true;
	}

	// シーンが終了した時、切り替える
	if (isChangeScene_) {

		// シーンを切り替える
		ChangeScene(currentScene_->NextSceneState());
		currentScene_->Update();
		isChangeScene_ = false;
	} else {

		// 現在シーンの更新処理
		currentScene_->Update();
	}

#ifdef _DEBUG

	// シーンに対してのデバック処理
	DebugChangeScene();
#endif
}

void SceneManager::Draw() {

	// 現在シーンの描画処理
	currentScene_->Draw();
}

void SceneManager::ChangeScene(SceneState nextSceneState) {

	// シーンの状態を保存
	currentSceneState_ = nextSceneState;

	switch (nextSceneState) {

	case SceneState::Unknown:
	case SceneState::Title:

		// 前の要素を削除
		currentScene_.reset();

		// タイトルシーンを挿入
		currentScene_ = std::make_unique<TitleScene>();
		currentScene_->Initialize(input_, inputCommand_.get(), modelManager_.get(), textureManager_, audioManager_, dxCommon_);
		break;

	case SceneState::Game: {

		// 前の要素を削除
		currentScene_.reset();

		// ゲームシーンを挿入
		std::unique_ptr<GameScene> gameScene = std::make_unique<GameScene>();
		gameScene->Initialize(input_, inputCommand_.get(), modelManager_.get(), textureManager_, audioManager_, dxCommon_);
		currentScene_ = std::move(gameScene);
		break;
	}

	case SceneState::GE:
		// 前の要素を削除
		currentScene_.reset();

		// GEシーンを挿入
		currentScene_ = std::make_unique<GEScene>();
		currentScene_->Initialize(input_, inputCommand_.get(), modelManager_.get(), textureManager_, audioManager_, dxCommon_);
		break;
	}
}

void SceneManager::DebugChangeScene() {

	// シーンのデバック
	ImGui::Begin("SceneState");

	int currentIndex = static_cast<int>(currentSceneState_);
	// シーンを切り替える
	if (ImGui::Combo("currentSceneState", &currentIndex, sceneNames, IM_ARRAYSIZE(sceneNames))) {
		currentSceneState_ = static_cast<SceneState>(currentIndex);
		ChangeScene(currentSceneState_);
	}

	ImGui::End();
}

void SceneManager::LoadModelData() {

	// グリッドモデルをロードと登録
	modelManager_->RegisterMode("Grid", Model::CreateGridPlane({ 200.0f,200.0f }));

	// 平面モデルをロードと登録
	modelManager_->RegisterMode("plane.obj", "Plane");
	// 箱モデルをロードと登録
	modelManager_->RegisterMode("cube.obj", "Cube");

	// 地面モデルをロードと登録
	modelManager_->RegisterMode("terrain.obj", "Terrain");
	// 歩く人型モデルをロードと登録
	modelManager_->RegisterMode("walk.gltf", "Walk");
}