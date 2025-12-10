#include"ArrowUI.h"
#include"EasingManager.h"
#include"FPSCounter.h"
#include"MyMath.h"
using namespace GameEngine;

void ArrowUI::Initialize(const uint32_t& texture) {

    worldTransform_.Initialize({ {1.5f,1.5f,1.5f},{0.0f,0.0f,0.0f},{0.0f,15.0f,0.0f} });

    material_.Initialize({ 1.0f,0.0f,0.0f,1.0f }, {}, 0.0f, false);
    material_.SetTextureHandle(texture);
}

void ArrowUI::Update(const Matrix4x4& cameraMatrix) {

    if (!isActive_) { return; }
 
    timer_ += FpsCounter::deltaTime;
    float posY = 0.0f;
    if (timer_ <= 0.5f) {

        float localT = timer_ / 0.5f;

        posY = Lerp(0.0f, 2.0f, EaseIn(localT));
    } else {

        float localT = (timer_ - 0.5f) / 0.5f;

        posY = Lerp(2.0f, 0.0f, EaseIn(localT));
    }

    if (timer_ >= 1.0f) {
        timer_ = 0.0f;
    }

    worldTransform_.SetWorldMatrix(MakeBillboardMatrix(worldTransform_.transform_.scale, worldTransform_.transform_.translate + Vector3(0.0f, posY,0.0f), cameraMatrix, worldTransform_.transform_.rotate.z));
}
