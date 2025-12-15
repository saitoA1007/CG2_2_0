#include "TutorialUI.h"
using namespace GameEngine;

void TutorialUI::Initialize(GameEngine::TextureManager* textureManager) {
    sprites_.clear();
    sprites_.reserve(3);
    textureHandles_.clear();
    textureHandles_.reserve(3);

    // スプライト0
    {
        auto spr = Sprite::Create(sprite0Pos_, sprite0Size_, {0.0f, 0.5f}, {1.0f,1.0f,1.0f,1.0f});
        sprites_.push_back(std::move(spr));
        uint32_t gh = textureManager->GetHandleByName("Tutorial_01.png");
        textureHandles_.push_back(gh);
    }
    // スプライト1
    {
        auto spr = Sprite::Create(sprite1Pos_, sprite1Size_, {0.0f, 0.5f}, {1.0f,1.0f,1.0f,1.0f});
        sprites_.push_back(std::move(spr));
        uint32_t gh = textureManager->GetHandleByName("Tutorial_02.png");
        textureHandles_.push_back(gh);
    }
    // スプライト2
    {
        auto spr = Sprite::Create(sprite2Pos_, sprite2Size_, {0.0f, 0.5f}, {1.0f,1.0f,1.0f,1.0f});
        sprites_.push_back(std::move(spr));
        uint32_t gh = textureManager->GetHandleByName("Tutorial_03.png");
        textureHandles_.push_back(gh);
    }
}

void TutorialUI::Update() {
    for (auto& spr : sprites_) {
        spr->Update();
    }
}
