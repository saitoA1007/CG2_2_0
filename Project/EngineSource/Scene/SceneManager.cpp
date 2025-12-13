#include"SceneManager.h"

#include"ImguiManager.h"
#include"ModelRenderer.h"

using namespace GameEngine;

SceneManager::~SceneManager() {
	currentScene_.reset();
	currentScene_.release();
}

void SceneManager::Initialize(SceneContext* context, SceneRegistry* sceneRegistry) {

	// エンジン機能を取得する
	context_ = context;

	// モデルを読み込む
	LoadModelData();

	// 画像を読み込む
	LoadSpriteData();

	// アニメーションデータを読み込む
	LoadAnimationData();

	// 音声データを読み込む
	LoadAudioData();

	// シーンの生成機能を初期化
	sceneRegistry_ = sceneRegistry;

	// デバックカメラを生成
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize({ 0.0f,2.0f,-20.0f }, 1280, 720, context_->graphicsDevice->GetDevice());
	context_->debugCamera_ = debugCamera_.get();
	// グリッドの初期化
	gridModel_ = context_->modelManager->GetNameByModel("Grid");
	gridWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	// 使用するカメラのフラグ
#ifdef _DEBUG
	isDebugView_ = true;
#else
	isDebugView_ = false;
#endif
	
	// デフォルトシーンで初期化
	ChangeScene(sceneRegistry_->GetDefaultScene());
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
		ChangeScene(currentScene_->NextSceneName());
		isChangeScene_ = false;
	} else {

		// 現在シーンの更新処理
		currentScene_->Update();
	}
}

void SceneManager::DebugUpdate() {

	// デバック状態を切り替える
	if (context_->input->TriggerKey(DIK_F)) {
		if (isDebugView_) {
			isDebugView_ = false;
		} else {
			isDebugView_ = true;
		}
	}

	// デバック状態で無ければ早期リターン
	if (!isDebugView_) { return; }

	// デバックカメラを操作
	debugCamera_->Update(context_->input);
	// グリッドの更新処理
	gridWorldTransform_.transform_.translate = Vector3(context_->debugCamera_->GetTargetPosition().x, -0.1f, context_->debugCamera_->GetTargetPosition().z);
	gridWorldTransform_.UpdateTransformMatrix();
}

void SceneManager::DebugSceneUpdate() {
	//　停止中でも適応される処理
	currentScene_->DebugUpdate();
}

void SceneManager::Draw() {
	// デバック用のグリッドを描画
#ifdef _DEBUG
	if (isDebugView_) {
		// モデルの単体描画前処理
		ModelRenderer::PreDraw(RenderMode3D::Grid);
		// グリッドを描画
		ModelRenderer::DrawGrid(gridModel_, gridWorldTransform_, context_->debugCamera_->GetVPMatrix(), context_->debugCamera_->GetCameraResource());
	}
#endif

	// 現在シーンの描画処理
	currentScene_->Draw(isDebugView_);
}

void SceneManager::LoadModelData() {

	// グリッドモデルをロードと登録
	context_->modelManager->RegisterMode("Grid", Model::CreateGridPlane({ 200.0f,200.0f }));

	// モデルリソースを全てロードする
	context_->modelManager->LoadAllModel();
}

void SceneManager::LoadSpriteData() {

	// テクスチャのリソースを全てロードする
	context_->textureManager->LoadAllTexture();
}

void SceneManager::LoadAnimationData() {
	// 歩くアニメーションデータを登録する
	context_->animationManager->RegisterAnimation("Walk", "walk.gltf");
}

void SceneManager::LoadAudioData() {
	context_->audioManager->LoadAllAudio();
}

void SceneManager::ChangeScene(const std::string& sceneName) {

	// シーン名を保存
	currentSceneName_ = sceneName;

	// 前の要素を削除
	currentScene_.reset();

	// 新しいシーンを作成
	currentScene_ = sceneRegistry_->CreateScene(sceneName);

	if (currentScene_) {
		// 新しく作ったシーンを初期化
		currentScene_->Initialize(context_);
		// 1回だけ更新処理を挟む
		currentScene_->Update();
	} else {
		// 新しいシーンのインスタンスを作れなかった場合
		assert(0 && "Scene not found");
	}
}

void SceneManager::ResetCurrentScene() {
	// 現在のシーンを再初期化する
	ChangeScene(currentSceneName_);
}