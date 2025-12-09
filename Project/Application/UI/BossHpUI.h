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
	void Initialize(const int32_t& maxHp);

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

public:

	void SetCurrentHp(const int32_t& hp) { 
		currentHp_ = hp;
		if (currentHp_ <= 0) {
			currentHp_ = 0;
		}
	}

	/// <summary>
	/// 画像データを取得
	/// </summary>
	/// <returns></returns>
	GameEngine::Sprite* GetSprite() { return sprite_.get(); }

	// 演出用の画像データを取得
	GameEngine::Sprite* GetEffectSprite() { return effectSprite_.get(); }

	GameEngine::Sprite* GetFrameSprite() { return frameSprite_.get(); }

	GameEngine::Sprite* GetNameSprite() { return bossNameSprite_.get(); }

private:

	std::unique_ptr<GameEngine::Sprite> sprite_;

	// hpの演出用
	std::unique_ptr<GameEngine::Sprite> effectSprite_;

	// フレーム
	std::unique_ptr<GameEngine::Sprite> frameSprite_;

	// ボスの名前
	std::unique_ptr<GameEngine::Sprite> bossNameSprite_;

	float preScaleX_ = 1.0f;

	// 最大hp
	int32_t maxHp_ = 0;

	// 現在のhp
	int32_t currentHp_ = 0;

	// 演出用のhpゲージが移動する位置
	std::list<Point> points_;

	float maxTime_ = 0.5f;

	// デバック用
	Vector2 size_ = { 640.0f,48.0f };
	Vector2 position_ = { 320.0f,32.0f };

	Vector2 bossNameSize_ = {};
	Vector2 bossNamePosition_ = {};
	
	std::string kGroupName_ = "BossHpUI";

private:

	/// <summary>
	/// 演出の更新処理
	/// </summary>
	void EffectUpdate();

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};