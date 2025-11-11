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

void SceneManager::Initialize(SceneContext* context) {

	// エンジン機能を取得する
	context_ = context;

	// white2x2の画像をロード
	whiteGH_ = context_->textureManager->Load("Resources/Textures/white2x2.png");

	// モデルを読み込む
	LoadModelData();

	// 画像を読み込む
	LoadSpriteData();
	
	// シーンの初期化。最初はタイトルシーンに設定
	ChangeScene(SceneState::GE);
}

void SceneManager::Update() {

	// 入力処理のコマンドシステムを更新処理
	context_->inputCommand->Update();

	// 終了したらシーンの切り替え処理を有効
	if (currentScene_->IsFinished() && !isChangeScene_) {
		isChangeScene_ = true;
	}

	// シーンが終了した時、切り替える
	if (isChangeScene_) {

		// シーンを切り替える
		ChangeScene(currentScene_->NextSceneState());
		isChangeScene_ = false;
	} else {

		// 現在シーンの更新処理
		currentScene_->Update();
	}
}

void SceneManager::Draw() {

	// 現在シーンの描画処理
	currentScene_->Draw();
}

void SceneManager::LoadModelData() {

	// グリッドモデルをロードと登録
	context_->modelManager->RegisterMode("Grid", Model::CreateGridPlane({ 200.0f,200.0f }));

	// 平面モデルをロードと登録
	context_->modelManager->RegisterMode("Plane","plane.obj");
	// 箱モデルをロードと登録
	context_->modelManager->RegisterMode("Cube","cube.obj");

	// 地面モデルをロードと登録
	context_->modelManager->RegisterMode("Terrain","terrain.obj");
	// 歩く人型モデルをロードと登録
	context_->modelManager->RegisterMode("Walk","walk.gltf");
}

void SceneManager::LoadSpriteData() {

	// white2x2の画像を登録する
	context_->textureManager->RegisterTexture("white", "Resources/Textures/white2x2.png");
	// uvCheckerの画像を登録する
	context_->textureManager->RegisterTexture("uvChecker", "Resources/Textures/uvChecker.png");
	// 草原の画像を登録する
	context_->textureManager->RegisterTexture("grass", "Resources/Models/Terrain/grass.png");
}

void SceneManager::ChangeScene(SceneState nextSceneState) {

	// シーンの状態を保存
	currentSceneState_ = nextSceneState;

	// 前の要素を削除
	currentScene_.reset();

	// 新しいシーンを作成
	currentScene_ = CreateScene(nextSceneState);

	if (currentScene_) {
		// 新しく作ったシーンを初期化
		currentScene_->Initialize(context_);
		// 1回だけ更新処理を挟む
		currentScene_->Update();
	} else {
		// 新しいシーンのインスタンスを作れなかった場合
		assert(0);
	}
}

void SceneManager::ResetCurrentScene() {
	// 現在のシーンを再初期化する
	ChangeScene(currentSceneState_);
}

std::unique_ptr<BaseScene> SceneManager::CreateScene(SceneState sceneState) {
	switch (sceneState) {

	case SceneState::Unknown:
	case SceneState::Title:
		return std::make_unique<TitleScene>();

	case SceneState::Game:
		return std::make_unique<GameScene>();

	case SceneState::GE:
		return std::make_unique<GEScene>();

	default:
		return nullptr;
	}
}