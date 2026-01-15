#pragma once
#include"IBossState.h"
#include"Application/Enemy/BossContext.h"

class BossStateOut : public IBossState {
public:

	BossStateOut(BossContext& context);

	/// <summary>
	/// 入りの処理
	/// </summary>
	void Enter() override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// 終わりの処理
	/// </summary>
	void Exit() override;

private:
	// ボスの共通データを取得
	BossContext& bossContext_;

	std::string kGroupName = "BossStateOut";

	float timer_ = 0.0f;

	float maxTime_ = 1.0f;

private:

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterDebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};