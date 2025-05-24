#include"Emitter.h"
#include<cassert>

#include"EngineSource/2D/ImGuiManager.h"
using namespace GameEngine;

void ParticleEmitter::Initialize(GameEngine::Model* model, const uint32_t& textureHandle) {
	// Nullチェック
	assert(model);

	// モデルの取得
	model_ = model;

	// テクスチャを取得
	textureHandle_ = textureHandle;

	// エミッタの初期化
	emitter_.transform.translate = { 0.0f,0.0f,0.0f };
	emitter_.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter_.transform.scale = { 1.0f,1.0f,1.0f };
	emitter_.count = 3; // 発生数
	emitter_.frequency = 0.5f; // 0.5秒ごとに発生
	emitter_.frequencyTime = 0.0f; // 発生頻度用の時刻、0で初期化

	// ワールド行列の初期化
	worldTransform_.Initialize(emitter_.transform);
}

void ParticleEmitter::Update() {

	emitter_.frequencyTime += kDeltaTime; // 時刻を進める
	if (emitter_.frequency <= emitter_.frequencyTime) { // 頻度より大きいなら発生
		isCall_ = true;  // 発生フラグをtrue
		emitter_.frequencyTime -= emitter_.frequency; // 余計に過ぎた時間を加味して頻度計算する
	} else {
		isCall_ = false;
	}

	// トラスフォームを設定
	worldTransform_.SetTransform(emitter_.transform);

	// 行列の更新
	worldTransform_.UpdateTransformMatrix();

#ifdef _DEBUG
	ImGui::Begin("DebugEmitter");
	ImGui::DragFloat3("EmitterTranslate", &emitter_.transform.translate.x, 0.01f);
	ImGui::End();
#endif
}

void ParticleEmitter::Draw(const Matrix4x4 VPMatrix) {
	model_->Draw(worldTransform_, textureHandle_, VPMatrix);
}