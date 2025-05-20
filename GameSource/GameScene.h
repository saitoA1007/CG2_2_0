#pragma once
#include"EngineSource/Input/InPut.h"
#include"EngineSource/Core/TextureManager.h"
#include"EngineSource/3D/Camera.h"
#include"EngineSource/3D/DebugCamera.h"
#include"EngineSource/3D/AxisIndicator.h"
#include"EngineSource/3D/DirectionalLight.h"
#include"EngineSource/3D/Model.h"
#include"EngineSource/3D/WorldTransform.h"

class GameScene {
public:

	~GameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="textureManager"></param>
	void Initialize(GameEngine::TextureManager* textureManager, GameEngine::DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	/// <param name="input"></param>
	void Update(GameEngine::Input * input);

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

private:

	GameEngine::DirectXCommon* dxCommon_;

	// 軸方向モデル用のテクスチャハンドル
	uint32_t axisTextureHandle_ = 0u;
	// 軸方向表示
	std::unique_ptr<GameEngine::AxisIndicator> axisIndicator_;

	// カメラ
	std::unique_ptr<GameEngine::Camera> camera_;
	// デバックカメラ
	std::unique_ptr<GameEngine::DebugCamera> debugCamera_;
	// デバックカメラの有効の判定
	bool isDebugCameraActive_ = false;

	// 平行光源
	std::unique_ptr<GameEngine::DirectionalLight> directionalLight_;
	Vector4 lightColor_;
	Vector3 lightDir_;
	float intensity_;

	// 平面モデル
	GameEngine::Model* planeModel_;
	GameEngine::WorldTransform planeWorldTransform_;
	// uvCheckerのテクスチャ
	uint32_t uvTextureHandle_=0u;
	// 平面色
	Vector4 planeColor_{ 1.0f,1.0f,1.0f,1.0f };
};