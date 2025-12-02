#pragma once
#include"BaseScene.h"

// エンジン機能をインクルード
#include"Camera.h"
#include"Model.h"
#include"WorldTransform.h"
#include"Sprite.h"
#include"DebugRenderer.h"
#include"CollisionManager.h"
#include"Extension/CustomMaterial/IceMaterial.h"
#include"Animator.h"

// アプリ機能をインクルード
#include"Application/Player/Player.h"
#include"Application/Camera/CameraController.h"
#include"Application/Light/SceneLightingController.h"
#include"Application/Stage/StageManager.h"
#include"Application/Enemy/BossEnemy.h"
#include"Application/Enemy/EnemyAttackManager.h"
#include"Application/Graphics/Terrain.h"
#include"Application/Stage/StageWallPlane.h"
#include <array>
#include"Application/Graphics/PlaneProjectionShadow.h"
#include"Application/Enemy/Effect/EnemyRushEffect.h"

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
	/// 終了したことを伝える
	/// </summary>
	/// <returns></returns>
	bool IsFinished() override { return isFinished_; };

	/// <summary>
	/// 次のシーン遷移する場面の値を取得
	/// </summary>
	/// <returns></returns>
	SceneState NextSceneState() override { return SceneState::Title; }

private: // エンジンの低レイヤー機能を取得

	// デバック描画するリスト
	std::unique_ptr<GameEngine::DebugRenderer> debugRenderer_;
	// デバックの表示管理
	bool isDrawCollision_ = true;

	// 当たり判定の管理
	std::unique_ptr<GameEngine::CollisionManager> collisionManager_;

private: // シーン機能

	// 終了フラグ
	bool isFinished_ = false;

	// メインカメラ
	std::unique_ptr<GameEngine::Camera> mainCamera_;

	// 天球
	GameEngine::Model* skyDomeModel_;
	GameEngine::WorldTransform skyDomeWorldTransform_;

	// 氷の地面を作るためのモデル
	GameEngine::Model* icePlaneModel_;
	// 地面
	std::unique_ptr<Terrain> terrain_;

	// ライト
	std::unique_ptr<SceneLightingController> sceneLightingController_;

	// 自キャラのモデル
	GameEngine::Model* playerModel_;
	// プレイヤー
	std::unique_ptr<Player> player_;
	// プレイヤーの影
	std::unique_ptr<PlaneProjectionShadow> playerShadow_;

	// カメラコントローラークラス
	std::unique_ptr<CameraController> cameraController_;

	// ステージを生成する
	GameEngine::Model* wallModel_;
	std::unique_ptr<StageManager> stageManager_;

	// StageWallPlane用のモデル
	GameEngine::Model* stageWallPlaneModel_;
	// 板ポリ壁（6つ）
	static inline const size_t kNumStageWalls = 6;
	std::array<StageWallPlane, kNumStageWalls> stageWallPlanes_;
    // Stage wall shared material
    std::unique_ptr<IceMaterial> stageWallPlaneMaterial_;

	// ボス敵モデル
	GameEngine::Model* bossEnemyModel_;
	// ボス敵
	std::unique_ptr<BossEnemy> bossEnemy_;

	// ボスのアニメーションデータ
	std::map<std::string, AnimationData> bossEnemyAnimationData_;
	// ボスのアニメーションを再生するクラス
	std::unique_ptr<GameEngine::Animator> bossEnemyAnimator_;
	// ボスの影
	std::unique_ptr<PlaneProjectionShadow> bossEnemyShadow_;

	// ボスの遠距離攻撃管理クラス
	std::unique_ptr<EnemyAttackManager> enemyAttackManager_;
	// ボスの突進攻撃演出
	std::unique_ptr<EnemyRushEffect> enemyRushEffect_;

	// 氷柱のモデル
	GameEngine::Model* iceFallModel_;
	// 敵の突進攻撃演出のモデル
	GameEngine::Model* enemyRushModel_;

	// 平面モデル
	GameEngine::Model* planeModel_;

	// ボスロックオンフラグ
    bool isBossLockOn_ = false;

private:

	/// <summary>
	/// 入力のコマンドを設定する
	/// </summary>
	void InputRegisterCommand();

	/// <summary>
	/// 当たり判定の更新処理
	/// </summary>
	void UpdateCollision();
};