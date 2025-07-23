#include"FPSCounter.h"
#include"EngineSource/2D/ImGuiManager.h"
using namespace GameEngine;

void FpsCounter::Initialize() {
	preTime_ = std::chrono::high_resolution_clock::now();
}

void FpsCounter::Update() {
	// 現在時刻を取得
	currentTime_ = std::chrono::high_resolution_clock::now();

	// 前のフレームとの経過時間を求める
	std::chrono::duration<float> delta = currentTime_ - preTime_;
	preTime_ = currentTime_;

	// 秒数を取得
	elapsedTime_ += delta.count();
	frameCount_++;

	// 1秒ごとにFPSを更新
	if (elapsedTime_ >= 1.0f) {
		maxFrameCount_ = frameCount_;
		elapsedTime_ = 0.0f;
		frameCount_ = 0;
	}
}

void FpsCounter::DrawImGui() {
	ImGui::Begin("FpsCounter");
	ImGui::DragInt("FpsCount", &frameCount_);
	ImGui::DragInt("MaxFpsCount", &maxFrameCount_);
	ImGui::End();
}