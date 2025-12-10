#include"SceneManager.h"

// 各シーン
#include"Application/Scene/TitleScene.h"
#include"Application/Scene/TDGameScene.h"
#include"Application/Scene/ResultScene.h"

// シーン遷移
#include"Application/Scene/Transition/Fade.h"

#include"ImguiManager.h"
#include"ModelRenderer.h"

using namespace GameEngine;

SceneManager::~SceneManager() {
	currentScene_.reset();
	currentScene_.release();
}

void SceneManager::Initialize(SceneContext* context) {

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

	// デバックカメラを生成
	debugCamera_ = std::make_unique<DebugCamera>();
	debugCamera_->Initialize({ 0.0f,2.0f,-20.0f }, 1280, 720, context_->graphicsDevice->GetDevice());
	context_->debugCamera_ = debugCamera_.get();
	// グリッドの初期化
	gridModel_ = context_->modelManager->GetNameByModel("Grid");
	gridWorldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });

	//　シーン遷移システムを初期化
	sceneTransition_ = std::make_unique<SceneTransition>();
	sceneTransition_->Initialize();

	// 使用するカメラのフラグ
#ifdef _DEBUG
	isDebugView_ = true;
#else
	isDebugView_ = false;
#endif
	
	// シーンの初期化。
	ChangeScene(SceneState::TDGame);
}

void SceneManager::Update() {

	// 入力処理のコマンドシステムを更新処理
	context_->inputCommand->Update();

	// シーン遷移の更新処理
	sceneTransition_->Update();

	// シーンの切り替え
	if (sceneTransition_->IsMidTransition() && isChangeScene_) {
		isChangeScene_ = false;
		ChangeScene(currentScene_->NextSceneState());
	}

	// シーン遷移の開始
	if (currentScene_->IsFinished() && !isChangeScene_ && !sceneTransition_->IsActive()) {
		if (isChangeScene_ || sceneTransition_->IsActive()) {
			return;
		}
		isChangeScene_ = true;
		// トランジション開始
		sceneTransition_->Start(currentScene_->GetTransitionEffect());
	}

	// 遷移演出中でなければシーンを更新
	if (!sceneTransition_->IsActive()) {
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
		//ModelRenderer::PreDraw(RenderMode3D::Grid);
		// グリッドを描画
		//ModelRenderer::DrawGrid(gridModel_, gridWorldTransform_, context_->debugCamera_->GetVPMatrix(), context_->debugCamera_->GetCameraResource());
	}
#endif

	// 現在シーンの描画処理
	currentScene_->Draw(isDebugView_);
}

void SceneManager::DrawUI() {

	// UIの描画
	currentScene_->DrawUI();

	// シーン遷移演出を描画
	sceneTransition_->Draw();
}

void SceneManager::LoadModelData() {

	// グリッドモデルをロードと登録
	context_->modelManager->RegisterMode("Grid", Model::CreateGridPlane({ 200.0f,200.0f }));

	// モデルリソースを全てロードする
	context_->modelManager->LoadAllModel();

	// モデルの接線を取得する
	GameEngine::Model* planeModel = context_->modelManager->GetNameByModel("PlaneXZ");
	planeModel->normalMapData_ = Model::CreateNormalMapData("planeXZ.obj","PlaneXZ");

	GameEngine::Model* exteriorPlaneModel = context_->modelManager->GetNameByModel("ExteriorPlane");
	exteriorPlaneModel->normalMapData_ = Model::CreateNormalMapData("exteriorPlane.obj", "ExteriorPlane");
}

void SceneManager::LoadSpriteData() {

	// white2x2の画像を登録する
	//context_->textureManager->RegisterTexture("Resources/Textures/white2x2.png");
	//// uvCheckerの画像を登録する
	//context_->textureManager->RegisterTexture("Resources/Textures/uvChecker.png");
	//// 草原の画像を登録する
	//context_->textureManager->RegisterTexture("Resources/Models/Terrain/grass.png");

	// テクスチャのリソースを全てロードする
	context_->textureManager->LoadAllTexture();
}

void SceneManager::LoadAnimationData() {
	// 歩くアニメーションデータを登録する
	//context_->animationManager->RegisterAnimation("Walk", "walk.gltf");

	// ボスのアニメーションを読み込む
	context_->animationManager->RegisterAnimations("BossBird.IceBreath", "BossBird.IceBreath.gltf", "Resources/Animations");
	context_->animationManager->RegisterAnimations("BossBirdBaseMove", "BossBirdBaseMove.gltf", "Resources/Animations");
	context_->animationManager->RegisterAnimations("BossBirdRush", "BossBirdRush.gltf", "Resources/Animations");
	context_->animationManager->RegisterAnimations("BossBirdScream", "BossBird.Screamgltf.gltf", "Resources/Animations");
	context_->animationManager->RegisterAnimations("BossBird_Appearance_Animation", "BossBird.Screamgltf.gltf", "Resources/Animations");
	context_->animationManager->RegisterAnimations("ShootDown_Animation", "ShootDown_Animation.gltf", "Resources/Animations");

	// プレイヤーのアニメーションを読み込む
	context_->animationManager->RegisterAnimations("PlayerRush", "PlayerRush.gltf", "Resources/Animations");
	context_->animationManager->RegisterAnimations("PlayerWalk", "PlayerWalk.gltf", "Resources/Animations");
	context_->animationManager->RegisterAnimations("PlayerDownAttack", "DownAttack.gltf", "Resources/Animations");
	context_->animationManager->RegisterAnimations("PlayerAirMove", "PlayerAirMove.gltf", "Resources/Animations");
}

void SceneManager::LoadAudioData() {
	//context_->audioManager->LoadAllAudio();
	GameEngine::AudioManager::GetInstance().LoadAllAudio();
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
		break;

	case SceneState::TDGame:
		return std::make_unique<TDGameScene>();
		break;

	case SceneState::Result:
		return std::make_unique<ResultScene>();
		break;

	default:
		return nullptr;
		break;
	}
}