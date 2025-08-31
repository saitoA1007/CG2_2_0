#pragma once
#include<iostream>
#include<variant>
#include<map>
#include<string>

#include"EngineSource/Math/Vector3.h"

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
	void CreateGroup(const std::string& groupName);

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
	
private:
	GameParamEditor() = default;
	~GameParamEditor() = default;
	GameParamEditor(const GameParamEditor&) = delete;
	GameParamEditor& operator=(const GameParamEditor&) = delete;

	// 全データ
	std::map<std::string, Group> datas_;

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
};