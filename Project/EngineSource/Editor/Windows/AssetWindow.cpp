#include"AssetWindow.h"
#include"ImGuiManager.h"

void AssetWindow::Draw() {
    ImGui::Begin("Asset", &isActive);
    ImGui::Text("Scene view goes here");
    ImGui::End();
}