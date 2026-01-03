#pragma once
#include<vector>
#include"Wall.h"

class StageManager {
public:

	void Initialize();

	/// <summary>
	/// 壁の要素を取得
	/// </summary>
	/// <returns></returns>
	std::vector<std::unique_ptr<Wall>>& GetWalls() { return walls_; }

private:

	// 生成する壁
	std::vector<std::unique_ptr<Wall>> walls_;

	// ステージの大きさ
	Vector3 stageSize_ = {50.0f,0.0f,50.0f};

	// 壁の大きさ
	Vector3 wallSize_ = {5.0f,2.0f,50.0f};

private:

	// ステージを生成する
	void GenerateWalls();
};