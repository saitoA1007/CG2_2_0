#pragma once
#include"BaseScene.h"

// エンジン機能をインクルード
#include"Camera.h"
#include"DebugCamera.h"
#include"Model.h"
#include"Sprite.h"
#include"WorldTransform.h"

#include"Extension/CustomMaterial/IceRockMaterial.h"
#include"Application/Light/SceneLightingController.h"

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
	/// 終了したことを伝える
	/// </summary>
	/// <returns></returns>
	bool IsFinished() override { return isFinished_; };

	/// <summary>
	/// 次のシーン遷移する場面の値を取得
	/// </summary>
	/// <returns></returns>
	SceneState NextSceneState() override { return SceneState::ALGame; }

private: // シーン機能

	// 終了フラグ
	bool isFinished_ = false;

	// メインカメラ
	std::unique_ptr<GameEngine::Camera> mainCamera_;

	// 天球
	GameEngine::Model* skyDomeModel_;
	GameEngine::WorldTransform skyDomeWorldTransform_;

	// ライト
	std::unique_ptr<SceneLightingController> sceneLightingController_;

	// ボスモデル
	GameEngine::Model* bossEnemyModel_;
	GameEngine::Model* bossEnemyEyeModel_;
	// 行列
	GameEngine::WorldTransform bossWorldTransform_;
	// マテリアル
	std::unique_ptr<IceRockMaterial> iceMaterial_;
	std::string groupName_ = "Boss_Material";

	// タイトル画像
	std::unique_ptr<GameEngine::Sprite> titleSprite_;
	uint32_t titleGH_ = 0;

	// スペース画像
	std::unique_ptr<GameEngine::Sprite> spaceSprite_;
	uint32_t spaceGH_ = 0;

	// タイトル音声
	uint32_t titleSH_ = 0;
	// 決定音
	uint32_t decisionSH_ = 0;
};