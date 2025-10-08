#pragma once
#include"BaseScene.h"

// エンジン機能をインクルード
#include"Camera.h"
#include"DebugCamera.h"
#include"Model.h"
#include"WorldTransform.h"
#include"LightManager.h"
#include"DirectionalLight.h"

class GameScene : public BaseScene {
public:

	~GameScene();

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
	SceneState NextSceneState() override { return SceneState::Title; }

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
	std::unique_ptr<GameEngine::Camera> camera_;
	// デバックカメラ
	std::unique_ptr<GameEngine::DebugCamera> debugCamera_;
	// デバックカメラの有効の判定
	bool isDebugCameraActive_ = false;

	Transform cameraTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };

	// グリッドを描画するためのモデル
	std::unique_ptr<GameEngine::Model> gridModel_;
	GameEngine::WorldTransform gridWorldTransform_;


	// ライト
	std::unique_ptr<GameEngine::LightManager> lightManager_;
	// 平行光源
	GameEngine::DirectionalLight::DirectionalLightData directionalData_;
	// 点光源
	//GameEngine::PointLight::PointLightData pointLightData_;
	// スポットライト
	//GameEngine::SpotLight::SpotLightData spotLightData_;

	// 地面モデル
	std::unique_ptr<GameEngine::Model> terrainModel_;
	uint32_t grassGH_ = 0u;
	GameEngine::WorldTransform terrainWorldTransform_;

	// 平面モデル
	std::unique_ptr<GameEngine::Model> planeModel_;
	uint32_t uvCheckerGH_ = 0u;
	GameEngine::WorldTransform planeWorldTransform_;

	// デバック機能をテストする用の変数
	float testNumber = 0;
	Vector3 testVector{};

	// アニメーションモデル
	std::unique_ptr<GameEngine::Model> boxAnimationModel_;
	GameEngine::WorldTransform boxAnimationWorldTransform_;
	AnimationData boxAnimation_;

	// アニメーションモデル
	std::unique_ptr<GameEngine::Model> bronAnimationModel_;
	GameEngine::WorldTransform bronAnimationWorldTransform_;
	AnimationData bronAnimation_;
	Skeleton skeletonBron_;
	SkinCluster skinClusterBron_;
	float timer_ = 0.0f;

private:

	/// <summary>
	/// デバックした値を登録
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// デバックした値を取得
	/// </summary>
	void ApplyDebugParam();
};