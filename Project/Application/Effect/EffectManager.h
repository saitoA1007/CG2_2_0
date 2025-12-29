#pragma once
#include"IEffect.h"
#include"Vector3.h"
#include<string>
#include<memory>
#include<vector>

class EffectManager {
public:
	EffectManager();

	void Initialize();

	void Update();

	// パーティクル演出を追加
	void PlayParticle(const std::string& name, const Vector3& pos);

	const std::vector<std::unique_ptr<IEffect>>& GetEffects() const { return effects_; }

	// 指定した演出グループを取得
	std::vector<IEffect*> GetEffectsByType(EffectType type) const;

	// 演出を削除
	void Clear();

private:

	// 存在している演出
	std::vector<std::unique_ptr<IEffect>> effects_;
};