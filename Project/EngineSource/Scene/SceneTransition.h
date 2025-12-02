#pragma once
#include"Sprite.h"

// シーン遷移の演出タイプ
enum class TransitionType {
	Fade,        // フェード
};

class SceneTransition {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize();

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update();

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw();

public:

	/// <summary>
	/// トランジション開始
	/// </summary>
	/// <param name="type">演出タイプ</param>
	/// <param name="duration">演出時間（秒）</param>
	void Start(TransitionType type, float maxTime);

	/// <summary>
	/// トランジション中かどうか
	/// </summary>
	bool IsActive() const { return isActive_; }

	/// <summary>
	/// フェードアウトが完了したか
	/// </summary>
	bool IsMidTransition() const { return isActive_ && timer_ >= maxTime_ * 0.5f; }

private:
	// トランジションが有効か
	bool isActive_ = false;

	// 演出タイプ
	TransitionType type_ = TransitionType::Fade;

	// 経過時間
	float timer_ = 0.0f;

	// 演出時間
	float maxTime_ = 1.0f;

	// 画像データ
	std::unique_ptr<GameEngine::Sprite> sprite_;

private:

	/// <summary>
	/// フェードの更新処理
	/// </summary>
	void Fade();

};