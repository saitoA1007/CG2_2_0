#pragma once
#include"TextureManager.h"

enum class ScenePlayMode {
	Play, // 再生
	Pause // 停止
};

class EditorToolBar {
public:

	EditorToolBar(GameEngine::TextureManager* textureManager);

	void Initialize();

	void Run();

	// 状態取得
	ScenePlayMode GetPlayMode() const { return playMode_; }

	// 更新しているかを取得
	bool GetIsActiveUpdate() const;

private:
	D3D12_GPU_DESCRIPTOR_HANDLE playImagesrvHandle_;
	D3D12_GPU_DESCRIPTOR_HANDLE pauseImagesrvHandle_;
	

	ScenePlayMode playMode_ = ScenePlayMode::Pause;

private:

	/// <summary>
	/// ショートカットキーでも行えるようにする
	/// </summary>
	void UpdateShortcutsKey();
};