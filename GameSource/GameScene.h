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

#include"Rope.h"

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

	// 地面モデル
	GameEngine::Model* terrainModel_;
	uint32_t grassGH_ = 0u;
	GameEngine::WorldTransform terrainWorldTransform_;

	// ロープ
	std::unique_ptr<Rope> rope_;
	// ロープの点モデル
	GameEngine::Model* sphereModel_;
	uint32_t whiteGH_ = 0u;

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

	// ブレンドモード
	const char* blendModeName_[6] = { "kBlendModeNone","kBlendModeNormal","kBlendModeAdd","kBlendModeSubtract","kBlendModeMultily","kBlendModeScreen"};
	int selectBlendNum_ = 0;
	GameEngine::BlendMode blendMode_ = GameEngine::BlendMode::kBlendModeNormal;
};