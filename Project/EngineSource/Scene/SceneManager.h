#pragma once
#include"SceneContext.h"
#include"BaseScene.h"
#include"Camera.h"

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
	/// <param name="context"></param>
	void Initialize(SceneContext* context);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// デバック時の更新処理
	/// </summary>
	void DebugUpdate();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// シーンの切り替え処理
	/// </summary>
	/// <param name="sceneState">切り替え先の状態</param>
	void ChangeScene(SceneState nextSceneState);

	/// <summary>
	/// 現在のシーンをリセットする
	/// </summary>
	void ResetCurrentScene();

	/// <summary>
	/// 現在のシーンの状態を取得する
	/// </summary>
	/// <returns></returns>
	SceneState GetCurrentSceneState() const { return currentSceneState_; }

private: // エンジン機能

	// エンジン機能群
	SceneContext* context_ = nullptr;

private: // シーン機能

	// 現在のシーン
	std::unique_ptr<BaseScene> currentScene_;

	// シーンの切り替え処理をしているか判断する
	bool isChangeScene_ = false;

	// 現在のシーン状態の保存
	SceneState currentSceneState_ = SceneState::Unknown;

	// デバックカメラ
	std::unique_ptr<GameEngine::DebugCamera> debugCamera_;

	// グリッドを描画するためのモデル
	GameEngine::Model* gridModel_;
	GameEngine::WorldTransform gridWorldTransform_;

	// デバックカメラとの切り替えを管理
	bool isDebugView_ = true;

private:

	/// <summary>
	/// 使用するモデルデータを読み込む
	/// </summary>
	void LoadModelData();

	/// <summary>
	/// 使用する画像データを読み込む
	/// </summary>
	void LoadSpriteData();

	/// <summary>
	/// シーンを作成する
	/// </summary>
	/// <param name="sceneState"></param>
	/// <returns></returns>
	std::unique_ptr<BaseScene> CreateScene(SceneState sceneState);
};