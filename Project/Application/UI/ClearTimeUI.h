#pragma once
#include <array>
#include"Sprite.h"
#include"TextureManager.h"
#include"Application/ClearTime.h"

class ClearTimeUI {
public:

	// 時間を表示するためのデータ
	struct TimeData {
		std::unique_ptr<GameEngine::Sprite> numSprite_;
		uint32_t number = 0;
	};

public:

	ClearTimeUI(GameEngine::TextureManager* textureManager);

	// 表示する桁
	std::array<TimeData, static_cast<size_t>(ClearTime::Digit::MaxCount)>& GetNumberSprite() { return numbersSprites_; }

	// 桁の中間
	GameEngine::Sprite* GetnumDottoSprite() { return numMiddle_.get(); }
	uint32_t GetDottoTexture() const { return numMiddleGH_; }

private:

	// 番号のテクスチャ
	uint32_t numbersGH_[10];

	// 表示する桁
	std::array<TimeData, static_cast<size_t>(ClearTime::Digit::MaxCount)> numbersSprites_;

	// 桁の中間
	std::unique_ptr<GameEngine::Sprite> numMiddle_;
	uint32_t numMiddleGH_ = 0;

	Vector2 basePos_ = {640.0f,420.0f};
};