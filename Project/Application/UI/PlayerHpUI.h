#pragma once
#include<vector>
#include"Sprite.h"
#include "TextureManager.h"

class PlayerHpUI {
public:

	void Initialize(const int32_t& maxHp, GameEngine::TextureManager* textureManager);

	void Update();

public:

	void SetCurrentHp(const int32_t& hp) { currentHp_ = hp; }

	// hp
	const std::vector<std::unique_ptr<GameEngine::Sprite>>& GetHpSprites() const { return hpSprites_; }
	// hp画像
	uint32_t GetHpGH() const { return hpGH_; }

private:

	// スプライト
	std::vector<std::unique_ptr<GameEngine::Sprite>> hpSprites_;
	// HPの画像
	uint32_t hpGH_ = 0;

	// 最大hp
	int32_t maxHp_ = 0;

	// 現在のhp
	int32_t currentHp_ = 0;

	// 配置する基準の位置
	Vector2 startPos_ = { 32.0f, 656.0f };

	// 画像サイズ
	Vector2 spriteSize_ = { 64.0f, 64.0f };
	// 間隔
	float spacing_ = 4.0f;
};