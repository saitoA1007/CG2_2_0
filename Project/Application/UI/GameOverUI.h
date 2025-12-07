#pragma once
#include "Sprite.h"
#include "Input.h"
#include "InputCommand.h"
#include "TextureManager.h"

class GameOverUI {
public:
	GameOverUI() = default;
	~GameOverUI() = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="input">入力ポインタ</param>
	/// <param name="inputCommand">入力コマンドポインタ</param>
	/// <param name="textureManager">テクスチャマネージャ</param>
	void Initialize(GameEngine::Input* input, GameEngine::InputCommand* inputCommand, GameEngine::TextureManager* textureManager);

	/// <summary>
	/// 更新処理
	/// </summary>
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

    // アクティブ状態取得・設定
    bool IsActive() const { return isActive_; }

private:
	// 入力関連
	GameEngine::Input* input_ = nullptr;
	GameEngine::InputCommand* inputCommand_ = nullptr;

	// 背景スプライト
	std::unique_ptr<GameEngine::Sprite> bgSprite_;
	uint32_t bgGH_ = 0;

	// ゲームオーバーロゴスプライト
	std::unique_ptr<GameEngine::Sprite> logoSprite_;
	uint32_t logoGH_ = 0;

	// やりなおすテキストスプライト
	std::unique_ptr<GameEngine::Sprite> retrySprite_;
	uint32_t retryGH_ = 0;

	// タイトルにもどるテキストスプライト
	std::unique_ptr<GameEngine::Sprite> titleSprite_;
	uint32_t titleGH_ = 0;

	// 内部状態
	bool isActive_ = false;
};