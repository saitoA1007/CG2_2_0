#pragma once
#include <array>
#include<list>

#include"BaseScene.h"

// エンジン機能をインクルード
#include"Camera.h"
#include"Model.h"
#include"WorldTransform.h"
#include"Sprite.h"
#include"DebugRenderer.h"
#include"CollisionManager.h"
#include"Extension/CustomMaterial/IceMaterial.h"
#include"ParticleSystem/ParticleBehavior.h"
#include"Animator.h"

// アプリ機能をインクルード
#include"Application/Player/Player.h"
#include"Application/Camera/CameraController.h"
#include"Application/Light/SceneLightingController.h"
#include"Application/Stage/StageManager.h"
#include"Application/Stage/StageWallPlane.h"
#include"Application/Enemy/BossEnemy.h"
#include"Application/Enemy/EnemyAttackManager.h"
#include"Application/Enemy/Effect/EnemyRushEffect.h"
#include"Application/Enemy/Effect/EnemyWindAttackParticle.h"
#include"Application/Enemy/Effect/WingsParticle.h"
#include"Application/Graphics/Terrain.h"
#include"Application/Graphics/PlaneProjectionShadow.h"
#include"Application/Graphics/BgRock.h"
#include"Application/Scene/Transition/Fade.h"
#include"Application/UI/BossHpUI.h"
#include"Application/UI/PlayerHpUI.h"
#include"Application/UI/GameOverUI.h"
#include"Application/Player/Effect/PlayerChargeEffect.h"
#include"Application/Player/Effect/PlayerRushEffect.h"
#include"Application/Player/Effect/PlayerAttackDownEffect.h"

class TDGameScene : public BaseScene {
public:

	~TDGameScene();

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="input"></param>
	void Initialize(SceneContext* context) override;

	/// <summary>
	/// 更新処理
	/// </summary>
	void Update() override;

	/// <summary>
	/// デバック時、処理して良いものを更新する
	/// </summary>
	void DebugUpdate() override;

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(const bool& isDebugView) override;

	/// <summary>
	/// UIの描画
	/// </summary>
	void DrawUI() override;

	/// <summary>
	/// 終了したことを伝える
	/// </summary>
	/// <returns></returns>
	bool IsFinished() override { return isFinished_; };

	/// <summary>
	/// 次のシーン遷移する場面の値を取得
	/// </summary>
	/// <returns></returns>
	SceneState NextSceneState() override { return SceneState::Result; }

	// 遷移するクラス
	std::unique_ptr<ITransitionEffect> GetTransitionEffect() override { return std::make_unique<Fade>(); }

private: // エンジンの低レイヤー機能を取得

	// デバック描画するリスト
	std::unique_ptr<GameEngine::DebugRenderer> debugRenderer_;
	// デバックの表示管理
	bool isDrawCollision_ = true;

	// 当たり判定の管理
	std::unique_ptr<GameEngine::CollisionManager> collisionManager_;

private:
	//==================================================
	// シーン機能
	//==================================================

	// 終了フラグ
	bool isFinished_ = false;

	// メインカメラ
	std::unique_ptr<GameEngine::Camera> mainCamera_;
	// カメラコントローラークラス
	std::unique_ptr<CameraController> cameraController_;

	// 天球
	GameEngine::Model* skyDomeModel_;
	GameEngine::WorldTransform skyDomeWorldTransform_;

	// 氷の地面を作るためのモデル
	GameEngine::Model* icePlaneModel_;
	// 地面
	std::unique_ptr<Terrain> terrain_;

	// 背景モデル
	GameEngine::Model* bgIceRockModel_;
	GameEngine::Model* bgRockModel_;
	// 背景の岩オブジェクト
	std::unique_ptr<BgRock> bgRock_;

	// ライト
	std::unique_ptr<SceneLightingController> sceneLightingController_;

	//==================================================
	// プレイヤー
	//==================================================

	// 自キャラのモデル
	GameEngine::Model* playerModel_;
	// プレイヤー
	std::unique_ptr<Player> player_;
	// プレイヤーの影
	std::unique_ptr<PlaneProjectionShadow> playerShadow_;
	// プレイヤー用アニメーター
	std::unique_ptr<GameEngine::Animator> playerAnimator_;
    // プレイヤーのアニメーションデータ
    std::array<std::map<std::string, AnimationData>, static_cast<size_t>(kPlayerAnimationCount)> playerAnimationData_;

	// プレイヤーのエフェクトインスタンス
	std::unique_ptr<PlayerChargeEffect> playerChargeEffect_;
	std::unique_ptr<PlayerRushEffect> playerRushEffect_;
	std::unique_ptr<PlayerAttackDownEffect> playerAttackDownEffect_;

	//==================================================
	// ステージ
	//==================================================

	// ステージを生成する
	GameEngine::Model* wallModel_;
	std::unique_ptr<StageManager> stageManager_;

	// StageWallPlane用のモデル
	GameEngine::Model* stageWallPlaneModel_;
	// 板ポリ壁（6つ）
	static inline const size_t kNumStageWalls = 6;
	std::array<StageWallPlane, kNumStageWalls> stageWallPlanes_;
    // ステージ用の板ポリ壁で共有するマテリアル
	std::unique_ptr<IceMaterial> stageWallPlaneMaterial_;

	// プレイヤーの移動範囲を制限するためのコライダー群
	static inline const float kBoundaryHalfHeight = 10000.0f; // 縦方向には到達不可能なほど高く設定
	std::array<std::unique_ptr<GameEngine::OBBCollider>, kNumStageWalls> boundaryColliders_;

	//==================================================
	// ボス
	//==================================================

	// ボス敵モデル
	GameEngine::Model* bossEnemyModel_;
	// ボス敵
	std::unique_ptr<BossEnemy> bossEnemy_;

	// ボスのアニメーションデータ
	std::array<std::map<std::string, AnimationData>, static_cast<size_t>(enemyAnimationType::MaxCount)> enemyAnimationData_;

	// ボスのアニメーションを再生するクラス
	std::unique_ptr<GameEngine::Animator> bossEnemyAnimator_;
	// ボスの影
	std::unique_ptr<PlaneProjectionShadow> bossEnemyShadow_;

	// ボスの遠距離攻撃管理クラス
	std::unique_ptr<EnemyAttackManager> enemyAttackManager_;
	// ボスの突進攻撃演出
	std::unique_ptr<EnemyRushEffect> enemyRushEffect_;
	// ボスの風攻撃演出
	std::unique_ptr<EnemyWindAttackParticle> enemyWindAttackParticle_;
	// 翼の演出
	std::unique_ptr<WingsParticle> enemyWingsParticleParticle_;

	// ボスの纏っているパーティクル
	std::unique_ptr<GameEngine::ParticleBehavior> bossWearParticle_;
	std::unique_ptr<GameEngine::ParticleBehavior> bossWearAdditionParticle_;

	//==================================================
	// モデル
	//==================================================

	// 氷柱のモデル
	GameEngine::Model* iceFallModel_;
	// 破壊した氷柱モデル
	GameEngine::Model* breakIceFallModel_;
	// 敵の突進攻撃演出のモデル
	GameEngine::Model* enemyRushModel_;
	// 翼のモデル
	GameEngine::Model* wingModel_;
	
	// 平面モデル
	GameEngine::Model* planeModel_;
	// 風の演出用モデル
	GameEngine::Model* windModel_;

	// プレイヤー用エフェクトのモデル（EnemyRushEffectと同じモデルを使用）
	GameEngine::Model* playerChargeEffectModel_ = nullptr;
	GameEngine::Model* playerRushEffectModel_ = nullptr;
	GameEngine::Model* playerAttackDownEffectModel_ = nullptr;

	// ボスロックオンフラグ
    bool isBossLockOn_ = false;

	// 空気を演出するためのパーティクル
	std::unique_ptr<GameEngine::ParticleBehavior> airParticle_;

	// 仮の操作方法UI
	std::unique_ptr<GameEngine::Sprite> playGuideSprite_;
	uint32_t playGuideGH_ = 0;

	// ボスのHpを表示
	std::unique_ptr<BossHpUI> bossHpUI_;
	// プレイヤーのHpを表示
	std::unique_ptr<PlayerHpUI> playerHpUI_;

	// ゲームオーバーUI
	std::unique_ptr<GameOverUI> gameOverUI_;

private:

	// ボスヒット時にほとんどの更新を停止するための制御
	bool isBossHitFreezeActive_ = false;
	float bossHitFreezeTimer_ = 0.0f;
	static inline const float kBossHitFreezeDuration = 0.25f; // 秒
	// ボスのヒット状態の前フレーム保存（ヒット瞬間の検出用）
	bool prevBossHit_ = false;

	/// <summary>
	/// 入力のコマンドを設定する
	/// </summary>
	void InputRegisterCommand();

	/// <summary>
	/// 当たり判定の更新処理
	/// </summary>
	void UpdateCollision();
};