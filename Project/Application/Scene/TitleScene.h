#pragma once
#include"BaseScene.h"

// エンジン機能をインクルード
#include"Camera.h"
#include"DebugCamera.h"
#include"Model.h"
#include"WorldTransform.h"

#include"Application/Scene/Transition/Fade.h"
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
	/// 次のシーン遷移する場面の名前を取得
	/// </summary>
	/// <returns></returns>
	std::string NextSceneName() override { return "Title"; }

	/// <summary>
	/// 遷移する演出
	/// </summary>
	/// <returns></returns>
	std::unique_ptr<ITransitionEffect> GetTransitionEffect() override { return std::make_unique<Fade>(); }

private: // シーン機能

	// 終了フラグ
	bool isFinished_ = false;

	// メインカメラ
	std::unique_ptr<GameEngine::Camera> mainCamera_;

	// ライトの管理
	std::unique_ptr<SceneLightingController> sceneLightingController_;

	// スカイボックス
	GameEngine::Model* skyboxModel_;
	GameEngine::WorldTransform skyboxWorldTransform_;
	uint32_t skyboxGH_ = 0u;

	// 地面モデル
	GameEngine::Model* terrainModel_;
	uint32_t grassGH_ = 0u;
	GameEngine::WorldTransform terrainWorldTransform_;
	float terrainMetalic_ = 0.01f;
	float terrainShininess_ = 250.0f;

	// 球のモデル
	GameEngine::Model* sphereModel_;
	GameEngine::WorldTransform sphereWorldTransform_;
	uint32_t monsterGH_ = 0;
	float sphereMetalic_ = 0.01f;
	float sphereShininess_ = 500.0f;

	// objのplaneモデル
	GameEngine::Model* objPlaneModel_;
	GameEngine::WorldTransform objPlaneWorldTransform_;

	// gltfのplaneモデル
	GameEngine::Model* gltfPlaneModel_;
	GameEngine::WorldTransform gltfPlaneWorldTransform_;

private:

	/// <summary>
	/// デバックした値を登録
	/// </summary>
	void RegisterDebugParam();

	/// <summary>
	/// デバックした値を取得
	/// </summary>
	void ApplyDebugParam();
};