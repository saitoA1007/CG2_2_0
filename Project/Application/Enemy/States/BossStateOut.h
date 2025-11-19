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

};