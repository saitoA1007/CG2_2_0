#pragma once
#include <functional>

#include"InputCommand.h"
#include"TextureManager.h"
#include"Sprite.h"

class GameOverUI {
public:

	void Initialize(GameEngine::InputCommand* inputCommand, GameEngine::TextureManager* textureManager);

	void Update();

	// スプライト取得
	GameEngine::Sprite* GetLogoSprite() { return logoSprite_.get(); }
	GameEngine::Sprite* GetRetrySprite() { return retrySprite_.get(); }
	GameEngine::Sprite* GetTitleSprite() { return titleSprite_.get(); }
	GameEngine::Sprite* GetBgSprite() { return bgSprite_.get(); }
	// テクスチャハンドル取得
	uint32_t GetBgGH() const { return bgGH_; }
	uint32_t GetLogoGH() const { return logoGH_; }
	uint32_t GetRetryGH() const { return retryGH_; }
	uint32_t GetTitleGH() const { return titleGH_; }

	void SetRetry(std::function<void()> f) { onRetry_ = std::move(f); }
	void SetTitle(std::function<void()> f) { onTitle_ = std::move(f); }

private:
	GameEngine::InputCommand* inputCommand_ = nullptr;

	// 関数
	std::function<void()> onRetry_ = nullptr;
	std::function<void()> onTitle_ = nullptr;

	int32_t selectNum_ = 0;

	// 背景画像
	std::unique_ptr<GameEngine::Sprite> bgSprite_;
	uint32_t bgGH_ = 0;

	// ゲームオーバー文字
	std::unique_ptr<GameEngine::Sprite> logoSprite_;
	uint32_t logoGH_ = 0;

	// リトライ文字
	std::unique_ptr<GameEngine::Sprite> retrySprite_;
	uint32_t retryGH_ = 0;

	// タイトル文字
	std::unique_ptr<GameEngine::Sprite> titleSprite_;
	uint32_t titleGH_ = 0;

private:

	/// <summary>
	/// 入力処理
	/// </summary>
	void ProcessInput();
};