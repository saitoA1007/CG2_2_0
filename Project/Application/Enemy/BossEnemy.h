#pragma once
#include<array>
#include<functional>

#include"Collider.h"
#include"DebugRenderer.h"
#include"Animator.h"
#include"Extension/CustomMaterial/BossMaterial.h"

#include"BossContext.h"
#include"States/IBossState.h"

#include"EnemyAttackManager.h"

class BossEnemy : public GameEngine::GameObject {
public:

	/// <summary>
	/// 初期化処理
	/// </summary>
	void Initialize(const float& stageRadius, EnemyAttackManager* enemyAttackManager, GameEngine::Animator* animator,
		std::array<std::map<std::string, AnimationData>, static_cast<size_t>(enemyAnimationType::MaxCount)>* animationData, GameEngine::DebugRenderer* debugRenderer);

	// テクスチャを設定する
	void SetTexture(const uint32_t& bossTex, const uint32_t& noiseTex) {
		bossMaterial_->materialData_->textureHandle = bossTex;
		bossMaterial_->materialData_->noiseTexture = noiseTex;
	}

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update(const Vector3& targetPos);

	/// <summary>
	/// ワールド行列を取得
	/// </summary>
	/// <returns></returns>
	GameEngine::WorldTransform& GetWorldTransform() { return worldTransform_; }

	/// <summary>
	/// コライダー取得
	/// </summary>
	GameEngine::Collider* GetCollider() { return bodyCollider_.get(); }

	// マテリアルを取得
	BossMaterial* GetMaterial() { return bossMaterial_.get(); };

	// 当たり判定の球データ
	Sphere GetSphereData();

	// ワールド座標
	Vector3 GetWorldPosition() { return worldTransform_.GetWorldPosition(); }

	/// <summary>
	/// 突進攻撃の演出
	/// </summary>
	/// <returns></returns>
	bool IsRushAttack() const { return bossContext_.isRushAttack_; }
	// 突進の時の速度
	Vector3 GetRushVelocity() const { return bossContext_.rushVelocity; }

	// 突進攻撃の当たり判定の有効フラグ
	bool IsRushCollisionActive() const { return bossContext_.isRushCollisionActive_; }

	// 現在のhpを取得
	int32_t GetCurrentHP() const { return bossContext_.hp; }
	// 最大hp
	int32_t GetMaxHp() const { return kMaxHp_; }

	// 透明度
	float GetAlpha() const {return alpha_;}

	// 当たり判定フラグ
	bool IsHit() const { return isHit_; }

	// 翼のアニメーション
	bool IsWingsEffect() const { return bossContext_.isWingsEffect_; }

	// 破壊演出
	bool IsDestroyEffect() const { return bossContext_.isDestroyEffect; }

	// ボスの全ての処理が終了
	bool isFinished() const { return bossContext_.isFinished_; }

	// 終了フェーズに入った時に位置をリセット出来るタイミングを取得する
	void SetResetPosition() {
		bossContext_.isResetPos = true;
	}

	// ボスの状態をInに設定
    void SetBossStateIn() {
		bossContext_.bossStateRequest_ = BossState::In;
    }

	// 現在の状態を取得する
	BossState GetBossState() const { return bossState_; }

	// 入りの翼演出フラグ
	bool IsEnterWingsEffect() const { return bossContext_.isEnterWingsEffect_; }
	bool IsEnterBurstWingsEffect() const { return bossContext_.isEnterBurstWingsEffect_; }

	bool IsStopBGm() const { return bossContext_.isStopBgm_; }

private: // 調整項目
	
	// 最大体力
	int32_t kMaxHp_ = 2;

	// サイズ
	float scale_ = 1.0f;

	// ヒットクールタイム
	float hitCoolTime_ = 2.0f;

private: // メンバ変数

	// 遠距離などの攻撃を管理
	EnemyAttackManager* enemyAttackManager_ = nullptr;

	// ワールド行列
	GameEngine::WorldTransform worldTransform_;

	// ボスの共通パラメータ
	BossContext bossContext_;

	// 行動状態を保存
	std::array<std::unique_ptr<IBossState>, static_cast<size_t>(BossState::MaxCount)> statesTable_;
	// 現在の状態
	IBossState* currentState_ = nullptr;

	// 両翼の当たり判定も追加する場合はstd::arrayにしてenumで管理する
	// 体の当たり判定
	std::unique_ptr<GameEngine::SphereCollider> bodyCollider_;

	float bodyColliderSize_ = 3.0f;

	// デバック用のグループ名
	std::string kGroupName_ = "Boss";

	std::string MatName_ = "BossMaterial";

	// 当たり判定
	bool isHit_ = false;

	// 時間
	float timer_ = 1.0f;

	// 当たり判定に使用する透明度
	float alpha_ = 1.0f;

	// 現在の状態
	BossState bossState_ = BossState::In;

	// 音声ハンドル
	uint32_t bossDamagedSH_ = 0; // ダメージ音声
    uint32_t attackHitSH_ = 0;   // 攻撃が当たった音声

	// ボスのマテリアル
	std::unique_ptr<BossMaterial> bossMaterial_;

private:

	/// <summary>
	/// 当たり判定
	/// </summary>
	/// <param name="result"></param>
	void OnCollisionEnter([[maybe_unused]] const GameEngine::CollisionResult& result);

	/// <summary>
	/// 値を登録する
	/// </summary>
	void RegisterBebugParam();

	/// <summary>
	/// 値を適応する
	/// </summary>
	void ApplyDebugParam();
};