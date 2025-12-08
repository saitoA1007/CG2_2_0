#pragma once
#include"BaseScene.h"

// エンジン機能をインクルード
#include"Camera.h"
#include"DebugCamera.h"
#include"Model.h"
#include"Sprite.h"
#include"WorldTransform.h"
#include"DebugRenderer.h"
#include"CollisionManager.h"

#include"Application/Scene/Transition/Fade.h"

#include"Application/Stage/StageManager.h"
#include"Application/Stage/StageWallPlane.h"
#include"Application/Player/Player.h"
#include"Animator.h"
#include"Application/Graphics/PlaneProjectionShadow.h"
#include"Application/Graphics/Terrain.h"
#include"Extension/CustomMaterial/IceMaterial.h"
#include"Application/Light/SceneLightingController.h"
#include"Application/Camera/CameraController.h"

class TitleScene : public BaseScene {
public:

	~TitleScene();

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
	void DebugUpdate() override{}

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
	SceneState NextSceneState() override { return SceneState::TDGame; }

	std::unique_ptr<ITransitionEffect> GetTransitionEffect() override { return std::make_unique<Fade>(); }

private: // シーン機能

	// 終了フラグ
	bool isFinished_ = false;

	// メインカメラ
	std::unique_ptr<GameEngine::Camera> mainCamera_;

	// タイトル画像(仮)
	std::unique_ptr<GameEngine::Sprite> titleSprite_;
	uint32_t titleGH_ = 0;

	// スペース画像(仮)
	std::unique_ptr<GameEngine::Sprite> spaceSprite_;
	uint32_t spaceGH_ = 0;

private:
	/// <summary>
	/// 入力コマンドを登録する
	/// </summary>
	void InputRegisterCommand();

private:
	std::unique_ptr<StageManager> stageManager_;
	GameEngine::Model* stageWallPlaneModel_ = nullptr;
	static inline const size_t kNumStageWalls = 6;
	std::array<StageWallPlane, kNumStageWalls> stageWallPlanes_;
	std::unique_ptr<IceMaterial> stageWallPlaneMaterial_;

	GameEngine::Model* playerModel_ = nullptr;
	std::unique_ptr<Player> player_;
	std::unique_ptr<GameEngine::Animator> playerAnimator_;
	std::array<std::map<std::string, AnimationData>, static_cast<size_t>(PlayerAnimationType::MaxCount)> playerAnimationData_;
	std::unique_ptr<PlaneProjectionShadow> playerShadow_;

	std::unique_ptr<Terrain> terrain_;
	GameEngine::Model* icePlaneModel_ = nullptr;

	// ライト
	std::unique_ptr<SceneLightingController> sceneLightingController_;

	GameEngine::Model* wallModel_ = nullptr;

	GameEngine::Model* playerChargeEffectModel_ = nullptr;
	GameEngine::Model* playerRushEffectModel_ = nullptr;
	GameEngine::Model* playerAttackDownEffectModel_ = nullptr;

	std::unique_ptr<class PlayerChargeEffect> playerChargeEffect_;
	std::unique_ptr<class PlayerRushEffect> playerRushEffect_;
	std::unique_ptr<class PlayerAttackDownEffect> playerAttackDownEffect_;

	std::unique_ptr<GameEngine::CollisionManager> collisionManager_;
	std::unique_ptr<GameEngine::DebugRenderer> debugRenderer_;

	std::unique_ptr<CameraController> cameraController_;

	void UpdateCollision();
};