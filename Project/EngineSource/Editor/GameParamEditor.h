#pragma once
#include<iostream>
#include<variant>
#include<map>
#include<string>

#include"Vector3.h"
#include"Vector2.h"

#include<json.hpp>
#include"ImguiManager.h"

using json = nlohmann::json;

class GameParamEditor final {
public:

	// 項目
	struct Item {
		std::variant<int32_t, uint32_t, float, Vector2, Vector3, bool, std::string> value;
	};

	// グループ
	struct Group {
		std::map<std::string, Item> items;
		std::string sceneName; // シーンの名前
	};

public:

	// シングルトン
	static GameParamEditor* GetInstance();
	
	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// グループ作成
	/// </summary>
	/// <param name="groupName"></param>
	void CreateGroup(const std::string& groupName,const std::string& sceneName = "none");

	/// <summary>
	/// ディレクトリの全ファイル読み込み
	/// </summary>
	void LoadFiles();

	/// <summary>
	/// ファイルから読み込む
	/// </summary>
	/// <param name="groupName"></param>
	void LoadFile(const std::string& groupName);

	/// <summary>
	/// グループを選択
	/// </summary>
	/// <param name="groupName"></param>
	void SelectGroup(const std::string& groupName);

	/// <summary>
	/// 選択中のグループを取得
	/// </summary>
	/// <returns></returns>
	const std::string& GetSelectGroup() const { return selectedGroupName_; }

	/// <summary>
	/// シーンを選択
	/// </summary>
	/// <param name="sceneName"></param>
	void SetActiveScene(const std::string& sceneName);

	/// <summary>
	/// 選択中のシーンを取得
	/// </summary>
	/// <returns></returns>
	const std::string& GetActiveScene() const { return activeSceneName_; }

	/// <summary>
	/// 値を登録する
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="groupName"></param>
	/// <param name="key"></param>
	/// <param name="value"></param>
	template<typename T>
	void AddItem(const std::string& groupName, const std::string& key, const T& value) {
		// グループの参照を取得
		Group& group = datas_[groupName];

		// すでに登録されていれば何もしない
		if (group.items.find(key) != group.items.end()) {
			return;
		}

		// アクティブなシーンを登録
		if (group.sceneName.empty()) {
			group.sceneName = activeSceneName_;
		}

		// 新しい項目のデータを設定
		Item newItem{};
		newItem.value = value;
		// 設定した項目をstd::mapに追加
		group.items[key] = newItem;
	}

	/// <summary>
	/// 登録した値を取得する
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="groupName"></param>
	/// <param name="key"></param>
	/// <returns></returns>
	template<typename T>
	T GetValue(const std::string& groupName, const std::string& key) const {
		// 指定グループが存在するかチェック
		assert(datas_.find(groupName) != datas_.end());

		// 指定キーが存在するかチェック
		const Group& group = datas_.at(groupName);
		assert(group.items.find(key) != group.items.end());

		auto itItem = group.items.find(key);

		// 型があるかを確認する
		assert(std::holds_alternative<T>(itItem->second.value));

		// グループの参照を取得
		return std::get<T>(itItem->second.value);
	}

private:
	GameParamEditor() = default;
	~GameParamEditor() = default;
	GameParamEditor(const GameParamEditor&) = delete;
	GameParamEditor& operator=(const GameParamEditor&) = delete;

	// 全データ
	std::map<std::string, Group> datas_;

	// 選択中のグループ名
	std::string selectedGroupName_;

	// 現在アクティブなシーン名
	std::string activeSceneName_ = "None";

	// グローバル変数の保存先ファイルパス
	const std::string kDirectoryPath = "Resources/Json/";

private:

	// jsonファイルに値を保存するためのビジター
	struct JsonSaveVisitor {
		json& jsonData;
		explicit JsonSaveVisitor(json& jsonNode) : jsonData(jsonNode){}
		
		void operator()(const Vector3& value) const {
			jsonData = json::array({ value.x, value.y, value.z });
		}

		void operator()(const Vector2& value) const {
			jsonData = json::array({ value.x, value.y });
		}

		template<typename T>
		void operator()(const T& value) const {
			jsonData = value;
		}
	};

	// ImGuiで表示する用のパラメータを管理する
	struct DebugParameterVisitor {
		const std::string& itemName;
		explicit DebugParameterVisitor(const std::string& name) : itemName(name) {}

		void operator()(int32_t& value) const {
			ImGui::DragInt(itemName.c_str(), &value, 1);
		}

		void operator()(uint32_t& value) const {
			ImGui::DragScalar(itemName.c_str(), ImGuiDataType_U32, &value, 1.0f);
		}

		void operator()(float& value) const {
			ImGui::DragFloat(itemName.c_str(), &value, 0.01f);
		}

		void operator()(Vector2& value) const {
			ImGui::DragFloat2(itemName.c_str(), reinterpret_cast<float*>(&value), 0.01f);
		}

		void operator()(Vector3& value) const {
			ImGui::DragFloat3(itemName.c_str(), reinterpret_cast<float*>(&value), 0.01f);
		}

		void operator()(bool& value) const {
			ImGui::Checkbox(itemName.c_str(), &value);
		}

		void operator()(std::string& value) const {
			//ImGui::InputText(itemName.c_str(), value.data());
			ImGui::Text(itemName.c_str(), &value);
		}

		// 対応出来ない型がきた場合の処理
		template<typename T>
		void operator()(T& value) const {
			ImGui::TextColored(ImVec4(1, 0, 0, 1), "[%s] は未対応の型です", itemName.c_str());
		}
	};

private:

	/// <summary>
	/// jsonファイルに保存する
	/// </summary>
	/// <param name="groupName"></param>
	void SaveFile(const std::string& groupName);

	/// <summary>
	/// グループの管理
	/// </summary>
	void DrawGroupHierarchy();

	/// <summary>
	/// グループに存在するパラメータを管理
	/// </summary>
	void DrawParameterInspector();

	template<typename T>
	void SetValue(const std::string& groupName, const std::string& key, T value) {
		// グループの参照を取得
		Group& group = datas_[groupName];
		// 新しい項目のデータを設定
		Item newItem{};
		newItem.value = value;
		// 設定した項目をstd::mapに追加
		group.items[key] = newItem;
	}
};