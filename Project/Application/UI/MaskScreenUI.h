#pragma once
#include"InputCommand.h"
#include"Sprite.h"

class MaskScreenUI {
public:

	void Initialize(GameEngine::InputCommand* inputCommand);

	void Update();

	// 有効か
	bool IsActive() const { return isActive_; }

	// マスク状態か
	bool IsMask()const { return isMask_; }

	// 画像データ
	GameEngine::Sprite* GetUpSprite() { return upMaskSprite_.get(); }
	GameEngine::Sprite* GetDownSprite() { return downMaskSprite_.get(); }

private:

	GameEngine::InputCommand* inputCommand_ = nullptr;

	// 上のマスク画像
	std::unique_ptr<GameEngine::Sprite> upMaskSprite_;
	// 下のマスク画像
	std::unique_ptr<GameEngine::Sprite> downMaskSprite_;

	bool isActive_ = false;

	bool isMask_ = false;

	// マスクの大きさ
	float maskSize_ = 48.0f;

	// タイマー
	float timer_ = 0.0f;
	float maxTime_ = 0.5f;

	float upStartPosY_ = 0.0f;
	float upEndPosY_ = 0.0f;

	float downStartPosY_ = 0.0f;
	float downEndPosY_ = 0.0f;
};