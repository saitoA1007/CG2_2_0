#pragma once
#include<iostream>
#include<variant>
#include<map>
#include<string>

#include"Vector3.h"

#include<json.hpp>

using json = nlohmann::json;

class GameParamEditor final {
public:

	// 項目
	struct Item {
		std::variant<int32_t, float, Vector3> value;
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


	void AddItem(const std::string& groupName, const std::string& key, int32_t value);

	void AddItem(const std::string& groupName, const std::string& key, float value);

	void AddItem(const std::string& groupName, const std::string& key, const Vector3& value);

	int32_t GetIntValue(const std::string& groupName, const std::string& key) const;

	float GetFloatValue(const std::string& groupName, const std::string& key) const;

	Vector3 GetVector3Value(const std::string& groupName, const std::string& key) const;

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

	void SetActiveScene(const std::string& sceneName);

	const std::string& GetActiveScene() const { return activeSceneName_; }

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

	/// <summary>
	/// jsonファイルに保存する
	/// </summary>
	/// <param name="groupName"></param>
	void SaveFile(const std::string& groupName);

	void SetValue(const std::string& groupName, const std::string& key, int32_t value);

	void SetValue(const std::string& groupName, const std::string& key, float value);

	void SetValue(const std::string& groupName, const std::string& key, const Vector3& value);

	void DrawGroupHierarchy();

	void DrawParameterInspector();
};