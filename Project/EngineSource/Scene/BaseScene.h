#pragma once
#include"SceneState.h"
#include"SceneContext.h"
#include"ITransitionEffect.h"

/// <summary>
/// 各シーンの元
/// </summary>
class BaseScene {
public:

	/// <summary>
	/// デストラクタ
	/// </summary>
	virtual ~BaseScene() = default;

	/// <summary>
	/// 初期化処理
	/// </summary>
	virtual void Initialize(SceneContext* context) = 0;

	/// <summary>
	/// 更新処理
	/// </summary>
	virtual void Update() = 0;

	/// <summary>
	/// デバック時、処理して良いものを更新する
	/// </summary>
	virtual void DebugUpdate() = 0;

	/// <summary>
	/// 描画処理
	/// </summary>
	virtual void Draw(const bool& isDebugView) = 0;

	// UIの描画
	virtual void DrawUI() = 0;

	/// <summary>
	/// 終了したことを伝える
	/// </summary>
	/// <returns></returns>
	virtual bool IsFinished() = 0;

	/// <summary>
	/// 次のシーン遷移する場面の値を取得
	/// </summary>
	/// <returns></returns>
	virtual SceneState NextSceneState() = 0;

	// 遷移クラス
	virtual std::unique_ptr<ITransitionEffect> GetTransitionEffect() = 0;

protected:
	// エンジン機能
	SceneContext* context_ = nullptr;
};