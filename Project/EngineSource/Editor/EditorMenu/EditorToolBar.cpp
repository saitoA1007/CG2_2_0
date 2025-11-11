#include"EditorToolBar.h"
#include"ImGuiManager.h"

EditorToolBar::EditorToolBar(GameEngine::TextureManager* textureManager) {

	// デバックで使用する画像を登録する
	textureManager->RegisterTexture("debugPlay", "Resources/Textures/DebugImages/debugPlay.png");
	textureManager->RegisterTexture("debugPause", "Resources/Textures/DebugImages/debugPause.png");

	// 画像のsrvHandleを取得する
	playImagesrvHandle_ = textureManager->GetTextureSrvHandlesGPU(textureManager->GetHandleByName("debugPlay"));
	pauseImagesrvHandle_ = textureManager->GetTextureSrvHandlesGPU(textureManager->GetHandleByName("debugPause"));
}

void EditorToolBar::Initialize() {

}

void EditorToolBar::Run() {

	// ショートカットキーでも更新処理を管理
	UpdateShortcutsKey();

	// メインメニュー
	if (ImGui::BeginMainMenuBar()) {
		ImVec2 imageSize = { 12.0f,12.0f };
		// playMode_ の状態に応じて表示するボタンを切り替える
		if (playMode_ == ScenePlayMode::Pause) {
			// 現在「停止」中の場合、「再生」ボタンを表示
			if (ImGui::ImageButton("PlayButton",static_cast<ImTextureID>(playImagesrvHandle_.ptr), imageSize)) {
				playMode_ = ScenePlayMode::Play;
			}
		} else {
			// 現在「再生」中の場合、「停止」ボタンを表示
			if (ImGui::ImageButton("PauseButton", static_cast<ImTextureID>(pauseImagesrvHandle_.ptr), imageSize)) {
				playMode_ = ScenePlayMode::Pause;
			}
		}
		ImGui::EndMainMenuBar();
	}
}

bool EditorToolBar::GetIsActiveUpdate() const {
	switch (playMode_)
	{
	case ScenePlayMode::Play:
		return true;
		break;

	case ScenePlayMode::Pause:
		return false;
		break;
	}
	return true;
}

void EditorToolBar::UpdateShortcutsKey() {
	ImGuiIO& io = ImGui::GetIO();

	// Ctrl + Pで再生
	if (io.KeyCtrl && !io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_P, false)) {
		playMode_ = ScenePlayMode::Play;
	}

	// Ctrl + Shift + Pで停止
	if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_P, false)) {
		playMode_ = ScenePlayMode::Pause;
	}
}