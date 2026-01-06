#include"EffectManager.h"

EffectManager::EffectManager() {
	// メモリを先に確保しておく
	effects_.resize(32);
}

void EffectManager::Initialize() {
    // プレイヤーの撃破演出を初期化
    playerDestroyEffect_ = std::make_unique<PlayerDestroyEffect>();
    playerDestroyEffect_->Initialize();
}

void EffectManager::Update(const Matrix4x4& worldMatrix, const Matrix4x4& viewMatrix) {

    breakIcesEffects_.remove_if([](const std::unique_ptr<BreakIceParticle>& breakIceParticle) {
        return breakIceParticle->IsFinished();
    });

    // 氷の破壊演出
    for (auto& breakIceFallParticle : breakIcesEffects_) {
        breakIceFallParticle->Update();
    }

    playerAttackEffects_.remove_if([](const std::unique_ptr<PlayerAttackEffect>& effect) {
        return effect->IsFinished();
        });

    // プレイヤーの攻撃演出
    for (auto& effect : playerAttackEffects_) {
        effect->Update(worldMatrix, viewMatrix);
    }

    // プレイヤーの撃破演出
    playerDestroyEffect_->Update(worldMatrix, viewMatrix);
}

void EffectManager::AddBreakIceParticle(const Vector3& pos) {
    std::unique_ptr<BreakIceParticle> tmp = std::make_unique<BreakIceParticle>();
    tmp->Initialize(0,pos);
    breakIcesEffects_.push_back(std::move(tmp));
}

void EffectManager::AddPlayerAttackEffect(const Vector3& pos) {
    std::unique_ptr<PlayerAttackEffect> tmp = std::make_unique<PlayerAttackEffect>();
    tmp->Initialize(pos);
    playerAttackEffects_.push_back(std::move(tmp));
}

void EffectManager::AddPlayerDestroyEffect(const Vector3& pos) {
    // 演出を発生
    playerDestroyEffect_->SetEmitPos(pos);
}

void EffectManager::PlayParticle(const std::string& name, const Vector3& pos) {
	name;
	pos;
}

std::vector<IEffect*> EffectManager::GetEffectsByType(EffectType type) const {
    std::vector<IEffect*> result;

    for (const auto& effect : effects_) {
        if (!effect->IsFinished() && effect->GetType() == type) {
            result.push_back(effect.get());
        }
    }

    return result;
}

void EffectManager::Clear() {

}