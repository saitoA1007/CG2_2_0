#pragma once
#include<functional>
#include <array>
#include"Sprite.h"
#include "TextureManager.h"
#include "InputCommand.h"

class ClearUI {
public:

	void Initialize(GameEngine::InputCommand* inputCommand, GameEngine::TextureManager* textureManager);

	void Update();

	// クリア文字
	GameEngine::Sprite* GetClearSprite() { return clearTextSprite_.get(); }
    // クリア文字の背面黒帯
    GameEngine::Sprite *GetClearBackSprite() { return clearTextBackSprite_.get(); }
	// 操作説明
	GameEngine::Sprite* GetGuideSprite() { return guideSprite_.get(); }
	// 背景
	GameEngine::Sprite* GetBgSprite() { return bgSprite_.get(); }

	// コールバック
	void SetBackTitle(std::function<void()> i) { backTitle_ = std::move(i); }

	// 有効フラグ
	void SetIsActice(const bool& isActive) { isActive_ = isActive; }
	bool IsActive() const { return isActive_; }

	uint32_t GetClearTexture() const { return clearTextGH_; }
	uint32_t GetGuidTexture() const { return guideGH_; }

	// 秒数に応じて時間のスプライトを表示する
	void ShowTimeSprites(int seconds);

	// スプライトのアニメーション用（実装無し)
	void Animate();

private:
	// 入力処理
	GameEngine::InputCommand* inputCommand_ = nullptr;

	// クリア文字
	std::unique_ptr<GameEngine::Sprite> clearTextSprite_;
	uint32_t clearTextGH_ = 0;

	// クリア文字の背面に敷く黒帯
	std::unique_ptr<GameEngine::Sprite> clearTextBackSprite_;

	// 操作UI
	std::unique_ptr<GameEngine::Sprite> guideSprite_;
	uint32_t guideGH_ = 0;

	// 背景画像
	std::unique_ptr<GameEngine::Sprite> bgSprite_;

	// 0〜9の数字スプライト
	std::array<std::unique_ptr<GameEngine::Sprite>, 10> clearTimeNumSprites_{};
	// クリア時間（秒)
	int clearSeconds_ = 0;

	bool isActive_ = false;
	bool prevIsActive_ = false;

	// アニメーションタイマー
	float animTimer_ = 0.0f;
	float animTotal_ = 4.0f;

	// 選択音声
	uint32_t selectSH_ = 0;

	// 戻る
	std::function<void()> backTitle_ = nullptr;

	// デバック用
	std::string kGroupName_ = "ClearUI";

private:

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();

};