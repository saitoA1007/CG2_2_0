#pragma once
#include"Input.h"
#include"TextureManager.h"
#include"AudioManager.h"

#include"InputCommand.h"
#include"ModelManager.h"

// シーンで使用するエンジン機能
struct SceneContext {
	GameEngine::Input* input = nullptr;
	GameEngine::InputCommand* inputCommand = nullptr;
	GameEngine::ModelManager* modelManager = nullptr;
	GameEngine::TextureManager* textureManager = nullptr;
	GameEngine::AudioManager* audioManager = nullptr;
	GameEngine::DirectXCommon* dxCommon = nullptr;
};