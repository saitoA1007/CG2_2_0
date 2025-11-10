#include"EditorWindowManager.h"
#include"ImGuiManager.h"

void EditorWindowManager::RegisterWindow(std::unique_ptr<IEditorWindow> window) {
	windows_.push_back(std::move(window));
}

void EditorWindowManager::DrawAllWindows() {
    for (auto& window : windows_) {
        if (window->isActive)
            window->Draw();
    }
}