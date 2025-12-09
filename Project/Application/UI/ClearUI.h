#pragma once
#include<functional>
#include"Sprite.h"
#include "TextureManager.h"
#include "InputCommand.h"

class ClearUI {
public:

	void Initialize(GameEngine::InputCommand* inputCommand, GameEngine::TextureManager* textureManager);

	void Update();

	// クリア文字
	GameEngine::Sprite* GetClearSprite() { return clearTextSprite_.get(); }
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

private:
	// 入力処理
	GameEngine::InputCommand* inputCommand_ = nullptr;

	// クリア文字
	std::unique_ptr<GameEngine::Sprite> clearTextSprite_;
	uint32_t clearTextGH_ = 0;

	// 操作UI
	std::unique_ptr<GameEngine::Sprite> guideSprite_;
	uint32_t guideGH_ = 0;

	// 背景画像
	std::unique_ptr<GameEngine::Sprite> bgSprite_;

	bool isActive_ = false;

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