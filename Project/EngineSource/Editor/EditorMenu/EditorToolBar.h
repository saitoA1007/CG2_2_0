#pragma once

enum class ScenePlayMode {
	Play, // 再生
	Pause // 停止
};

class EditorToolBar {
public:

	void Initialize();

	void Run();

	// 状態取得
	ScenePlayMode GetPlayMode() const { return playMode_; }

private:

	ScenePlayMode playMode_ = ScenePlayMode::Pause;
};