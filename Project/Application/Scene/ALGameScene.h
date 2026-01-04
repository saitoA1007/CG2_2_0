#pragma once
#include"BaseScene.h"

// エンジン機能をインクルード
#include"Camera.h"
#include"Model.h"
#include"WorldTransform.h"
#include"Sprite.h"
#include"DebugRenderer.h"
#include"CollisionManager.h"
#include"ParticleSystem/ParticleBehavior.h"

// アプリ機能をインクルード
#include"Application/Player/Player.h"
#include"Application/Camera/FollowCameraController.h"
#include"Application/Light/SceneLightingController.h"
#include"Application/Enemy/BossEnemy.h"
#include"Application/Enemy/Projectile/EnemyProjectileManager.h"
#include"Application/Stage/StageManager.h"
#include"Application/ClearTimeTracker.h"
#include"Application/Weapon/Sword.h"
#include"Application/Graphics/PlaneProjectionShadow.h"
#include"Application/Graphics/Terrain.h"
#include"Application/UI/BossHpUI.h"
#include"Application/UI/PlayerHpUI.h"
#include"Application/UI/GameOverUI.h"

class ALGameScene : public BaseScene {
public:

	~ALGameScene();

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
	void DebugUpdate() override {}

	/// <summary>
	/// 描画処理
	/// </summary>
	void Draw(const bool& isDebugView) override;

	/// <summary>
	/// 終了したことを伝える
	/// </summary>
	/// <returns></returns>
	bool IsFinished() override { return isFinished_; };

	/// <summary>
	/// 次のシーン遷移する場面の値を取得
	/// </summary>
	/// <returns></returns>
	SceneState NextSceneState() override { return sceneState_; }

private: // エンジンの低レイヤー機能を取得

	// デバック描画するリスト
	std::unique_ptr<GameEngine::DebugRenderer> debugRenderer_;

	// 当たり判定の管理
	std::unique_ptr<GameEngine::CollisionManager> collisionManager_;

private: // シーン機能

	// 終了フラグ
	bool isFinished_ = false;

	SceneState sceneState_ = SceneState::Result;

	// メインカメラ
	std::unique_ptr<GameEngine::Camera> mainCamera_;

	// カメラコントローラークラス
	std::unique_ptr<FollowCameraController> followCameraController_;

	// ライト
	std::unique_ptr<SceneLightingController> sceneLightingController_;

	//================================================
	// プレイヤー
	//================================================

	// 自キャラのモデル
	GameEngine::Model* playerModel_;
	// プレイヤー
	std::unique_ptr<Player> player_;
	// プレイヤーの影
	std::unique_ptr<PlaneProjectionShadow> playerShadow_;

	// プレイやーの移動パーティクル
	std::unique_ptr<GameEngine::ParticleBehavior> playerMoveParticle_;

	// ダメージを受けた時のパーティクル
	std::unique_ptr<GameEngine::ParticleBehavior> hitEffectParticle_;
	// ダメージを与えた時のパーティクル
	std::unique_ptr<GameEngine::ParticleBehavior> attackEffectParticle_;
	std::unique_ptr<GameEngine::ParticleBehavior> attackAccentEffectParticle_;

	// 武器モデル
	GameEngine::Model* swordModel_;
	// 武器
	std::unique_ptr<Sword> playerSword_;

	//=======================================================
	// 敵
	//=======================================================

	// ボスキャラのモデル
	GameEngine::Model* bossEnemyModel_;
	GameEngine::Model* bossEnemyEyeModel_;
	// ボス敵キャラのモデル
	std::unique_ptr<BossEnemy> bossEnemy_;
	// ボス敵の影
	std::unique_ptr<PlaneProjectionShadow> bossEnemyShadow_;

	// ボスの移動パーティクル
	std::unique_ptr<GameEngine::ParticleBehavior> bossEnmeyMoveParticle_;

	// 岩弾のモデル
	GameEngine::Model* rockBulletModel_;

	// 敵の遠距離攻撃を管理するクラス
	std::unique_ptr<EnemyProjectileManager> enemyProjectileManager_;

	//=========================================================
	// 地形
	//=========================================================

	// 天球
	GameEngine::Model* skyDomeModel_;
	GameEngine::WorldTransform skyDomeWorldTransform_;

	// 氷の地面を作るためのモデル
	GameEngine::Model* icePlaneModel_;
	// 地面
	std::unique_ptr<Terrain> terrain_;

	// 壁
	GameEngine::Model* wallModel_;
	std::unique_ptr<StageManager> stageManager_;

	// 空気を演出するためのパーティクル
	std::unique_ptr<GameEngine::ParticleBehavior> airParticle_;
	// 空気の揺らぎを管理
	float airTimer_ = 0.0f;
	float maxAirTime_ = 4.0f;
	float airSpeed_ = 2.0f;

	//===============================================
	// UI
	//===============================================

	// ボスのHpを表示
	std::unique_ptr<BossHpUI> bossHpUI_;

	// プレイヤーのHpを表示
	std::unique_ptr<PlayerHpUI> playerHpUI_;

	// ゲームオーバーUI
	std::unique_ptr<GameOverUI> gameOverUI_;
	// ゲームオーバーフラグ
	bool isGameOver_ = false;

	/// 演出

	// 平面モデル
	GameEngine::Model* planeModel_;

	
	// クリアまでの時間を計測
	std::unique_ptr<ClearTimeTracker> clearTimeTracker_;

private:

	/// <summary>
	/// 入力のコマンドを設定する
	/// </summary>
	void InputRegisterCommand();

	/// <summary>
	/// 当たり判定の更新処理
	/// </summary>
	void UpdateCollision();

	/// <summary>
	/// ゲームプレイの更新処理
	/// </summary>
	void GamePlayUpdate();
};
