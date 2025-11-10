#include"SceneWIndow.h"
#include"ImGuiManager.h"

void SceneWindow::Draw() {
    ImGui::Begin("SceneView", &isActive);
    ImGui::Text("Scene view goes here");
    ImGui::End();
}