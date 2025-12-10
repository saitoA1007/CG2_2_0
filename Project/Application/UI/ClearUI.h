#pragma once
#include<functional>
#include <array>
#include <chrono>
#include"Sprite.h"
#include "TextureManager.h"
#include "InputCommand.h"

class ClearUI {
public:

	// 時間を表示するためのデータ
	struct TimeData {
		std::unique_ptr<GameEngine::Sprite> numSprite_;
		uint32_t number = 0;
	};


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

	GameEngine::Sprite* GetnumDottoSprite() { return numMiddle_.get(); }

	GameEngine::Sprite* GetClearTimeTextSprite() { return clearTimeText_.get(); }

	// コールバック
	void SetBackTitle(std::function<void()> i) { backTitle_ = std::move(i); }

	// 有効フラグ
	void SetIsActice(const bool& isActive) { isActive_ = isActive; }
	bool IsActive() const { return isActive_; }

	uint32_t GetClearTexture() const { return clearTextGH_; }
	uint32_t GetGuidTexture() const { return guideGH_; }
	uint32_t GetDottoTexture() const { return numMiddleGH_; }
	uint32_t GetClearTImeTextTexture() const { return clearTimeTextGH_; }

	// 秒数に応じて時間のスプライトを表示する
	void ShowTimeSprites(int seconds);

	// スプライトのアニメーション用（実装無し)
	void Animate();

	void StartTime();

	// 表示する桁
	std::array<TimeData, 4>& GetNumberSprite() { return numbersSprites_; }

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
	//std::array<std::unique_ptr<GameEngine::Sprite>, 10> clearTimeNumSprites_{};
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

	// 最初の時間
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime_;
	
	// 最後の時間
	std::chrono::time_point<std::chrono::high_resolution_clock> endTime_;

	// 経過時間を保存
	std::chrono::duration<float> duration_;

	// 番号のテクスチャ
	uint32_t numbersGH_[10];

	// 表示する桁
	std::array<TimeData, 4> numbersSprites_;

	// クリア時間UI
	std::unique_ptr<GameEngine::Sprite> clearTimeText_;
	uint32_t clearTimeTextGH_ = 0;

	// 桁の中間
	std::unique_ptr<GameEngine::Sprite> numMiddle_;
	uint32_t numMiddleGH_ = 0;

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