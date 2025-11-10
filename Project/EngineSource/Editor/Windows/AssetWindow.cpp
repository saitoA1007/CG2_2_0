#include"AssetWindow.h"
#include"ImGuiManager.h"

void AssetWindow::Draw() {
    ImGui::Begin("Asset", &isActive);
    ImGui::Text("None");
    ImGui::End();
}