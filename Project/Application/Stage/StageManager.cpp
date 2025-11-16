#include"StageManager.h"
#include"GameParamEditor.h"
#include<numbers>
#include<cmath>

#include"ModelRenderer.h"
using namespace GameEngine;

void StageManager::Initialize() {
	// 生成する
	GenerateWalls();

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
	// 値を適応する
	ApplyDebugParam();
#else
	// 値を適応させる
	ApplyDebugParam();
#endif
}

void StageManager::Update() {

}

void StageManager::Draw(GameEngine::Model* wallModel) {

	// 壁を描画
	for (auto& wall : walls_) {
		ModelRenderer::Draw(wallModel, wall->GetWorldTransform());
	}
}

void StageManager::GenerateWalls() {
	walls_.clear();
	walls_.reserve(maxSideNumber_);

	// 等分した角度を求める
	float centralAngle = std::numbers::pi_v<float> *2.0f / static_cast<float>(maxSideNumber_);

	// 円の1辺の長さを求める
	float wallWidth = (std::numbers::pi_v<float> *2.0f * radius_) * (centralAngle / (std::numbers::pi_v<float> * 2.0f)) + offsetWallWidth_;

	// 円を生成する
	for (uint32_t i = 0; i < maxSideNumber_; ++i) {
		float angle = i * centralAngle;

		// 円状の位置を求める
		Vector3 anglePos = {
			std::cosf(angle) * radius_,
			0.0f,
			std::sinf(angle) * radius_
		};

		// 生成位置に移動
		Vector3 tmpPos = centerPosition_ + anglePos;

		// 壁の向き
		Vector3 dir = centerPosition_ - tmpPos;
		float rotateY = std::atan2f(dir.x, dir.z);

		// 壁を生成する
		std::unique_ptr<Wall> tmpWall = std::make_unique<Wall>();
		tmpWall->Initialilze({ {wallWidth,wallHeight_,wallDepth_},{0.0f,rotateY,0.0f},{tmpPos} }, respawnTime_, maxHp_);

		// 登録する
		walls_.push_back(std::move(tmpWall));
	}
}

void StageManager::RegisterBebugParam() {
	int index = 0;
	GameParamEditor::GetInstance()->AddItem("StageManager", "CenterPosition", centerPosition_, index++);
	GameParamEditor::GetInstance()->AddItem("StageManager", "Radius", radius_, index++);
	GameParamEditor::GetInstance()->AddItem("StageManager", "MaxSideNum", maxSideNumber_, index++);
	GameParamEditor::GetInstance()->AddItem("StageManager", "WallRespawnTime", respawnTime_, index++);
	GameParamEditor::GetInstance()->AddItem("StageManager", "WallMaxHp", maxHp_, index++);
	GameParamEditor::GetInstance()->AddItem("StageManager", "WallDepth", wallDepth_, index++);
	GameParamEditor::GetInstance()->AddItem("StageManager", "WallHeight", wallHeight_, index++);
	GameParamEditor::GetInstance()->AddItem("StageManager", "OffsetWallWidth", offsetWallWidth_, index++);
	// デバック用パラメータ
	GameParamEditor::GetInstance()->AddItem("StageManager", "IsCreate", isCreate_, index++);
}

void StageManager::ApplyDebugParam() {
	centerPosition_ = GameParamEditor::GetInstance()->GetValue<Vector3>("StageManager", "CenterPosition");
	radius_ = GameParamEditor::GetInstance()->GetValue<float>("StageManager", "Radius");
	maxSideNumber_ = GameParamEditor::GetInstance()->GetValue<uint32_t>("StageManager", "MaxSideNum");
	respawnTime_ = GameParamEditor::GetInstance()->GetValue<float>("StageManager", "WallRespawnTime");
	maxHp_ = GameParamEditor::GetInstance()->GetValue<uint32_t>("StageManager", "WallMaxHp");
	wallDepth_ = GameParamEditor::GetInstance()->GetValue<float>("StageManager", "WallDepth");
	wallHeight_ = GameParamEditor::GetInstance()->GetValue<float>("StageManager", "WallHeight");
	offsetWallWidth_ = GameParamEditor::GetInstance()->GetValue<float>("StageManager", "OffsetWallWidth");

	isCreate_ = GameParamEditor::GetInstance()->GetValue<bool>("StageManager", "IsCreate");
}

void StageManager::DebugUpdate() {
#ifdef _DEBUG
	// 値を適応
	ApplyDebugParam();
#endif

	if (isCreate_) {
		if (!created_) {
			GenerateWalls();
			isCreate_ = false;
			created_ = true;
		}
	} else {
		created_ = false;
	}
}