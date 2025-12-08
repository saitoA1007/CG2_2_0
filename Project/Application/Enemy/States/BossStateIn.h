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


	std::string kGroupName = "BossInMove";
	Vector3 startPos_ = {0.0f,0.0f,0.0f};
	Vector3 endPos_ = {0.0f,15.0f,0.0f};
	float moveHeight_ = 4.0f;
	uint32_t cycleCount_ = 3;

	float timer_ = 0.0f;

	float InTime_ = 2.0f;
	float waitTime_ = 2.0f;

	bool isMove_ = true;

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