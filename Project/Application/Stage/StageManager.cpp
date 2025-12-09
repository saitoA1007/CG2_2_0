#include"StageManager.h"
#include"GameParamEditor.h"
#include<numbers>
#include<cmath>

#include"Extension/CustomRenderer.h"
using namespace GameEngine;

void StageManager::Initialize(const uint32_t& wallTexture) {

	wallTextureHandle_ = wallTexture;

#ifdef _DEBUG
	// 値を登録する
	RegisterBebugParam();
#endif

	// 値を適応させる
	ApplyDebugParam();

	// 生成する
	GenerateWalls();

	aliveWalls_.clear();

	// 更新処理
	for (auto &wall : walls_) {
		// 壁の更新処理
		//wall->Update();

		// 生存している壁をリストに追加する
		if (wall->GetIsAlive()) {
			aliveWalls_.push_back(wall.get());
		}
	}
}

void StageManager::Update() {

	// 生存リストをクリア
	//aliveWalls_.clear();

	// 更新処理
	for (auto& wall : walls_) {

		// 壁が壊れた演出を追加
		if (wall->IsBreakParticleActive()) {
			AddBreakWallParticle(wall->GetWorldPosition());
		}

		// 壁の更新処理
		wall->Update();

		// 生存している壁をリストに追加する
		/*if (wall->GetIsAlive()) {
			aliveWalls_.push_back(wall.get());
		}*/
	}

	// 壁を壊す演出
	breakWallParticles_.remove_if([](const std::unique_ptr<BreakWallParticle>& breakWallParticle) {
		return breakWallParticle->IsFinished();
	});
	// 破壊演出の更新処理
	for (auto& breakWallParticle : breakWallParticles_) {
		breakWallParticle->Update();
	}
}

void StageManager::Draw(GameEngine::Model* wallModel, ID3D12Resource* lightGroupResource) {

	// 生存している壁を描画
	for (auto& wall : aliveWalls_) {
		CustomRenderer::DrawRock(wallModel, wall->GetWorldTransform(),lightGroupResource,wall->GetMaterial());
	}
}

void StageManager::GenerateWalls() {
	walls_.clear();
	walls_.reserve(maxSideNumber_);
	aliveWalls_.clear();
	aliveWalls_.reserve(maxSideNumber_);

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
		tmpWall->Initialilze({ {wallWidth,wallHeight_,wallDepth_},{0.0f,rotateY,0.0f},{tmpPos} }, respawnTime_, maxHp_, wallTextureHandle_);

		// 登録する
		walls_.push_back(std::move(tmpWall));
	}
}

void StageManager::AddBreakWallParticle(const Vector3& pos) {
	std::unique_ptr<BreakWallParticle> tmp = std::make_unique<BreakWallParticle>();
	tmp->Initialize(0, pos);
	tmp->SetEmitterPos(pos);
	breakWallParticles_.push_back(std::move(tmp));
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

	// 生存リストをクリア
	aliveWalls_.clear();
	// 生存している壁をリストに追加する
	for (auto& wall : walls_) {
		if (wall->GetIsAlive()) {
			aliveWalls_.push_back(wall.get());
		}
	}
}