#include "InspectorWindow.h"

using namespace GameEngine;

TextureManager* InspectorWindow::textureManager_ = nullptr;

InspectorWindow::InspectorWindow(TextureManager* textureManager) {
    textureManager_ = textureManager;
}

void InspectorWindow::Draw() {
    if (!isActive) return;

    if (!ImGui::Begin("ParameterInspector", &isActive)) {
        ImGui::End();
        return;
    }

    const std::string& selectedGroupName = GameParamEditor::GetInstance()->GetSelectGroup();

    // グループが選択されていない場合の表示
    if (selectedGroupName.empty()) {
        ImGui::TextDisabled("No group selected");
        ImGui::TextWrapped("No select");
        ImGui::End();
        return;
    }

    // 選択されたグループが存在するかチェック
    auto& allGroups = GameParamEditor::GetInstance()->GetAllGroups();
    auto itGroup = allGroups.find(selectedGroupName);
    if (itGroup == allGroups.end()) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Group not found");
        GameParamEditor::GetInstance()->SelectGroup("");
        ImGui::End();
        return;
    }

    // グループ名をヘッダーに表示
    ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Group: %s", selectedGroupName.c_str());
    ImGui::Separator();

    // 保存ボタン
    if (ImGui::Button("Save")) {
        GameParamEditor::GetInstance()->SaveFile(selectedGroupName);
        std::string message = std::format("{}.json saved.", selectedGroupName);
        MessageBoxA(nullptr, message.c_str(), "GameParamEditor", 0);
    }

    ImGui::SameLine();

    // 読み込みボタン
    if (ImGui::Button("Load")) {
        GameParamEditor::GetInstance()->LoadFile(selectedGroupName);
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    // グループの参照を取得
    GameParamEditor::Group& group = itGroup->second;

    // パラメータが無い場合
    if (group.items.empty()) {
        ImGui::TextDisabled("No parameters in this group");
        ImGui::TextWrapped("None parameter");
        ImGui::End();
        return;
    }

    // 優先順位でソートする
    std::vector<std::pair<std::string, GameParamEditor::Item*>> sortedItems;
    for (auto& [itemName, item] : group.items) {
        sortedItems.push_back({ itemName, &item });
    }
    // 優先順位でソート。小さい順で並べる
    std::sort(sortedItems.begin(), sortedItems.end(),
        [](const auto& a, const auto& b) {
            if (a.second->priority != b.second->priority) {
                return a.second->priority < b.second->priority;
            }
            return a.first < b.first;
        }
    );

    // ソート済みの順序で表示
    for (auto& [itemName, itemPtr] : sortedItems) {
        ImGui::PushID(itemName.c_str());

        // 型に応じて編集UI表示
        std::visit(DebugParameterVisitor{ itemName }, itemPtr->value);

        ImGui::PopID();
    }

    ImGui::End();
}

void InspectorWindow::EditTexutre(std::map<std::string, uint32_t>& value) {
   
    // リストの表示と削除
    ImGui::Text("Current Textures (%d)", (int)value.size());
    ImGui::Separator();

    // イテレータを使ってループ
    for (auto it = value.begin(); it != value.end(); ) {
        const std::string& key = it->first;
        uint32_t handle = it->second;

        ImGui::PushID(key.c_str());

        // データの表示
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s : %u", key.c_str(), handle);

        ImGui::SameLine();

        // 削除ボタン
        if (ImGui::Button("Delete")) {
            it = value.erase(it);
        } else {
            ++it;
        }

        ImGui::PopID();
    }

    ImGui::Spacing();
    ImGui::Separator();

    // 新規追加
    ImGui::Text("Add New Texture");

    // 入力フィールド
    ImGui::InputText("Texture Name", nameBuffer, sizeof(nameBuffer));

    // 追加ボタン
    if (ImGui::Button("Add")) {
        std::string inputName = nameBuffer;

        // マネージャーからハンドルを取得
        uint32_t handle = textureManager_->GetHandleByName(inputName);

        // 有効なハンドルならmapに追加
        value[inputName] = handle;

        // バッファのクリア
        memset(nameBuffer, 0, sizeof(nameBuffer));
    }
    ImGui::Separator();
}