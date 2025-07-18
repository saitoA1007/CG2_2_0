#pragma once
#include"EngineSource/Input/InPut.h"
#include"EngineSource/Core/TextureManager.h"
#include"EngineSource/3D/Camera/Camera.h"
#include"EngineSource/3D/Camera/DebugCamera.h"
#include"EngineSource/3D/AxisIndicator.h"
#include"EngineSource/3D/Model.h"
#include"EngineSource/3D/WorldTransform.h"
#include"EngineSource/3D/WorldTransforms.h"
#include"EngineSource/3D/Light/LightManager.h"
#include"EngineSource/3D/Light/DirectionalLight.h"

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

	// カメラトラスフォーム
	Transform cameraTransform_ = { {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,2.0f,-20.0f} };
	// カメラ
	std::unique_ptr<GameEngine::Camera> camera_;
	// デバックカメラ
	std::unique_ptr<GameEngine::DebugCamera> debugCamera_;
	// デバックカメラの有効の判定
	bool isDebugCameraActive_ = false;

	// white4x4テクスチャ
	uint32_t whiteGH_ = 0u;

	// グリッドを描画するためのモデル
	GameEngine::Model* gridModel_;
	GameEngine::WorldTransform gridWorldTransform_;

	// 地面モデル
	GameEngine::Model* terrainModel_;
	uint32_t grassGH_ = 0u;
	GameEngine::WorldTransform terrainWorldTransform_;

	// 平面モデル
	GameEngine::Model* planeModel_;
	uint32_t planeGH_ = 0u;
	Transform planeTransform_{};
	GameEngine::WorldTransform planeWorldTransform_;

	// ライト
	std::unique_ptr<GameEngine::LightManager> lightManager_;
	// 平行光源
	GameEngine::DirectionalLight::DirectionalLightData directionalData_;
	// 点光源
	GameEngine::PointLight::PointLightData pointLightData_;
	// スポットライト
	GameEngine::SpotLight::SpotLightData spotLightData_;

	// 箱
	Transform boxTransform_;
	GameEngine::Model* boxModel_;
	GameEngine::WorldTransform boxWorldTransform_;

	// ブレンドモード
	const char* blendModeName_[6] = { "kBlendModeNone","kBlendModeNormal","kBlendModeAdd","kBlendModeSubtract","kBlendModeMultily","kBlendModeScreen"};
	int selectBlendNum_ = 0;
	GameEngine::BlendMode blendMode_ = GameEngine::BlendMode::kBlendModeNormal;

	GameEngine::TextureManager* textureManager_;
};