#include"ConsoleWindow.h"
#include"ImGuiManager.h"

void ConsoleWindow::Draw() {
    ImGui::Begin("Console", &isActive);
    ImGui::Text("None");
    ImGui::End();
}