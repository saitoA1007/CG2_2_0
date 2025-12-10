#pragma once
#include "Sprite.h"
#include "Input.h"
#include "InputCommand.h"
#include "TextureManager.h"
#include <functional>

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

	// 有効/無効
	bool IsActive() const { return isActive_; }
	void SetActive(bool active); // 有効化時にアニメーション再生

	// 入力処理（Updateから呼ぶ）
	void HandleInput();

	// クリック時のコールバック
	void SetOnRetryClicked(std::function<void()> cb) { onRetryClicked_ = std::move(cb); }
	void SetOnTitleClicked(std::function<void()> cb) { onTitleClicked_ = std::move(cb); }

	// UI有効化時アニメーション開始
	void StartActivateAnimation();

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

	Vector2 retryPos_ = {640.0f, 360.0f};
	Vector2 retrySize_ = {375.0f, 80.0f};
	Vector2 titlePos_ = {640.0f, 488.0f};
	Vector2 titleSize_ = {500.0f, 80.0f};

	// ホバー判定フラグ（毎フレーム計算）
	bool isHoverRetry_ = false;
	bool isHoverTitle_ = false;

	// マウス以外の選択状態（保持用）
	bool kbSelectRetry_ = true;
	bool kbSelectTitle_ = false;

	// クリックコールバック
	std::function<void()> onRetryClicked_ = nullptr;
	std::function<void()> onTitleClicked_ = nullptr;

	// 有効化アニメーション管理
	bool isActivating_ = false;
	float activateAnimTimer_ = 0.0f;
	float activateAnimDuration_ = 1.0f; // 全体アニメ長さ

	void UpdateActivateAnimation();
};