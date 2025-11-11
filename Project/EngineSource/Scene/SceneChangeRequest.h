#pragma once
#include"SceneState.h"
#include <optional>

class SceneChangeRequest {
public:

	/// <summary>
	/// シーンの切り替えをリクエストする
	/// </summary>
	/// <param name="nextScene"></param>
	void RequestChange(SceneState nextScene) { requestSceneState_ = nextScene; }

	/// <summary>
	/// リクエストがあるかを取得
	/// </summary>
	/// <returns></returns>
	bool HasChangeRequest() const { return requestSceneState_.has_value(); }

	/// <summary>
	/// 変更したいシーンを取得
	/// </summary>
	/// <returns></returns>
	SceneState GetRequestScene() const { return requestSceneState_.value(); }

	/// <summary>
	/// シーンを変更するリクエストをクリア
	/// </summary>
	void ClearChangeRequest() { requestSceneState_.reset(); }

	/// <summary>
	/// 現在のシーンを取得
	/// </summary>
	/// <param name="currentScene"></param>
	void SetCurrentSceneState(SceneState currentScene) { currentSceneState_ = currentScene; }

	SceneState GetCurrentSceneState() const { return currentSceneState_; }

private:
	// リクエストするシーン
	std::optional<SceneState> requestSceneState_;
	// 現在のシーン
	SceneState currentSceneState_;
};