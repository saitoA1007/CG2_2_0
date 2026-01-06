#include"EffectCamera.h"

using namespace GameEngine;

void EffectCamera::Initialize() {
	// カメラの初期化
	camera_ = std::make_unique<Camera>();
	camera_->Initialize({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,4.0f,-10.0f} }, 1280, 720);
}

void EffectCamera::Update() {
	
}