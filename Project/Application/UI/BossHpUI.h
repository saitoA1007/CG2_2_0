#pragma once
#include<list>
#include"Sprite.h"

class BossHpUI {
public:

	struct Point {
		float startScale;
		float endScale;
		float timer = 0.0f;
	};

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

public:

	void SetCurrentHp(const uint32_t& hp) { currentHp_ = hp; }

	/// <summary>
	/// 画像データを取得
	/// </summary>
	/// <returns></returns>
	GameEngine::Sprite* GetSprite() { return sprite_.get(); }

	// 演出用の画像データを取得
	GameEngine::Sprite* GetEffectSprite() { return effectSprite_.get(); }

	// フレーム
	GameEngine::Sprite* GetFrameSprite() { return frameSprite_.get(); }

private:

	std::unique_ptr<GameEngine::Sprite> sprite_;

	// hpの演出用
	std::unique_ptr<GameEngine::Sprite> effectSprite_;

	// フレーム
	std::unique_ptr<GameEngine::Sprite> frameSprite_;

	float preScaleX_ = 1.0f;

	// 最大hp
	uint32_t maxHp_ = 0;

	// 現在のhp
	uint32_t currentHp_ = 0;

	// 演出用のhpゲージが移動する位置
	std::list<Point> points_;

	float maxTime_ = 0.5f;

	Vector2 size_ = { 640.0f,24.0f };
	Vector2 position_ = { 320.0f,32.0f };

private:

	/// <summary>
	/// 演出の更新処理
	/// </summary>
	void EffectUpdate();
};