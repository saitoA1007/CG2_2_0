#pragma once
#include"IBossState.h"
#include"Application/Enemy/BossContext.h"

class BossStateIn : public IBossState {
public:
    BossStateIn(BossContext& context);

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