#include"SceneMenuBar.h"
#include"ImGuiManager.h"

SceneMenuBar::SceneMenuBar(SceneChangeRequest* request) {
	sceneChangeRequest_ = request;
}

void SceneMenuBar::Run() {

    // 現在のシーンの状態を取得する
    sceneState_ = sceneChangeRequest_->GetCurrentSceneState();

    // メインメニュー
    if (ImGui::BeginMainMenuBar()) {

        if (ImGui::BeginMenu("File")) {

            if (ImGui::BeginMenu("SceneControll")) {
                // 現在のシーン表示
                ImGui::Text("CurrentScene: %s", sceneNames_[static_cast<int>(sceneState_)]);
                ImGui::Separator();

                // 移動したいシーンに移動する
                int currentIndex = static_cast<int>(sceneState_);
                if (ImGui::Combo("SelectScene", &currentIndex, sceneNames_, IM_ARRAYSIZE(sceneNames_))) {
                    SceneState newState = static_cast<SceneState>(currentIndex);
                    if (sceneChangeRequest_) {
                        sceneChangeRequest_->RequestChange(newState);
                    }
                }
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}