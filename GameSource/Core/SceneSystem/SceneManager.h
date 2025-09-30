#pragma once
#include"Input.h"
#include"TextureManager.h"
#include"AudioManager.h"

#include"Camera.h"

#include"Scene/BaseScene.h"

/// <summary>
/// シーンの管理
/// </summary>
class SceneManager {
public:
	SceneManager() = default;
	~SceneManager();

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="input">入力処理</param>
	/// <param name="textureManager">画像機能</param>
	/// <param name="audioManager">音声機能</param>
	/// <param name="dxc">Shader機能</param>
	/// <param name="dxCommon">DirectX全般の機能</param>
	void Initialize(GameEngine::Input* input, GameEngine::TextureManager* textureManager, GameEngine::AudioManager* audioManager, GameEngine::DXC* dxc, GameEngine::DirectXCommon* dxCommon);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

private: // エンジン機能

	// shader機能
	GameEngine::DXC* dxc_ = nullptr;

	// DirectX機能の取得
	GameEngine::DirectXCommon* dxCommon_ = nullptr;

	// 入力処理の取得
	GameEngine::Input* input_ = nullptr;

	// テクスチャ機能を取得
	GameEngine::TextureManager* textureManager_ = nullptr;

	// 音声機能を取得
	GameEngine::AudioManager* audioManager_ = nullptr;

private: // シーン機能

	// 現在のシーン
	std::unique_ptr<BaseScene> currentScene_;

	// シーンの切り替え処理をしているか判断する
	bool isChangeScene_ = false;

	// white2x2テクスチャの読み込み
	uint32_t whiteGH_ = 0u;

	// 現在のシーン状態の保存
	SceneState currentSceneState_ = SceneState::Unknown;

	// デバック用に各シーンの名前を保存している
	static inline const char* sceneNames[] = { "Title","Game"};

private:

	/// <summary>
	/// シーンの切り替え処理
	/// </summary>
	/// <param name="sceneState">切り替え先の状態</param>
	void ChangeScene(SceneState nextSceneState);

	/// <summary>
	/// デバックする時のシーン切り替え処理
	/// </summary>
	void DebugChangeScene();
};