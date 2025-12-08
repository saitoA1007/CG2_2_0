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
	logoSprite_ = Sprite::Create({ 640.0f, 180.0f }, { 1024.0f, 256.0f }, { 0.5f, 0.5f });
	logoGH_ = textureManager->GetHandleByName("gameOverText.png");

	// やりなおすスプライト
	retrySprite_ = Sprite::Create(retryPos_, retrySize_, { 0.5f, 0.5f });
	retryGH_ = textureManager->GetHandleByName("retryText.png");

	// タイトルにもどるスプライト
	titleSprite_ = Sprite::Create(titlePos_, titleSize_, { 0.5f, 0.5f });
	titleGH_ = textureManager->GetHandleByName("titleBackText.png");
}

void GameOverUI::Update() {
	if (!isActive_) return;

	// スプライトの更新
	if (bgSprite_) bgSprite_->Update();
	if (logoSprite_) logoSprite_->Update();
	if (retrySprite_) retrySprite_->Update();
	if (titleSprite_) titleSprite_->Update();

	// 入力処理（ホバー/クリック判定）
	HandleInput();
}

void GameOverUI::HandleInput() {
	if (!input_) return;

	Vector2 mousePos = input_->GetMousePosition();

	auto isInRect = [&](const Vector2& center, const Vector2& size, const Vector2& p) {
		Vector2 half = { size.x * 0.5f, size.y * 0.5f };
		Vector2 min = { center.x - half.x, center.y - half.y };
		Vector2 max = { center.x + half.x, center.y + half.y };
		return (p.x >= min.x && p.x <= max.x && p.y >= min.y && p.y <= max.y);
	};

	// ホバー判定
	isHoverRetry_ = isInRect(retryPos_, retrySize_, mousePos);
	isHoverTitle_ = isInRect(titlePos_, titleSize_, mousePos);

	// マウス左クリックトリガー
	bool click = input_->TriggerMouse(0);

	if (click) {
		if (isHoverRetry_ && onRetryClicked_) {
			onRetryClicked_();
		}
		if (isHoverTitle_ && onTitleClicked_) {
			onTitleClicked_();
		}
	}
}