#include"Rope.h"
#include<cassert>

#include"EngineSource/2D/ImguiManager.h"

using namespace GameEngine;

Rope::~Rope() {
	delete primitiveRenderer_;
}

void Rope::Initialize(GameEngine::Model* model, const uint32_t& textureHandle) {
	// NULLチェック
	assert(model);

	// モデルを取得
	sphereModel_ = model;
	sphereModel_->SetDefaultColor({ 1.0f,0.0f,0.0f,1.0f });
	// ワールド行列を更新
	worldTransform_.Initialize({ {0.1f,0.1f,0.1f},{0.0f,0.0f,0.0f},{ 0.0f,0.1f * 2.0f * 32.0f,0.0f} });
	// テクスチャを取得
	textureHandle_ = textureHandle;

	// 線を描画するクラスの初期化
	primitiveRenderer_ = PrimitiveRenderer::Create();

	// 先端座標
	startPos_ = { 0.0f,0.1f * 2.0f * 32.0f,0.0f };

	Node tmpRope;
	// ヒモを設定
	for (uint32_t i = 0; i < segmentCount; ++i) {
		// 半径を求める
		tmpRope.radius = 0.1f;
		// 初期位置を設定
		tmpRope.pos = { startPos_.x,startPos_.y - (static_cast<float>(i) * tmpRope.radius * 2.0f),startPos_.z };

		// 前の位置。(最初は前の位置など存在しないので、現在位置と同じ値を入れておく)
		tmpRope.prePos = tmpRope.pos;

		// 速度を求める
		tmpRope.speed.x = (tmpRope.pos.x - tmpRope.prePos.x) / dt;
		tmpRope.speed.y = (tmpRope.pos.y - tmpRope.prePos.y) / dt;
		tmpRope.speed.z = (tmpRope.pos.z - tmpRope.prePos.z) / dt;

		// 質量
		tmpRope.mass = 0.1f;

		tmpRope.locked = false;

		// 各頂点情報を適応
		points_[i] = tmpRope; 
	}

	// 先頭をロック
	points_[0].locked = true;

	// 取得した点を線データに格納
	for (uint32_t i = 0; i < lineMeshs_.size() - 1; ++i) {

		lineMeshs_[i] = std::make_unique<LineMesh>();
		lineMeshs_[i]->Initialize(points_[i].pos, points_[i + 1].pos, { 1.0f,1.0f,1.0f,1.0f });
	}
}

void Rope::Update() {

	// 先頭を追従させる
	points_[0].pos = startPos_;
	points_[0].prePos = startPos_;
	worldTransform_.SetTranslate(startPos_);

	// 外部による速度の更新
	for (uint32_t i = 0; i < points_.size(); ++i) {
		if (!points_[i].locked) {
			Vector3 temp = points_[i].pos;
			Vector3 v = points_[i].pos - points_[i].prePos;
			points_[i].pos += v; // 慣性(ヒモらしい余計な動きが出来る)
			points_[i].pos.y -= gravity * dt; // 重力加速度(シンプルに外から加わる力を入れる)
			points_[i].prePos = temp;
		}
	}

	// 定義した値より小さければ動きが止まる
	for (auto& node : points_) {
		if (!node.locked) {
			Vector3 velocity = node.pos - node.prePos;

			if (std::fabs(velocity.x) < velocityThreshold && std::fabs(velocity.y) < velocityThreshold && std::fabs(velocity.z) < velocityThreshold) {
				node.prePos = node.pos;
			}
		}
	}

	// 位置制約を解く（PBD）
	SolveDistanceConstraints(points_, 64);

	// 取得した点を線データに格納
	for (uint32_t i = 0; i < lineMeshs_.size() - 1; ++i) {
		lineMeshs_[i]->SetPosition(points_[i].pos, points_[i + 1].pos);
	}


	// 先端の点を更新
	worldTransform_.UpdateTransformMatrix();
	
#ifdef _DEBUG

	ImGui::Begin("DebugRope");

	ImGui::DragFloat3("RopePos[0]", &startPos_.x, 0.01f);

	if (ImGui::Button("start")) {
		points_[31].pos += Vector3(2.0f, 16.0f, 16.0f);
	}

	ImGui::End();
#endif
}

void Rope::DrawLine(const Matrix4x4& VPMatrix) {
	// 線を描画
	for (uint32_t i = 0; i < lineMeshs_.size() - 1; ++i) {
		primitiveRenderer_->DrawLine3d(lineMeshs_[i].get(), VPMatrix);
	}
}

void Rope::DrawSphere(const Matrix4x4& VPMatrix) {
	// ヒモの始点となる球を描画
	sphereModel_->Draw(worldTransform_, textureHandle_, VPMatrix);
}

void Rope::SolveDistanceConstraints(std::array<Node, segmentCount>& rope, int iterations) {
	for (int iter = 0; iter < iterations; ++iter) {
		for (uint32_t i = 0; i < rope.size() - 1; ++i) {
			Node& a = rope[i];
			Node& b = rope[i + 1];

			Vector3 delta = b.pos - a.pos;
			float currentDist = std::sqrtf(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
			float restLength = a.radius + b.radius;

			// 0の場合は飛ばす
			if (currentDist == 0.0f) continue;

			Vector3 correction;
			correction.x = delta.x * ((currentDist - restLength) / currentDist);
			correction.y = delta.y * ((currentDist - restLength) / currentDist);
			correction.z = delta.z * ((currentDist - restLength) / currentDist);

			// 逆質量を計算（0除算防止）
			float invMassA = (a.locked || a.mass <= 0.0f) ? 0.0f : 1.0f / a.mass;
			float invMassB = (b.locked || b.mass <= 0.0f) ? 0.0f : 1.0f / b.mass;
			float invMassSum = invMassA + invMassB;
			if (invMassSum == 0.0f) continue;

			// 補正量を質量比で分配
			if (!a.locked) {
				a.pos.x += correction.x * (invMassA / invMassSum);
				a.pos.y += correction.y * (invMassA / invMassSum);
				a.pos.z += correction.z * (invMassA / invMassSum);
			}
			if (!b.locked) {
				b.pos.x -= correction.x * (invMassB / invMassSum);
				b.pos.y -= correction.y * (invMassB / invMassSum);
				b.pos.z -= correction.z * (invMassB / invMassSum);
			}
		}
	}
}