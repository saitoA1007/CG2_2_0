#pragma once
#include"BaseScene.h"

// エンジン機能をインクルード
#include"Camera.h"
#include"DebugCamera.h"
#include"Model.h"
#include"WorldTransform.h"

class TitleScene : public BaseScene {
public:

	~TitleScene();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="input"></param>
	void Initialize(SceneContext* context) override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw() override;

	/// <summary>
	/// 終了したことを伝える
	/// </summary>
	/// <returns></returns>
	bool IsFinished() override { return isFinished_; };

	/// <summary>
	/// 次のシーン遷移する場面の値を取得
	/// </summary>
	/// <returns></returns>
	SceneState NextSceneState() override { return SceneState::Game; }

private: // シーン機能

	// 終了フラグ
	bool isFinished_ = false;

	// カメラ
	std::unique_ptr<GameEngine::Camera> camera_;
	// デバックカメラ
	std::unique_ptr<GameEngine::DebugCamera> debugCamera_;
	// デバックカメラの有効の判定
	bool isDebugCameraActive_ = false;

	Transform cameraTransform_ = { {1.0f,1.0f,1.0f},{-0.720f,0.0f,0.0f},{0.19f,-9.55f,-10.89f} };

	// グリッドを描画するためのモデル
	GameEngine::Model* gridModel_;
	GameEngine::WorldTransform gridWorldTransform_;
};