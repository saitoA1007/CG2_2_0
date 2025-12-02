#pragma once
#include"Sprite.h"

class BossHpUI {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	/// <param name="maxHp"></param>
	void Initialize(const uint32_t& maxHp);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	void SetCurrentHp(const uint32_t& hp) { currentHp_ = hp; }

	/// <summary>
	/// 画像データを取得
	/// </summary>
	/// <returns></returns>
	GameEngine::Sprite* GetSprite() { return sprite_.get(); }

private:

	std::unique_ptr<GameEngine::Sprite> sprite_;

	// 最大hp
	uint32_t maxHp_ = 0;

	// 現在のhp
	uint32_t currentHp_ = 0;
};