#pragma once
#include"IBossState.h"
#include"Application/Enemy/BossContext.h"

class BossStateOut : public IBossState {
public:

	enum class Phase {
		In,
		Fade
	};

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

	uint32_t cout_ = 0; 

	float timer_ = 0.0f;
	float InmaxTime_ = 4.0f;
	float FadeMaxTime_ = 2.0f;

	bool isSet_ = false;
	bool isActive_ = false;

	// 高さ
	float startPosY_ = 0.0f;
	float endPosY_ = 1.0f;

	// 左右の触れば
	float swaySpeed_ = 0.0f;
	float swayPhase_ = 0.0f;
	float swayWeithX_ = 10.0f;
	float swayWeithZ_ = 5.0f; 
	float cycleHeight_ = 2.0f;

	std::string kGroupName = "BossStateOut";

	Phase phase_ = Phase::In;

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