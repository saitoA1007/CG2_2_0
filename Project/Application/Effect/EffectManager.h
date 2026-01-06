#pragma once
#include"IEffect.h"
#include"Vector3.h"
#include<string>
#include<memory>
#include<vector>
#include<list>

#include"Object/BreakIceParticle.h"
#include"Object/Player/PlayerAttackEffect.h"
#include"Object/Player/PlayerDestroyEffect.h"
#include"Object/Enemy/EnemyDestroyEffect.h"

class EffectManager {
public:
	EffectManager();

	void Initialize();

	void Update(const Matrix4x4& worldMatrix, const Matrix4x4& viewMatrix);

public:

	// 氷の破壊演出を追加
	void AddBreakIceParticle(const Vector3& pos);

	// プレイヤーの攻撃演出
	void AddPlayerAttackEffect(const Vector3& pos);

	// プレイヤーの撃破演出
	void AddPlayerDestroyEffect(const Vector3& pos);

	// 敵の撃破演出
	void AddEnemyDestroyEffect(const Vector3& pos);

public:

	// 氷の破壊演出を取得
	std::list<std::unique_ptr<BreakIceParticle>>& GetBreakIceEffect() { return breakIcesEffects_; }

	// プレイヤーの攻撃演出
	std::list<std::unique_ptr<PlayerAttackEffect>>& GetPlayerAttackEffect() { return playerAttackEffects_; }

	// プレイヤーの撃破演出
	std::unique_ptr<PlayerDestroyEffect>& GetPlayerDestroyEffect() { return playerDestroyEffect_; }

	// 敵の撃破演出
	std::unique_ptr<EnemyDestroyEffect>& GetEnemyDestroyEffect() { return enemyDestroyEffect_;}

public:

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

	// 氷を破壊する演出
	std::list<std::unique_ptr<BreakIceParticle>> breakIcesEffects_;

	// 攻撃演出
	std::list<std::unique_ptr<PlayerAttackEffect>> playerAttackEffects_;

	// プレイヤーの撃破演出
	std::unique_ptr<PlayerDestroyEffect> playerDestroyEffect_;

	// 敵の撃破演出
	std::unique_ptr<EnemyDestroyEffect> enemyDestroyEffect_;
};