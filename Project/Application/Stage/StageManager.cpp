#include"StageManager.h"

void StageManager::Initialize() {

	// 壁を生成する
	GenerateWalls();
}

void StageManager::GenerateWalls() {
	// メモリを確保
	walls_.resize(4);

	for (size_t i = 0; i < 4; ++i) {

		Vector3 wallSize = {};
		Vector3 pos = {0.0f,0.0f,0.0f};

		if (i == 0) {
			// 右
			wallSize = wallSize_;
			pos.x = stageSize_.x * 0.5f + wallSize.x * 0.5f;
		} else if (i == 1) {
			// 左
			wallSize = wallSize_;
			pos.x = (stageSize_.x * 0.5f + wallSize.x * 0.5f) * -1.0f;
			
		} else if (i == 2) {
			// 上
			wallSize = { wallSize_.z,wallSize_.y,wallSize_.x };
			pos.z = stageSize_.z * 0.5f + wallSize.z * 0.5f;
		} else {
			// 下
			wallSize = { wallSize_.z,wallSize_.y,wallSize_.x };
			pos.z = (stageSize_.z * 0.5f + wallSize.z * 0.5f) * -1.0f;
		}

		// 壁を生成
		std::unique_ptr<Wall> tmpWall = std::make_unique<Wall>();
		tmpWall->Initialize({ wallSize ,{0.0f,0.0f,0.0f},pos });
		// 壁を登録
		walls_[i] = std::move(tmpWall);
	}
}