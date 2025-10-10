#pragma once
#include"Input.h"
#include"TextureManager.h"
#include"AudioManager.h"

#include"InputCommand.h"

// 各シーン
enum class SceneState {

	Title,       // タイトルシーン
	Game,        // ゲームシーン
	GE,          // GEシーン

	Unknown,
};

/// <summary>
/// 各シーンの元
/// </summary>
class BaseScene {
public:

	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~BaseScene() = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	virtual void Initialize(GameEngine::Input* input,GameEngine::InputCommand* inputCommand,GameEngine::TextureManager* textureManager, GameEngine::AudioManager* audioManager, GameEngine::DirectXCommon* dxCommon) = 0;

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw() = 0;

	/// <summary>
	/// 終了したことを伝える
	/// </summary>
	/// <returns></returns>
	virtual bool IsFinished() = 0;

	/// <summary>
	/// 次のシーン遷移する場面の値を取得
	/// </summary>
	/// <returns></returns>
	virtual SceneState NextSceneState() = 0;
};