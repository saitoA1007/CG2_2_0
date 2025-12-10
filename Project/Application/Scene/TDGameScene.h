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
#include"Application/Player/Effect/PlayerAttackEffect.h"
#include"Application/Player/Effect/PlayerLandingEffect.h"
#include"Application/UI/Letterbox.h"
#include"Application/UI/ClearUI.h"
#include"Application/UI/BossDestroyFade.h"

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
	SceneState NextSceneState() override { return nextSceneState_; }

	// 遷移するクラス
	std::unique_ptr<ITransitionEffect> GetTransitionEffect() override { return std::make_unique<Fade>(); }

    // 初のゲーム開始かどうかの設定
	static void SetIsFirstGameStart(bool isFirst) { sIsFirstGameStart_ = isFirst; }
    static bool IsFirstGameStart() { return sIsFirstGameStart_; }

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

    // 次に遷移するシーン
    SceneState nextSceneState_ = SceneState::TDGame;

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
	std::unique_ptr<PlayerAttackEffect> playerAttackEffect_;
	std::unique_ptr<PlayerLandingEffect> playerLandingEffect_;

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
	// ボス卵モデル
	GameEngine::Model* bossEggModel_ = nullptr;
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
	std::unique_ptr<EnemyWindAttackParticle> enemyWindShadowAttackParticle_;
	// 翼の演出
	std::unique_ptr<WingsParticle> enemyWingsParticleParticle_;

	// ボスの纏っているパーティクル
	std::unique_ptr<GameEngine::ParticleBehavior> bossWearParticle_;
	std::unique_ptr<GameEngine::ParticleBehavior> bossWearAdditionParticle_;

	// ボスが撃破された時のフェード
	std::unique_ptr<BossDestroyFade> bossDestroyFade_;
	bool isBossDestroyFade_ = false;

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

	// 回復用モデル
	GameEngine::Model* heartModel_;

	// プレイヤー用エフェクトのモデル（EnemyRushEffectと同じモデルを使用）
	GameEngine::Model* playerChargeEffectModel_ = nullptr;
	GameEngine::Model* playerRushEffectModel_ = nullptr;
	GameEngine::Model* playerAttackDownEffectModel_ = nullptr;
	GameEngine::Model* playerLandingEffectModel_ = nullptr;

	// ボスロックオンフラグ
    bool isBossLockOn_ = false;

	// 空気を演出するためのパーティクル
	std::unique_ptr<GameEngine::ParticleBehavior> airParticle_;

	// 仮の操作方法UI
	std::unique_ptr<GameEngine::Sprite> playGuideSprite_;
	uint32_t playGuideGH_ = 0;

	// ボスのHpを表示
	std::unique_ptr<BossHpUI> bossHpUI_;
	uint32_t bossNameGH_ = 0;
	// プレイヤーのHpを表示
	std::unique_ptr<PlayerHpUI> playerHpUI_;

	// ゲームオーバーUI
	std::unique_ptr<GameOverUI> gameOverUI_;

    // 初のゲーム開始かどうかのフラグ
    static inline bool sIsFirstGameStart_ = true;

    // 開始時アニメーション再生中フラグ
    bool isStartAnimationPlaying_ = true;

    // レターボックス
    std::unique_ptr<Letterbox> letterbox_;
    float letterboxAnimTimer_ = 0.0f;
    float letterboxAnimDuration_ = 0.25f; // seconds
    float letterboxStartHeight_ = 0.0f;
    float letterboxEndHeight_ = 0.0f;

	// クリアUI
	std::unique_ptr<ClearUI> clearUI_;

	// タイトルBGM
    uint32_t titleBGMHandle_ = 0;
	// ゲームBGM
    uint32_t gameBGMHandle_ = 0;
	// 決定音
    uint32_t decideSEHandle_ = 0;
	// 選択音
    uint32_t selectSEHandle_ = 0;
	// ボスロックオン音
    uint32_t bossLockOnSEHandle_ = 0;
	// ボスアンロック音
    uint32_t bossUnlockSEHandle_ = 0;

private:

	// ボスヒット時にほとんどの更新を停止するための制御
	bool isBossHitFreezeActive_ = false;
	float bossHitFreezeTimer_ = 0.0f;
	static inline const float kBossHitFreezeDuration = 0.25f; // 秒
	// ボスのヒット状態の前フレーム保存（ヒット瞬間の検出用）
	bool prevBossHit_ = false;

    // ボス登場時アニメーション制御
    bool bossIntroPlaying_ = false;           // 再生中フラグ
    bool bossIntroScheduled_ = false;         // 予約フラグ
    bool bossIntroDelayAfterFreeze_ = false;  // フリーズ解除後の遅延待ちフラグ
    float bossIntroTimer_ = 0.0f;             // 再生時間
    float bossIntroDelayTimer_ = 0.0f;        // 開始遅延タイマー
    static inline constexpr float kBossIntroDuration_ = 8.0f;
    static inline constexpr float kBossIntroStartDelay_ = 0.5f;

    // ボス撃破時アニメーション制御（登場時演出に準拠）
    bool bossOutroPlaying_ = false;           // 再生中フラグ
    bool bossOutroScheduled_ = false;         // 予約フラグ
    float bossOutroTimer_ = 0.0f;             // 再生時間
    static inline constexpr float kBossOutroDuration_ = 8.0f; // 秒
    static inline constexpr float kBossOutroStartDelay_ = 0.5f; // 秒

	/// <summary>
	/// 入力のコマンドを設定する
	/// </summary>
	void InputRegisterCommand();

	/// <summary>
	/// 当たり判定の更新処理
	/// </summary>
	void UpdateCollision();

    /// <summary>
    /// 開始アニメーションの更新処理
    /// </summary>
    void UpdateStartAnimation();

	bool isTitleLocked_ = true;
	bool isTransitioning_ = false;
	float transitionTimer_ = 0.0f;
	static inline constexpr float kTransitionDuration_ = 1.0f;

	std::unique_ptr<GameEngine::Sprite> titleSprite_;
	uint32_t titleGH_ = 0;

	std::unique_ptr<GameEngine::Sprite> spaceSprite_;
	uint32_t spaceGH_ = 0;

	// イージングで遷移する注視点
	Vector3 transitionStartTarget_ = {0.0f, 16.0f, 0.0f};
	Vector3 transitionEndTarget_ = {0.0f, 16.0f, 0.0f};
};