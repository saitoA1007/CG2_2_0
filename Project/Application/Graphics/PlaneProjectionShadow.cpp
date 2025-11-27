#include"PlaneProjectionShadow.h"
#include"MyMath.h"

using namespace GameEngine;

void PlaneProjectionShadow::Initialize(GameEngine::WorldTransform* casterWorldTransform) {

	// 投影元のオブジェクト行列
	casterWorldTransform_ = casterWorldTransform;

	// 影の初期化
	worldTransform_.Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,0.0f} });
	// マテリアルを初期化
	material_.Initialize({ 0.5f,0.5f,0.5f,0.4f }, { 1.0f,1.0f,1.0f }, 250.0f, false);

    // 平面投影行列の初期化
    shadowMatrix_ = MakeIdentity4x4();
    shadowMatrix_.m[1][1] = 0.01f;
}

void PlaneProjectionShadow::Update() {

	if (casterWorldTransform_) {
		worldTransform_.SetWorldMatrix(casterWorldTransform_->GetWorldMatrix() * shadowMatrix_);
	}
}