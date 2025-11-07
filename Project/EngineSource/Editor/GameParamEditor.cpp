#include"GameParamEditor.h"
#include<cassert>

using namespace GameEngine;

GameParamEditor* GameParamEditor::GetInstance() {
	static GameParamEditor instance;
	return &instance;
}

void GameParamEditor::CreateGroup(const std::string& groupName, const std::string& sceneName) {
	// 指定名のオブジェクトが無ければ追加する
	datas_[groupName].sceneName = sceneName;
}

void GameParamEditor::Update() {

	// グループを管理
	DrawGroupHierarchy();

	// 指定したグループのパラメータを管理
	DrawParameterInspector();
}

void GameParamEditor::SaveFile(const std::string& groupName) {

	// グループを検索
	std::map<std::string, Group>::iterator itGroup = datas_.find(groupName);

	// 未登録チェック
	assert(itGroup != datas_.end());

	json root;
	root = json::object();
	// jsonオブジェクト登録
	root[groupName] = json::object();

	// シーン名を保存
	root[groupName]["SceneName"] = activeSceneName_;

	// 各項目について
	for (std::map<std::string, Item>::iterator itItem = itGroup->second.items.begin(); itItem != itGroup->second.items.end(); ++itItem) {

		// 項目名を取得
		const std::string& itemName = itItem->first;
		// 項目の参照を取得
		Item& item = itItem->second;

		// jsonに値を保存する
		json & jsonNode = root[groupName][itemName];
		std::visit(JsonSaveVisitor{ jsonNode }, item.value);
	}

	// ディレクトリがなければ作成する
	std::filesystem::path dir(kDirectoryPath);
	if (!std::filesystem::exists(kDirectoryPath)) {
		std::filesystem::create_directory(kDirectoryPath);
	}

	// 書き込むJSONファイルのフルパスを合成する
	std::string filePath = kDirectoryPath + groupName + ".json";
	// 書き込み用ファイルストリーム
	std::ofstream ofs;
	// ファイルを書き込み用に開く
	ofs.open(filePath);

	// ファイルオープン失敗
	if (ofs.fail()) {
		std::string message = "Failed open data file for write.";
		MessageBoxA(nullptr, message.c_str(), "GameParamEditor", 0);
		assert(0);
		return;
	}

	// ファイルにjson文字列を書き込む(インデント幅4)
	ofs << std::setw(4) << root << std::endl;
	// ファイルを閉じる
	ofs.close();
}

void GameParamEditor::LoadFiles() {

	// ディレクトリがなければスキップする
	std::filesystem::path dir(kDirectoryPath);
	if (!std::filesystem::exists(kDirectoryPath)) {
		return;
	}

	std::filesystem::directory_iterator dir_it(kDirectoryPath);
	for (const std::filesystem::directory_entry& entry : dir_it) {

		// ファイルパスを取得
		const std::filesystem::path& filePath = entry.path();

		// ファイル拡張子を取得
		std::string extension = filePath.extension().string();
		// .jsonファイル以外はスキップ
		if (extension.compare(".json") != 0) {
			continue;
		}

		// ファイル読み込み
		LoadFile(filePath.stem().string());
	}

}

void GameParamEditor::LoadFile(const std::string& groupName) {

	// 読み込みJSONファイルのフルパスを合成する
	std::string filePath = kDirectoryPath + groupName + ".json";
	// 読み込み用ファイルストリーム
	std::ifstream ifs;
	// ファイルを読み込み用に開く
	ifs.open(filePath);

	// ファイルオープン失敗
	if (ifs.fail()) {
		std::string message = "Failed open data file for load.";
		MessageBoxA(nullptr, message.c_str(), "GameParamEditor", 0);
		assert(0);
	}

	
	json root;

	// json文字列からjsonのデータ構造に展開
	ifs >> root;
	// ファイルを閉じる
	ifs.close();

	// シーン名を読み込み
	std::string sceneName = root[groupName]["SceneName"].get<std::string>();
	datas_[groupName].sceneName = sceneName;

	// グループを検索
	json::iterator itGroup = root.find(groupName);
	// 未登録チェック
	assert(itGroup != root.end());

	// 各アイテムについて
	for (json::iterator itItem = itGroup->begin(); itItem != itGroup->end(); ++itItem) {
		// アイテム名を取得
		const std::string& itemName = itItem.key();
		if (itemName == "SceneName") {
			continue;
		}

		// パラメータの型を取得
		const auto itemType = itItem->type();

		switch (itemType)
		{
			// bool型を取得
		case json::value_t::boolean:
			SetValue(groupName, itemName, itItem->get<bool>());
			break;

			// int32_t型を取得
		case json::value_t::number_integer:
			SetValue(groupName, itemName, itItem->get<int32_t>());
			break;

			// uint32_t型を取得
		case json::value_t::number_unsigned:
			SetValue(groupName, itemName, itItem->get<uint32_t>());
			break;

			// float型を取得
		case json::value_t::number_float:
			SetValue(groupName, itemName, itItem->get<float>());
			break;

			// Vector型を取得
		case json::value_t::array:
			if (itItem->size() == 3) {
				Vector3 value = { itItem->at(0), itItem->at(1), itItem->at(2) };
				SetValue(groupName, itemName, value);
			} else if (itItem->size() == 2) {
				Vector2 value = { itItem->at(0), itItem->at(1) };
				SetValue(groupName, itemName, value);
			}
			break;

			// std::string型を取得
		case json::value_t::string:
			SetValue(groupName, itemName, itItem->get<std::string>());
			break;

		default:
			break;
		}
	}
}

void GameParamEditor::SelectGroup(const std::string& groupName) {
	selectedGroupName_ = groupName;
}

void GameParamEditor::SetActiveScene(const std::string& sceneName) {
	activeSceneName_ = sceneName;
}

void GameParamEditor::DrawGroupHierarchy() {

	if (!ImGui::Begin("ParameterHierarchy")) {
		ImGui::End();
		return;
	}

	// シーンフィルタ表示
	if (activeSceneName_.empty()) {
		ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "All Groups");
		ImGui::Separator();
	} else {
		ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Active Scene: %s", activeSceneName_.c_str());
		ImGui::Separator();
	}

	// 各グループをリスト表示
	for (auto& [groupName, group] : datas_) {

		// 指定したシーンにあった項目を表示する
		if (!activeSceneName_.empty() &&!group.sceneName.empty() &&
			group.sceneName != activeSceneName_) {
			continue;
		}

		bool isSelected = (selectedGroupName_ == groupName);

		// 項目数を表示
		std::string label = groupName;
		if (ImGui::Selectable(label.c_str(), isSelected)) {
			SelectGroup(groupName);
		}

		// 右クリックメニュー
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Save")) {
				SaveFile(groupName);
				std::string message = std::format("{}.json saved.", groupName);
				MessageBoxA(nullptr, message.c_str(), "GameParamEditor", 0);
			}
			if (ImGui::MenuItem("Load")) {
				LoadFile(groupName);
			}
			ImGui::EndPopup();
		}
	}

	ImGui::End();
}

void GameParamEditor::DrawParameterInspector() {
	if (!ImGui::Begin("Parameter Inspector")) {
		ImGui::End();
		return;
	}

	// グループが選択されていない場合の表示
	if (selectedGroupName_.empty()) {
		ImGui::TextDisabled("No group selected");
		ImGui::TextWrapped("No select");
		ImGui::End();
		return;
	}

	// 選択されたグループが存在するかチェック
	auto itGroup = datas_.find(selectedGroupName_);
	if (itGroup == datas_.end()) {
		ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Group not found");
		selectedGroupName_.clear();
		ImGui::End();
		return;
	}

	// グループ名をヘッダーに表示
	ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Group: %s", selectedGroupName_.c_str());
	ImGui::Separator();

	// 保存ボタン
	if (ImGui::Button("Save")) {
		SaveFile(selectedGroupName_);
		std::string message = std::format("{}.json saved.", selectedGroupName_);
		MessageBoxA(nullptr, message.c_str(), "GameParamEditor", 0);
	}

	ImGui::SameLine();

	// 読み込みボタン
	if (ImGui::Button("Load")) {
		LoadFile(selectedGroupName_);
	}

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// グループの参照を取得
	Group& group = itGroup->second;

	// パラメータが無い場合
	if (group.items.empty()) {
		ImGui::TextDisabled("No parameters in this group");
		ImGui::TextWrapped("None parameter");
		ImGui::End();
		return;
	}

	for (auto& [itemName, item] : group.items) {
		ImGui::PushID(itemName.c_str());

		// 型に応じて編集UI表示
		std::visit(DebugParameterVisitor{ itemName }, item.value);

		ImGui::PopID();
	}

	ImGui::End();
}