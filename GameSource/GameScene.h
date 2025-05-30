#pragma once
#include"EngineSource/Input/InPut.h"
#include"EngineSource/Core/TextureManager.h"
#include"EngineSource/3D/Camera/Camera.h"
#include"EngineSource/3D/Camera/DebugCamera.h"
#include"EngineSource/3D/AxisIndicator.h"
#include"EngineSource/3D/Model.h"
#include"EngineSource/3D/WorldTransform.h"
#include"EngineSource/3D/WorldTransforms.h"

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

	bool isEnablePostEffect_ = false;

	// ブレンドモード
	const char* blendModeName_[6] = { "kBlendModeNone","kBlendModeNormal","kBlendModeAdd","kBlendModeSubtract","kBlendModeMultily","kBlendModeScreen"};
	int selectBlendNum_ = 0;
	GameEngine::BlendMode blendMode_ = GameEngine::BlendMode::kBlendModeNormal;
};