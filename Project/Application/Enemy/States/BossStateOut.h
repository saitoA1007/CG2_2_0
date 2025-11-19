#pragma once
#include"IBossState.h"
#include"Application/Enemy/BossContext.h"

class BossStateOut : public IBossState {
public:
    BossStateOut(BossContext& context);

    void Enter() override;
    void Update() override;
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