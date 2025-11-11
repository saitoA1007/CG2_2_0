#pragma once
#include"SceneContext.h"

#include"Camera.h"

#include"BaseScene.h"

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
	/// 描画処理
	/// </summary>
	void Draw();

	/// <summary>
	/// 現在のシーンをリセットする
	/// </summary>
	void ResetCurrentScene();

private: // エンジン機能

	// エンジン機能
	SceneContext* context_ = nullptr;

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
	static inline const char* sceneNames[] = { "Title","Game","GE"};

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

	/// <summary>
	/// 使用するモデルデータを読み込む
	/// </summary>
	void LoadModelData();

	/// <summary>
	/// 使用する画像データを読み込む
	/// </summary>
	void LoadSpriteData();
};