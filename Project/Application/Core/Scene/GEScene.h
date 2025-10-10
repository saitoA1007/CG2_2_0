#pragma once
#include"BaseScene.h"

// エンジン機能をインクルード
#include"Camera.h"
#include"DebugCamera.h"
#include"Model.h"
#include"WorldTransform.h"

#include"Application/Player.h"
#include"Application/CameraController.h"

class GEScene : public BaseScene {
public:

	~GEScene();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="input"></param>
	void Initialize(GameEngine::Input* input, GameEngine::InputCommand* inputCommand, GameEngine::TextureManager* textureManager, GameEngine::AudioManager* audioManager, GameEngine::DirectXCommon* dxCommon) override;

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

private: // エンジンの低レイヤー機能を取得

	// 入力処理の取得
	GameEngine::Input* input_ = nullptr;

	// テクスチャ機能の取得
	GameEngine::TextureManager* textureManager_;

	// 音声機能を取得
	GameEngine::AudioManager* audioManager_ = nullptr;

	// DirectXの機能を取得
	GameEngine::DirectXCommon* dxCommon_;

	// 入力処理のコマンドシステム
	GameEngine::InputCommand* inputCommand_ = nullptr;

private: // シーン機能

	// 終了フラグ
	bool isFinished_ = false;

	// カメラ
	Transform cameraTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.19f,-9.55f,-10.89f} };
	std::unique_ptr<GameEngine::Camera> camera_;
	// デバックカメラ
	std::unique_ptr<GameEngine::DebugCamera> debugCamera_;
	// デバックカメラの有効の判定
	bool isDebugCameraActive_ = false;

	// グリッドを描画するためのモデル
	std::unique_ptr<GameEngine::Model> gridModel_;
	GameEngine::WorldTransform gridWorldTransform_;

	// 自キャラのモデル
	std::unique_ptr<GameEngine::Model> playerModel_;
	// プレイヤー
	std::unique_ptr<Player> player_;

	// 地面モデル
	std::unique_ptr<GameEngine::Model> terrainModel_;
	GameEngine::WorldTransform terrainWorldTransform_;

	// カメラコントローラークラス
	std::unique_ptr<CameraController> cameraController_;

private:

	/// <summary>
	/// 入力のコマンドを設定する
	/// </summary>
	void InputRegisterCommand();
};