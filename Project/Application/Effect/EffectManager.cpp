#include"EffectManager.h"

EffectManager::EffectManager() {
	// メモリを先に確保しておく
	effects_.resize(32);
}

void EffectManager::Initialize() {

}

void EffectManager::Update() {

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