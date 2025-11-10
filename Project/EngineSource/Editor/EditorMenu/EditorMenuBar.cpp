#include"EditorMenuBar.h"
#include"ImGuiManager.h"

void EditorMenuBar::Draw(EditorWindowManager* windowManager) {

	// メインメニュー
	if (ImGui::BeginMainMenuBar()) {

		if (ImGui::BeginMenu("File")) {
			ImGui::Text("None");
			ImGui::EndMenu();
		}

		// ウィンドウを表示するかを管理するメニュー
		if (ImGui::BeginMenu("Windows")) {
			for (auto& window : windowManager->GetWindows()) {
				ImGui::MenuItem(window->GetName().c_str(), nullptr, &window->isActive);
			}
			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}