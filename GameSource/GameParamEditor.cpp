#include"GameParamEditor.h"
#include<cassert>

#include"EngineSource/2D/ImguiManager.h"

using namespace GameEngine;

GameParamEditor* GameParamEditor::GetInstance() {
	static GameParamEditor instance;
	return &instance;
}

void GameParamEditor::CreateGroup(const std::string& groupName) {
	// 指定名のオブジェクトが無ければ追加する
	datas_[groupName];
}

void GameParamEditor::Update() {
	if (!ImGui::Begin("GameParamater", nullptr)) {
		ImGui::End();
		return;
	}

	// 各グループについて
	for (std::map<std::string, Group>::iterator itGroup = datas_.begin();
		itGroup != datas_.end(); ++itGroup) {

		// グループ名を取得
		const std::string& groupName = itGroup->first;
		// グループの参照を取得
		Group& group = itGroup->second;

		// コラプシングヘッダーでグループを表示（デフォルトで開いた状態も可能、ImGuiTreeNodeFlags_DefaultOpen)
		if (ImGui::CollapsingHeader(groupName.c_str())) {

			// インデントを追加して階層感を出す
			ImGui::Indent();

			// 各項目について
			for (std::map<std::string, Item>::iterator itItem = group.items.begin();
				itItem != group.items.end(); ++itItem) {

				// 項目名を取得
				const std::string& itemName = itItem->first;
				// 項目の参照を取得
				Item& item = itItem->second;

				// 各型の項目を表示
				if (std::holds_alternative<int32_t>(item.value)) {
					// int32_t型の値を保持していれば
					int32_t* ptr = std::get_if<int32_t>(&item.value);
					ImGui::DragInt(itemName.c_str(), ptr, 1);

				} else if (std::holds_alternative<float>(item.value)) {
					// float型の値を保持していれば
					float* ptr = std::get_if<float>(&item.value);
					ImGui::DragFloat(itemName.c_str(), ptr, 0.01f);

				} else if (std::holds_alternative<Vector3>(item.value)) {
					// Vector3型の値を保持していれば
					Vector3* ptr = std::get_if<Vector3>(&item.value);
					ImGui::DragFloat3(itemName.c_str(), reinterpret_cast<float*>(ptr), 0.01f);
				}
			}

			// 少しスペースを空ける
			ImGui::Spacing();

			// 保存ボタン
			if (ImGui::Button(("Save " + groupName).c_str())) {
				SaveFile(groupName);
				std::string message = std::format("{}.json saved.", groupName);
				MessageBoxA(nullptr, message.c_str(), "GameParamEditor", 0);
			}

			// インデントを戻す
			ImGui::Unindent();

			// グループ間にセパレータを追加（見やすさのため）
			ImGui::Separator();
		}
	}

	ImGui::End();
}

void GameParamEditor::SetValue(const std::string& groupName, const std::string& key, int32_t value) {
	// グループの参照を取得
	Group& group = datas_[groupName];
	// 新しい項目のデータを設定
	Item newItem{};
	newItem.value = value;
	// 設定した項目をstd::mapに追加
	group.items[key] = newItem;
}

void GameParamEditor::SetValue(const std::string& groupName, const std::string& key, float value) {
	// グループの参照を取得
	Group& group = datas_[groupName];
	// 新しい項目のデータを設定
	Item newItem{};
	newItem.value = value;
	// 設定した項目をstd::mapに追加
	group.items[key] = newItem;
}

void GameParamEditor::SetValue(const std::string& groupName, const std::string& key, const Vector3& value) {
	// グループの参照を取得
	Group& group = datas_[groupName];
	// 新しい項目のデータを設定
	Item newItem{};
	newItem.value = value;
	// 設定した項目をstd::mapに追加
	group.items[key] = newItem;
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

	// 各項目について
	for (std::map<std::string, Item>::iterator itItem = itGroup->second.items.begin(); itItem != itGroup->second.items.end(); ++itItem) {

		// 項目名を取得
		const std::string& itemName = itItem->first;
		// 項目の参照を取得
		Item& item = itItem->second;

		// int32_t型の値を保持
		if (std::holds_alternative<int32_t>(item.value)) {
			// int32_t型の値を登録
			root[groupName][itemName] = std::get<int32_t>(item.value);

		} else if (std::holds_alternative<float>(item.value)) {
			// float型の値を登録
			root[groupName][itemName] = std::get<float>(item.value);

		} else if (std::holds_alternative<Vector3>(item.value)) {
			// Vector3型の値を登録
			Vector3 value = std::get<Vector3>(item.value);
			root[groupName][itemName] = json::array({ value.x,value.y,value.z });
		}
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

	// グループを検索
	json::iterator itGroup = root.find(groupName);
	// 未登録チェック
	assert(itGroup != root.end());

	// 各アイテムについて
	for (json::iterator itItem = itGroup->begin(); itItem != itGroup->end(); ++itItem) {
		// アイテム名を取得
		const std::string& itemName = itItem.key();

		// int32_t型の値を保持
		if (itItem->is_number_integer()) {
			// int型の値を登録
			int32_t value = itItem->get<int32_t>();
			SetValue(groupName, itemName, value);

		} else if (itItem->is_number_float()) {
			float value = itItem->get<float>();
			SetValue(groupName, itemName, value);
		} else if (itItem->is_array() && itItem->size() == 3) {
			// float型のjson配列登録
			Vector3 value = { itItem->at(0),itItem->at(1), itItem->at(2) };
			SetValue(groupName, itemName, value);
		}
	}
}

void GameParamEditor::AddItem(const std::string& groupName, const std::string& key, int32_t value) {
	// グループの参照を取得
	Group& group = datas_[groupName];

	// すでに登録されていれば何もしない
	if (group.items.find(key) != group.items.end()) {
		return;
	}

	// 新しい項目のデータを設定
	Item newItem{};
	newItem.value = value;
	// 設定した項目をstd::mapに追加
	group.items[key] = newItem;
}

void GameParamEditor::AddItem(const std::string& groupName, const std::string& key, float value) {
	// グループの参照を取得
	Group& group = datas_[groupName];

	// すでに登録されていれば何もしない
	if (group.items.find(key) != group.items.end()) {
		return;
	}

	// 新しい項目のデータを設定
	Item newItem{};
	newItem.value = value;
	// 設定した項目をstd::mapに追加
	group.items[key] = newItem;
}

void GameParamEditor::AddItem(const std::string& groupName, const std::string& key, const Vector3& value) {
	// グループの参照を取得
	Group& group = datas_[groupName];

	// すでに登録されていれば何もしない
	if (group.items.find(key) != group.items.end()) {
		return;
	}

	// 新しい項目のデータを設定
	Item newItem{};
	newItem.value = value;
	// 設定した項目をstd::mapに追加
	group.items[key] = newItem;
}

int32_t GameParamEditor::GetIntValue(const std::string& groupName, const std::string& key) const {
	// 指定グループが存在するかチェック
	assert(datas_.find(groupName) != datas_.end());

	// 指定キーが存在するかチェック
	const Group& group = datas_.at(groupName);
	assert(group.items.find(key) != group.items.end());

	auto itItem = group.items.find(key);

	// グループの参照を取得
	return std::get<int32_t>(itItem->second.value);
}

float GameParamEditor::GetFloatValue(const std::string& groupName, const std::string& key) const {
	// 指定グループが存在するかチェック
	assert(datas_.find(groupName) != datas_.end());

	// 指定キーが存在するかチェック
	const Group& group = datas_.at(groupName);
	assert(group.items.find(key) != group.items.end());

	auto itItem = group.items.find(key);

	// グループの参照を取得
	return std::get<float>(itItem->second.value);
}

Vector3 GameParamEditor::GetVector3Value(const std::string& groupName, const std::string& key) const {
	// 指定グループが存在するかチェック
	assert(datas_.find(groupName) != datas_.end());

	// 指定キーが存在するかチェック
	const Group& group = datas_.at(groupName);
	assert(group.items.find(key) != group.items.end());

	auto itItem = group.items.find(key);

	// グループの参照を取得
	return std::get<Vector3>(itItem->second.value);
}