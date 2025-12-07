#include "GameOverUI.h"
#include "SpriteRenderer.h"
#include "GameParamEditor.h"

using namespace GameEngine;

void GameOverUI::Initialize(Input* input, InputCommand* inputCommand, TextureManager* textureManager) {
	input_ = input;
	inputCommand_ = inputCommand;

	// 背景スプライト（画面全体）
	bgSprite_ = Sprite::Create({ 640.0f, 360.0f }, { 1280.0f, 720.0f }, { 0.5f, 0.5f },
		{ 0.0f, 0.0f, 0.0f, 0.75f });

	// ロゴスプライト
	logoSprite_ = Sprite::Create({ 640.0f, 180.0f }, { 600.0f, 128.0f }, { 0.5f, 0.5f });
	logoGH_ = textureManager->GetHandleByName("gameOverLogo.png");

	// やりなおすスプライト
	retrySprite_ = Sprite::Create({ 640.0f, 360.0f }, { 400.0f, 48.0f }, { 0.5f, 0.5f });
	retryGH_ = textureManager->GetHandleByName("retryText.png");

	// タイトルにもどるスプライト
	titleSprite_ = Sprite::Create({ 640.0f, 440.0f }, { 400.0f, 48.0f }, { 0.5f, 0.5f });
	titleGH_ = textureManager->GetHandleByName("titleBackText.png");
}

void GameOverUI::Update() {
	if (bgSprite_) bgSprite_->Update();
	if (logoSprite_) logoSprite_->Update();
	if (retrySprite_) retrySprite_->Update();
	if (titleSprite_) titleSprite_->Update();
}